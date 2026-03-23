#include "KE/Systems/RenderSystem.h"
#include "KE/Systems/WindowSystem.h"
#include "KE/Vulkan/VulkanContext.h"  
#include "KE/Engine.h"
#include "KE/GameFramework/GameInstance.h"
#include "KE/GameFramework/World/World.h"
#include "KE/Vulkan/Managers/SwapchainManager.h"
#include "KE/Vulkan/Managers/CommandManager.h"
#include "KE/Vulkan/Managers/PipelineManager.h"
#include "KE/Vulkan/Managers/RenderPassManager.h"
#include "KE/Vulkan/Managers/DescriptorManager.h"
#include "KE/Vulkan/Managers/WireframePipeline.h"
#include "KE/Vulkan/Managers/BufferManager.h"
#include "KE/Vulkan/Managers/DeviceManager.h"
#include "KE/Vulkan/Managers/SyncManager.h"
#include "KE/Vulkan/VKinfo.h"
#include "KE/Vulkan/RenderInfo.h"
#include "CoreMinimal.h"
#include <glm/glm.hpp>

RenderSystem::RenderSystem ()
	: m_vulkanContext ( MakeUnique<VulkanContext> () )
	{
	LogDebug ( "RenderSystem Created" );
	}

const VkClearValue RenderSystem::m_clearValues[ 2 ] = {
	{0.2f, 0.2f, 0.5f, 1.0f},  // Color
	{1.0f, 0}                    // Depth/stencil
	};

RenderSystem::~RenderSystem () = default;

bool RenderSystem::CreateUniformBuffers ()
	{
	uint32_t imageCount = Swapchain->GetImagesCount ();
	m_FrameUniformBuffers.resize ( imageCount );

	LogDebug ( "Creating uniform buffers for ", imageCount, " frames..." );

	for (uint32_t i = 0; i < imageCount; i++)
		{
			// View/projection uniform buffer
		m_FrameUniformBuffers[ i ].ViewProjectionBuffer =
			BuffMgr->CreateUniformBuffer ( VIEW_PROJ_BUFFER_SIZE );

		if (!m_FrameUniformBuffers[ i ].ViewProjectionBuffer.IsValid ())
			{
			LogError ( "Failed to create view/projection uniform buffer for frame ", i );
			return false;
			}

			// Dynamic object uniform buffer
		m_FrameUniformBuffers[ i ].ObjectBuffer =
			BuffMgr->CreateBuffer (
				OBJECT_BUFFER_SIZE,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

		if (!m_FrameUniformBuffers[ i ].ObjectBuffer.IsValid ())
			{
			LogError ( "Failed to create object uniform buffer for frame ", i );
			return false;
			}

			// Map object buffer for quick access
		m_FrameUniformBuffers[ i ].MappedObjectData =
			BuffMgr->MapBuffer ( m_FrameUniformBuffers[ i ].ObjectBuffer );

		if (!m_FrameUniformBuffers[ i ].MappedObjectData)
			{
			LogError ( "Failed to map object uniform buffer for frame ", i );
			return false;
			}

		m_FrameUniformBuffers[ i ].CurrentObjectCount = 0;
		}

	LogDebug ( "Uniform buffers created successfully" );
	return true;
	}


bool RenderSystem::CreateDescriptorSets ()
	{
	uint32_t imageCount = Swapchain->GetImagesCount ();
	m_FrameDescriptorSets.resize ( imageCount );

	LogDebug ( "Creating descriptor sets for ", imageCount, " frames..." );

	// Проверяем, что пул существует
	VkDescriptorPool perFramePool = DescMgr->GetDescriptorPool ( "PerFramePool" );
	if (perFramePool == VK_NULL_HANDLE)
		{
		LogError ( "CreateDescriptorSets: PerFramePool is VK_NULL_HANDLE" );
		return false;
		}

	for (uint32_t i = 0; i < imageCount; i++)
		{
		if (!m_FrameUniformBuffers[ i ].IsValid ())
			{
			LogError ( "Uniform buffers for frame ", i, " are not valid" );
			return false;
			}

		LogDebug ( "  Allocating global descriptor set for frame ", i );

		// Allocate global descriptor set (view/projection)
		m_FrameDescriptorSets[ i ].GlobalSet =
			DescMgr->AllocateDescriptorSet ( "PerFramePool", m_GlobalLayout );

		if (m_FrameDescriptorSets[ i ].GlobalSet == VK_NULL_HANDLE)
			{
			LogError ( "Failed to allocate global descriptor set for frame ", i );

			// Пытаемся сбросить пул и попробовать снова
			LogDebug ( "  Resetting PerFramePool and retrying..." );
			DescMgr->ResetDescriptorPool ( "PerFramePool" );

			m_FrameDescriptorSets[ i ].GlobalSet =
				DescMgr->AllocateDescriptorSet ( "PerFramePool", m_GlobalLayout );

			if (m_FrameDescriptorSets[ i ].GlobalSet == VK_NULL_HANDLE)
				{
				LogError ( "Still failed to allocate global descriptor set after pool reset" );
				return false;
				}
			}

		LogDebug ( "  Updating global descriptor set for frame ", i,
				   " with buffer ", ( void * ) m_FrameUniformBuffers[ i ].ViewProjectionBuffer.Buffer );

		  // Update global descriptor with view/projection buffer
		DescMgr->UpdateBufferDescriptor (
			m_FrameDescriptorSets[ i ].GlobalSet,
			0,  // binding 0
			m_FrameUniformBuffers[ i ].ViewProjectionBuffer.Buffer,
			0,
			VK_WHOLE_SIZE,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER );

		// Pre-allocate space for per-object descriptor sets
		m_FrameDescriptorSets[ i ].PerObjectSets.reserve ( MAX_OBJECTS_PER_FRAME );
		}

	LogDebug ( "Descriptor sets created successfully" );
	return true;
	}


void RenderSystem::UpdateUniformBuffers ( uint32_t FrameIndex )
	{
	
	if (!m_RenderInfo.IsValid () || FrameIndex >= m_FrameUniformBuffers.size ()) return;

	auto & frameUB = m_FrameUniformBuffers[ FrameIndex ];

	// Update view/projection uniform buffer
	FViewProjectionUniform viewProj;
	viewProj.View = CEMath::ToGLM ( m_RenderInfo.Camera.GetViewMatrix () );
	viewProj.Projection = CEMath::ToGLM ( m_RenderInfo.Camera.GetProjectionMatrix () );

	BuffMgr->UpdateUniformBuffer (
		frameUB.ViewProjectionBuffer,
		&viewProj,
		sizeof ( viewProj ) );

	// Update object uniform buffer with all meshes
	const auto & meshes = m_RenderInfo.RenderMeshes;
	uint32_t objectCount = 0;

	for (size_t i = 0; i < meshes.size () && objectCount < MAX_OBJECTS_PER_FRAME; i++)
		{
		const FMeshInfo & mesh = meshes[ i ];
		if (!mesh.IsValid ()) continue;

		FObjectUniform * objectData =
			static_cast< FObjectUniform * > ( frameUB.MappedObjectData ) + objectCount;

		objectData->Model = CEMath::ToGLM ( mesh.Model );
		objectData->Color = glm::vec4 ( 1.0f, 1.0f, 1.0f, 1.0f ); // Default white
		objectData->AdditionalData = glm::vec4 ( 0.0f );

		objectCount++;
		}

	frameUB.CurrentObjectCount = objectCount;

	// Note: No need to flush because we use HOST_COHERENT bit
	}

bool RenderSystem::PreInit ()
	{
	LogInfo ( "Checking vulkan support" );
	if (!m_vulkanContext->PreInit ())
		{
		LogError ( "Failed to preInit vulkan context" );
		return false;
		}
	return true;
	}

bool RenderSystem::Init ()
	{
	LogInfo ( "Initializing RenderSystem" );

	m_window = GetEngine ()->GetWindowHandle ();
	if (!m_window)
		{
		LogError ( "Failed to get window handle" );
		return false;
		}
	LogInfo ( "Window handle obtained successfully" );

	m_vulkanContext->SetWindow ( m_window );

	if (!m_vulkanContext->Init ())
		{
		LogError ( "Failed to init vulkan context" );
		return false;
		}

	auto windowSystem = GetEngine ()->GetWindow ();
	if (windowSystem)
		{
		windowSystem->OnResize = [ this ] ( int width, int height )
			{
			OnWindowResize ( width, height );
			};
		}

	if (!CacheManagers ())
		{
		LogError ( "Failed to cache managers" );
		return false;
		}

	 // ПОЛУЧАЕМ DESCRIPTOR LAYOUTS
	if (!DescMgr)
		{
		LogError ( "DescriptorManager not available" );
		return false;
		}

	m_GlobalLayout = DescMgr->GetGlobalLayout ();
	m_PerObjectLayout = DescMgr->GetPerObjectLayout ();

	if (m_GlobalLayout == VK_NULL_HANDLE || m_PerObjectLayout == VK_NULL_HANDLE)
		{
		LogError ( "Failed to get descriptor set layouts" );
		return false;
		}

		// СОЗДАЕМ UNIFORM-БУФЕРЫ И ДЕСКРИПТОРЫ
	if (!CreateUniformBuffers ())
		{
		LogError ( "Failed to create uniform buffers" );
		return false;
		}

	if (!CreateDescriptorSets ())
		{
		LogError ( "Failed to create descriptor sets" );
		return false;
		}

		// Получаем количество изображений в swapchain
	uint32_t imageCount = static_cast< uint32_t >( Swapchain->GetImagesCount () );
	if (imageCount == 0)
		{
		LogError ( "Swapchain has no images" );
		return false;
		}
	LogDebug ( "Swapchain image count: ", imageCount );

	// Создаем command buffers для каждого изображения swapchain
	CmdManager->SetFrameCommandBuffers ( imageCount );
	if (CmdManager->GetFrameCommandBufferCount () != imageCount)
		{
		LogError ( "Failed to create frame command buffers" );
		return false;
		}
	LogDebug ( "Created ", imageCount, " frame command buffers" );

	// Создаем объекты синхронизации для каждого кадра
	if (!SyncMgr->CreateFrameSyncObjects ( imageCount ))
		{
		LogError ( "Failed to create frame sync objects" );
		return false;
		}
	LogDebug ( "Created ", imageCount, " frame sync objects" );

	CreateTriangleBuffer ();
	if (!m_triangleVertexBuffer.IsValid ())
		{
		LogError ( "Failed to create triangle vertex buffer" );
		return false;
		}

	bIsInitialized = true;
	LogInfo ( "RenderSystem initialized successfully" );
	return true;
	}


	void RenderSystem::OnWindowResize ( int width, int height )
		{
		if (!IsInitialized ()) return;

		LogDebug ( "RenderSystem handling window resize: ", width, "x", height );

		// Ждём завершения всех GPU операций
		auto * deviceMgr = static_cast< DeviceManager * >( m_vulkanContext->GetInfo ()->Managers.DeviceManager.get () );
		if (deviceMgr)
			{
			vkDeviceWaitIdle ( deviceMgr->GetDevice () );
			}

			// ========== ДОБАВЬТЕ ЭТИ СТРОКИ ==========
			// Очищаем старые ресурсы
		CleanupFrameResources ();
		// ========================================

		// Пересоздаём swapchain
		if (Swapchain)
			{
			if (!Swapchain->RecreateSwapChain ())
				{
				LogError ( "Failed to recreate swapchain after resize" );
				}
			}

			// Пересоздаём render pass и framebuffers
		if (RenderPassMgr)
			{
			if (!RenderPassMgr->RecreateForSwapChain ())
				{
				LogError ( "Failed to recreate render pass after resize" );
				}
			}

			// ========== ДОБАВЬТЕ ЭТИ СТРОКИ ==========
			// Пересоздаём uniform-буферы и дескрипторы с новым количеством изображений
		if (!CreateUniformBuffers ())
			{
			LogError ( "Failed to recreate uniform buffers after resize" );
			}

		if (!CreateDescriptorSets ())
			{
			LogError ( "Failed to recreate descriptor sets after resize" );
			}
			// ========================================
		}


void RenderSystem::Shutdown ()
	{
	LogInfo ( "Shutting down RenderSystem" );

	auto * deviceMgr = static_cast< DeviceManager * >( m_vulkanContext->GetInfo ()->Managers.DeviceManager.get () );
	if (deviceMgr)
		{
		vkDeviceWaitIdle ( deviceMgr->GetDevice () );

		// ========== ДОБАВЬТЕ ЭТУ СТРОКУ ==========
		CleanupFrameResources ();  // Очищаем uniform-буферы
		// =======================================

		// Очищаем вершинный буфер треугольника
		if (BuffMgr && m_triangleVertexBuffer.IsValid ())
			{
			BuffMgr->DestroyBuffer ( m_triangleVertexBuffer );
			LogDebug ( "Triangle vertex buffer destroyed" );
			}

		if (BuffMgr)
			{
			BuffMgr->Shutdown ();
			}
		}

	m_vulkanContext->Shutdown ();
	m_vulkanContext.reset ();
	bIsInitialized = false;
	}

void RenderSystem::CleanupFrameResources ()
	{
	if (!BuffMgr) return;

	for (auto & frame : m_FrameUniformBuffers)
		{
		if (frame.MappedObjectData)
			{
			BuffMgr->UnmapBuffer ( frame.ObjectBuffer );
			frame.MappedObjectData = nullptr;
			}
		BuffMgr->DestroyBuffer ( frame.ViewProjectionBuffer );
		BuffMgr->DestroyBuffer ( frame.ObjectBuffer );
		}
	m_FrameUniformBuffers.clear ();
	m_FrameDescriptorSets.clear ();
	}

void RenderSystem::Update ( float DeltaTime )
	{
	if (!IsInitialized ()) return;

	if (!RenderScene ())
		{
		static int ErrorCount = 0;
		ErrorCount++;
		if (ErrorCount > 5)
			{
			LogFatal ( "Fatal error during render scene. Error count: ", ErrorCount );
			CEngine::Get ().RequestShutdown ();
			return;
			}
		}
	static int frameCount = 0;
	static auto lastTime = std::chrono::high_resolution_clock::now ();
	frameCount++;

	auto currentTime = std::chrono::high_resolution_clock::now ();
	float elapsed = std::chrono::duration<float> ( currentTime - lastTime ).count ();
	if (elapsed >= 1.0f)
		{
		float fps = frameCount / elapsed;
		LogDebug ( "FPS: ", fps );
		frameCount = 0;
		lastTime = currentTime;
		}
	}

const std::string RenderSystem::GetSystemName () const
	{
	return "RenderSystem";
	}

void RenderSystem::SetEngineName ( const std::string & inName )
	{
	m_vulkanContext->SetEngineName ( inName );
	}

void RenderSystem::SetAplicationName ( const std::string & inName )
	{
	m_vulkanContext->SetAplicationName ( inName );
	}

bool RenderSystem::CacheManagers ()
	{
	auto Info = m_vulkanContext->GetInfo ();
	if (!Info)
		{
		LogError ( "VulkanInfo is null" );
		return false;
		}

	Swapchain = dynamic_cast< SwapchainManager * >( Info->Managers.SwapchainManager.get () );
	CmdManager = dynamic_cast< CommandManager * >( Info->Managers.CommandManager.get () );
	PipelineMgr = dynamic_cast< PipelineManager * >( Info->Managers.PipelineManager.get () );
	RenderPassMgr = dynamic_cast< RenderPassManager * >( Info->Managers.RenderPassManager.get () );
	SyncMgr = dynamic_cast< CSyncManager * >( Info->Managers.SyncManager.get () );
	BuffMgr = dynamic_cast< BufferManager * >( Info->Managers.BufferManager.get () );
	DescMgr = dynamic_cast< DescriptorManager * >( Info->Managers.DescriptorManager.get () );

	bool allValid = Swapchain && CmdManager && PipelineMgr && RenderPassMgr && SyncMgr && BuffMgr && DescMgr;

	if (!allValid)
		{
		if (!Swapchain) LogError ( "SwapchainManager not available" );
		if (!CmdManager) LogError ( "CommandManager not available" );
		if (!PipelineMgr) LogError ( "PipelineManager not available" );
		if (!RenderPassMgr) LogError ( "RenderPassManager not available" );
		if (!SyncMgr) LogError ( "SyncManager not available" );
		if (!BuffMgr) LogError ( "BufferManager not available" );
		if (!DescMgr) LogError ( "DescriptorManager not available" );
		}
	else
		{
		LogDebug ( "All managers cached successfully" );
		}

	return allValid;
	}

void RenderSystem::CreateTriangleBuffer ()
	{
	struct Vertex
		{
		float pos[ 3 ];
		float color[ 3 ];
		};

	std::vector<Vertex> vertices = {
		{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Нижняя - красный
		{{0.5f, 0.5f, 0.0f},  {0.0f, 1.0f, 0.0f}},  // Правая - зеленый
		{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}   // Левая - синий
		};

	auto * bufferMgr = static_cast< BufferManager * >( m_vulkanContext->GetInfo ()->Managers.BufferManager.get () );
	m_triangleVertexBuffer = bufferMgr->CreateVertexBuffer ( vertices );

	if (!m_triangleVertexBuffer.IsValid ())
		{
		LogError ( "Failed to create triangle vertex buffer" );
		}
	else
		{
		LogDebug ( "Triangle vertex buffer created, size: ", m_triangleVertexBuffer.Size );
		}

	}


bool RenderSystem::RenderTriangle ()
	{
	static int warnCount = 0;
	if (warnCount < 1)
		{
		LogWarn ( "Nothing to render, render fallback triangle for debug" );
		warnCount++;
		}
	uint32_t currentFrame = SyncMgr->GetCurrentFrame ();

	VkFence inFlightFence = SyncMgr->GetInFlightFence ( currentFrame );
	if (inFlightFence != VK_NULL_HANDLE)
		{
		SyncMgr->WaitForFence ( inFlightFence );
		SyncMgr->ResetFence ( inFlightFence );
		}

	uint32_t imageIndex;
	VkSemaphore imageAvailableSemaphore = SyncMgr->GetImageAvailableSemaphore ( currentFrame );

	if (!Swapchain->AcquireNextImage ( imageAvailableSemaphore, imageIndex ))
		{
		LogWarn ( "Failed to acquire next image" );
		return false;
		}

	VkCommandBuffer cmdBuffer = CmdManager->GetFrameCommandBuffer ( imageIndex );
	if (cmdBuffer == VK_NULL_HANDLE)
		{
		LogError ( "Failed to get command buffer for image ", imageIndex );
		return false;
		}

	vkResetCommandBuffer ( cmdBuffer, 0 );

	CmdManager->RecordCommandBuffer ( cmdBuffer, [ & ] ( VkCommandBuffer cmd )
									  {
									  VkRenderPassBeginInfo renderPassInfo {};
									  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
									  renderPassInfo.renderPass = RenderPassMgr->GetMainRenderPass ();
									  renderPassInfo.framebuffer = RenderPassMgr->GetFramebuffer ( imageIndex );
									  renderPassInfo.renderArea.offset = { 0, 0 };
									  renderPassInfo.renderArea.extent = Swapchain->GetExtent ();
									  renderPassInfo.clearValueCount = 2;
									  renderPassInfo.pClearValues = m_clearValues;

									  vkCmdBeginRenderPass ( cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

									  VkViewport viewport {};
									  viewport.x = 0.0f;
									  viewport.y = 0.0f;
									  viewport.width = static_cast< float >( Swapchain->GetExtent ().width );
									  viewport.height = static_cast< float >( Swapchain->GetExtent ().height );
									  viewport.minDepth = 0.0f;
									  viewport.maxDepth = 1.0f;
									  vkCmdSetViewport ( cmd, 0, 1, &viewport );

									  VkRect2D scissor {};
									  scissor.offset = { 0, 0 };
									  scissor.extent = Swapchain->GetExtent ();
									  vkCmdSetScissor ( cmd, 0, 1, &scissor );

									  // Рендерим треугольник
									  VkPipeline trianglePipeline = PipelineMgr->GetPipeline ( "TrianglePipeline" );
									  if (trianglePipeline != VK_NULL_HANDLE && m_triangleVertexBuffer.IsValid ())
										  {
										  VkDeviceSize offsets [] = { 0 };
										  vkCmdBindVertexBuffers ( cmd, 0, 1, &m_triangleVertexBuffer.Buffer, offsets );
										  vkCmdBindPipeline ( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline );
										  vkCmdDraw ( cmd, 3, 1, 0, 0 );
										  }

									  vkCmdEndRenderPass ( cmd );
									  } );

	VkSemaphore renderFinishedSemaphore = SyncMgr->GetRenderFinishedSemaphore ( currentFrame );

	auto * deviceMgr = static_cast< DeviceManager * >( m_vulkanContext->GetInfo ()->Managers.DeviceManager.get () );
	VkQueue graphicsQueue = deviceMgr->GetGraphicsQueue ();

	VkSubmitInfo submitInfo {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

	VkResult result = vkQueueSubmit ( graphicsQueue, 1, &submitInfo, inFlightFence );
	if (result != VK_SUCCESS)
		{
		LogError ( "Failed to submit command buffer: ", static_cast< int >( result ) );
		return false;
		}

	if (!Swapchain->Present ( renderFinishedSemaphore, imageIndex ))
		{
		LogWarn ( "Failed to present image" );
		return false;
		}

	SyncMgr->NextFrame ();
	return true;
	}

bool RenderSystem::RenderScene ()
	{
	if (UpdateRenderInfo ())
		{
		if (!RenderWorld ())
			{
			LogError ( "error during Render World" );
			return false;
			}
		}
	else
		{
		if (!RenderTriangle ())
			{
			LogError ( "Error during Render Fallback triangle" );
			return false;
			}
		}

	return true;
	}

bool RenderSystem::UpdateRenderInfo ()
	{
	m_RenderInfo.Clear ();
	auto * gameInstance = CEngine::Get ().GetGameInstance ();
	if (!gameInstance)
		{
		static bool warned = false;
		if (!warned)
			{
			LogWarn ( "GameInstance is null, cannot update render info" );
			warned = true;
			}
		return false;
		}

	auto currentWorld = gameInstance->GetWorld ();
	if (currentWorld)
		{
		currentWorld->CollectRenderInfo ( &m_RenderInfo );
		}
	return m_RenderInfo.HasInfo && m_RenderInfo.IsValid ();
	}
bool RenderSystem::RenderWorld ()
	{
	uint32_t currentFrame = SyncMgr->GetCurrentFrame ();
	UpdateUniformBuffers ( currentFrame );
	VkFence inFlightFence = SyncMgr->GetInFlightFence ( currentFrame );
	if (inFlightFence != VK_NULL_HANDLE)
		{
		SyncMgr->WaitForFence ( inFlightFence );
		SyncMgr->ResetFence ( inFlightFence );
		}

	uint32_t imageIndex;
	VkSemaphore imageAvailableSemaphore = SyncMgr->GetImageAvailableSemaphore ( currentFrame );

	if (!Swapchain->AcquireNextImage ( imageAvailableSemaphore, imageIndex ))
		{
		LogWarn ( "Failed to acquire next image" );
		return false;
		}

	VkCommandBuffer cmdBuffer = CmdManager->GetFrameCommandBuffer ( imageIndex );
	if (cmdBuffer == VK_NULL_HANDLE)
		{
		LogError ( "Failed to get command buffer for image ", imageIndex );
		return false;
		}

	vkResetCommandBuffer ( cmdBuffer, 0 );


	CmdManager->RecordCommandBuffer ( cmdBuffer, [ & ] ( VkCommandBuffer cmd )
									  {
									  VkRenderPassBeginInfo renderPassInfo {};
									  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
									  renderPassInfo.renderPass = RenderPassMgr->GetMainRenderPass ();
									  renderPassInfo.framebuffer = RenderPassMgr->GetFramebuffer ( imageIndex );
									  renderPassInfo.renderArea.offset = { 0, 0 };
									  renderPassInfo.renderArea.extent = Swapchain->GetExtent ();
									  renderPassInfo.clearValueCount = 2;
									  renderPassInfo.pClearValues = m_clearValues;

									  vkCmdBeginRenderPass ( cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

									  VkViewport viewport {};
									  viewport.x = 0.0f;
									  viewport.y = 0.0f;
									  viewport.width = static_cast< float >( Swapchain->GetExtent ().width );
									  viewport.height = static_cast< float >( Swapchain->GetExtent ().height );
									  viewport.minDepth = 0.0f;
									  viewport.maxDepth = 1.0f;
									  vkCmdSetViewport ( cmd, 0, 1, &viewport );

									  VkRect2D scissor {};
									  scissor.offset = { 0, 0 };
									  scissor.extent = Swapchain->GetExtent ();
									  vkCmdSetScissor ( cmd, 0, 1, &scissor );

									  //render meshes
									  if (!RenderMeshes ( cmd ))
										  {
										  LogError ( "Error during Render Meshes" );
										  return false;
										  }
									  //render terrain
									  if (!RenderTerrain ( cmd ))
										  {
										  LogError ( "Error during Render Terrain" );
										  return false;
										  }
										  //render wireframe
									  if (!RenderWireframe ( cmd ))
										  {
										  LogError ( "Error during Render wireframe" );
										  return false;
										  }
									  vkCmdEndRenderPass ( cmd );
									  } );

	VkSemaphore renderFinishedSemaphore = SyncMgr->GetRenderFinishedSemaphore ( currentFrame );

	auto * deviceMgr = static_cast< DeviceManager * >( m_vulkanContext->GetInfo ()->Managers.DeviceManager.get () );
	VkQueue graphicsQueue = deviceMgr->GetGraphicsQueue ();

	VkSubmitInfo submitInfo {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

	VkResult result = vkQueueSubmit ( graphicsQueue, 1, &submitInfo, inFlightFence );
	if (result != VK_SUCCESS)
		{
		LogError ( "Failed to submit command buffer: ", static_cast< int >( result ) );
		return false;
		}

	if (!Swapchain->Present ( renderFinishedSemaphore, imageIndex ))
		{
		LogWarn ( "Failed to present image" );
		return false;
		}

	SyncMgr->NextFrame ();
	return true;
	}

	bool RenderSystem::RenderMeshes ( VkCommandBuffer cmd )
		{
		uint32_t currentFrame = SyncMgr->GetCurrentFrame ();
		
		if (currentFrame >= m_FrameDescriptorSets.size ())
			{
			LogError ( "RenderMeshes: currentFrame ", currentFrame, " out of range (size: ", m_FrameDescriptorSets.size (), ")" );
			return false;
			}

		if (cmd == VK_NULL_HANDLE)
			{
			LogError ( "RenderMeshes: CommandBuffer is VK_NULL_HANDLE" );
			return false;
			}

		VkDescriptorSet globalSet = m_FrameDescriptorSets[ currentFrame ].GlobalSet;
		if (globalSet == VK_NULL_HANDLE)
			{
			LogError ( "RenderMeshes: globalSet is VK_NULL_HANDLE for frame ", currentFrame );
			return false;
			}



		const auto & meshes = m_RenderInfo.RenderMeshes;

		for (size_t i = 0; i < meshes.size (); i++)
			{
			const FMeshInfo & mesh = meshes[ i ];

			if (!mesh.IsValid ())
				{
				LogWarn ( "Skipping invalid mesh at index ", i );
				continue;
				}

			VkPipeline pipeline = PipelineMgr->GetPipeline ( mesh.PipelineName );
			if (pipeline == VK_NULL_HANDLE)
				{
				LogError ( "Pipeline not found for: ", mesh.PipelineName );
				continue;
				}

			VkPipelineLayout layout = PipelineMgr->GetPipelineLayout ( mesh.PipelineName + "Layout" );
			if (layout == VK_NULL_HANDLE)
				{
				LogError ( "Layout not found for: ", mesh.PipelineName );
				continue;
				}

			vkCmdBindPipeline ( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );


			VkDescriptorSet sets[ 1 ] = { globalSet };

			vkCmdBindDescriptorSets (
				cmd,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				layout,
				0,              // firstSet
				1,              // descriptorSetCount
				sets,     // pDescriptorSets
				0,              // dynamicOffsetCount
				nullptr );       // pDynamicOffsets

			// Используем push constants только для model matrix
			struct FModelPushConstant
				{
				glm::mat4x4 model;
				} modelPush;

			modelPush.model = CEMath::ToGLM ( mesh.Model );

			vkCmdPushConstants (
				cmd,
				layout,
				VK_SHADER_STAGE_VERTEX_BIT,  // Только vertex shader
				0,                            // offset
				sizeof ( modelPush ),            // size
				&modelPush );                   // data

			// Bind vertex buffers
			VkBuffer vertexBuffers [] = { mesh.VertexBuffer };
			VkDeviceSize offsets [] = { 0 };
			vkCmdBindVertexBuffers ( cmd, 0, 1, vertexBuffers, offsets );

			if (mesh.IndexBuffer != VK_NULL_HANDLE && mesh.IndexCount > 0)
				{
				vkCmdBindIndexBuffer ( cmd, mesh.IndexBuffer, 0, VK_INDEX_TYPE_UINT32 );
				vkCmdDrawIndexed ( cmd, mesh.IndexCount, 1, 0, 0, 0 );
				}
			else
				{
				vkCmdDraw ( cmd, mesh.VertexCount, 1, 0, 0 );
				}
			}


		return true;
		}

	bool RenderSystem::RenderTerrain ( VkCommandBuffer cmd )
		{
	

		if (m_RenderInfo.Terrains.empty ())
			{
			LogTrace ( "No terrains to render" );
			return true;
			}

		uint32_t currentFrame = SyncMgr->GetCurrentFrame ();

		if (currentFrame >= m_FrameDescriptorSets.size ())
			{
			LogError ( "RenderTerrain: currentFrame ", currentFrame, " out of range (size: ", m_FrameDescriptorSets.size (), ")" );
			return false;
			}
		// Pipeline checks
		VkPipeline terrainPipeline = PipelineMgr->GetPipeline ( "TerrainPipeline" );
		VkPipelineLayout terrainLayout = PipelineMgr->GetPipelineLayout ( "TerrainLayout" );

		if (terrainPipeline == VK_NULL_HANDLE || terrainLayout == VK_NULL_HANDLE)
			{
			LogError ( "Pipeline or layout NULL" );
			return false;
			}

		vkCmdBindPipeline ( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, terrainPipeline );

		// Descriptor sets
		VkDescriptorSet globalSet = VK_NULL_HANDLE;
		if (currentFrame < m_FrameDescriptorSets.size ())
			globalSet = m_FrameDescriptorSets[ currentFrame ].GlobalSet;


		if (globalSet != VK_NULL_HANDLE)
			{
			VkDescriptorSet sets[ 1 ] = { globalSet };
			vkCmdBindDescriptorSets ( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
									  terrainLayout, 0, 1, sets, 0, nullptr );

			}

		const auto & terrains = m_RenderInfo.Terrains;


		for (size_t i = 0; i < terrains.size (); i++)
			{

			const FTerrainRenderInfo & terrain = terrains[ i ];


			if (!terrain.IsValid ())
				{
				LogWarn ( "Invalid terrain, skipping" );
				continue;
				}

			if (terrain.VertexBuffer == VK_NULL_HANDLE)
				{
				LogError ( "Vertex buffer NULL" );
				continue;
				}


			struct FTerrainPushConstants
				{
				glm::mat4x4 model;
				glm::vec4 terrainParams;
				} pushConstants;

			pushConstants.model = CEMath::ToGLM ( terrain.Model );
			pushConstants.terrainParams = glm::vec4 ( 1.0f, 1.0f, 0.001f, 1.0f );

			vkCmdPushConstants ( cmd, terrainLayout,
								 VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
								 0, sizeof ( pushConstants ), &pushConstants );



			// Bind vertex buffer
			VkBuffer vertexBuffers [] = { terrain.VertexBuffer };
			VkDeviceSize offsets [] = { 0 };
			vkCmdBindVertexBuffers ( cmd, 0, 1, vertexBuffers, offsets );


			// Draw
			if (terrain.IndexBuffer != VK_NULL_HANDLE && terrain.IndexCount > 0)
				{
				vkCmdBindIndexBuffer ( cmd, terrain.IndexBuffer, 0, VK_INDEX_TYPE_UINT32 );

				vkCmdDrawIndexed ( cmd, terrain.IndexCount, 1, 0, 0, 0 );

				}
			else
				{
				vkCmdDraw ( cmd, terrain.VertexCount, 1, 0, 0 );

				}
			}

		return true;
		}

	bool RenderSystem::RenderWireframe ( VkCommandBuffer cmd )
		{
		m_RenderInfo;
		uint32_t currentFrame = SyncMgr->GetCurrentFrame ();

		if (!m_RenderInfo.bDrawCollisions || m_RenderInfo.DebugCollisions.empty ())
			{
			return true;
			}

		if (cmd == VK_NULL_HANDLE)
			{
			LogError ( "RenderDebugWireFrame: CommandBuffer is NULL" );
			return false;
			}

		if (currentFrame >= m_FrameDescriptorSets.size ())
			{
			LogError ( "RenderWireframe: currentFrame ", currentFrame, " out of range (size: ", m_FrameDescriptorSets.size (), ")" );
			return false;
			}

		VkPipeline wireframePipeline = PipelineMgr->GetPipeline ( "WireframePipeline" );
		VkPipelineLayout wireframeLayout = PipelineMgr->GetPipelineLayout ( "WireframePipelineLayout" );

		if (wireframePipeline == VK_NULL_HANDLE)
			{
			LogError ( "RenderDebugWireFrame: wireframePipeline is NULL" );
			return false;
			}

		if (wireframeLayout == VK_NULL_HANDLE)
			{
			LogError ( "RenderDebugWireFrame: wireframeLayout is NULL" );
			return false;
			}

		vkCmdBindPipeline ( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, wireframePipeline );

		// Bind global descriptor set if valid
		VkDescriptorSet globalSet = VK_NULL_HANDLE;
		if (currentFrame < m_FrameDescriptorSets.size ())
			{
			globalSet = m_FrameDescriptorSets[ currentFrame ].GlobalSet;
			}

		if (globalSet != VK_NULL_HANDLE)
			{
			VkDescriptorSet sets[ 1 ] = { globalSet };
			vkCmdBindDescriptorSets (
				cmd,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				wireframeLayout,
				0, 1, sets,
				0, nullptr );
			}

		CWireframeGenerator generator;

		for (const auto & collision : m_RenderInfo .DebugCollisions)
			{
			if (!collision.IsValid ()) continue;

			std::vector<FWireframeVertex> vertices;

			switch (collision.ShapeType)
				{
					case ECollisionShape::SPHERE:
						generator.GenerateSphere ( vertices, collision.WorldLocation,
												   collision.Params.Sphere.Radius, collision.DebugColor );
						break;
					case ECollisionShape::BOX:
						generator.GenerateBox ( vertices, collision.WorldLocation,
												collision.WorldRotation, collision.Params.Box.HalfExtents, collision.DebugColor );
						break;
					case ECollisionShape::CAPSULE:
						generator.GenerateCapsule ( vertices, collision.WorldLocation,
													collision.WorldRotation, collision.Params.Capsule.Radius,
													collision.Params.Capsule.HalfHeight, collision.DebugColor );
						break;
					case ECollisionShape::CYLINDER:
						generator.GenerateCylinder ( vertices, collision.WorldLocation,
													 collision.WorldRotation, collision.Params.Cylinder.Radius,
													 collision.Params.Cylinder.Height, collision.DebugColor );
						break;
					case ECollisionShape::CONE:
						generator.GenerateCone ( vertices, collision.WorldLocation,
												 collision.WorldRotation, collision.Params.Cone.Radius,
												 collision.Params.Cone.Height, collision.DebugColor );
						break;
					default:
						continue;
				}

			if (vertices.empty ()) continue;

			struct FWireframePushConstants
				{
				glm::mat4x4 model;
				} wireframePush;

			wireframePush.model = CEMath::ToGLM ( FMat4 (
				collision.WorldLocation,
				collision.WorldRotation,
				collision.WorldScale ) );

			vkCmdPushConstants (
				cmd,
				wireframeLayout,
				VK_SHADER_STAGE_VERTEX_BIT,
				0,
				sizeof ( wireframePush ),
				&wireframePush );

			FBuffer tempBuffer = BuffMgr->CreateVertexBuffer ( vertices );

			if (tempBuffer.IsValid ())
				{
				VkBuffer vertexBuffers [] = { tempBuffer.Buffer };
				VkDeviceSize offsets [] = { 0 };
				vkCmdBindVertexBuffers ( cmd, 0, 1, vertexBuffers, offsets );
				vkCmdDraw ( cmd, static_cast< uint32_t >( vertices.size () ), 1, 0, 0 );

				
				BuffMgr->DestroyBuffer ( tempBuffer ); 
				}
			}
		return true;
		}
