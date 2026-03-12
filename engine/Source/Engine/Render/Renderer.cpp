#include "Render/Renderer.h"
#include "Core/EngineInfo.h"
#include "Core/Engine.h"
#include "Render/Vulkan/VulkanContext.h"
#include "Render/Vulkan/Managers/SwapChainManager.h"
#include "Render/Vulkan/Managers/CommandManager.h"
#include "Render/Vulkan/Managers/SyncManager.h"
#include "Render/Vulkan/Managers/RenderPassManager.h"
#include "Render/Vulkan/Managers/PipelineManager.h"
#include "Render/Vulkan/Managers/BufferManager.h"
#include "Render/Vulkan/Managers/DescriptorManager.h"
#include "Render/Vulkan/Managers/WireframePipeline.h"
#include "Render/RenderInfo.h"

CRenderer::CRenderer ( FEngineInfo & inInfo )
	: IVulkanManager ( inInfo )
	{}

CRenderer::~CRenderer ()
	{
	Shutdown ();
	LOG_DEBUG ( "Renderer Destroyed" );
	}

bool CRenderer::Initialize ()
	{
	LogDebug ( "Initializing Renderer..." );

	// Create Vulkan context
	m_Info.Vulkan.VulkanContext = std::make_unique<CVulkanContext> ( m_Info );
	if (!m_Info.Vulkan.VulkanContext->Initialize ())
		{
		LogError ( "Failed to initialize Vulkan Context" );
		return false;
		}

		// Cache all managers
	m_SwapChainManager = static_cast< CSwapChainManager * >( m_Info.Vulkan.SwapChainManager.get () );
	m_CommandManager = static_cast< CCommandManager * >( m_Info.Vulkan.CommandManager.get () );
	m_SyncManager = static_cast< CSyncManager * >( m_Info.Vulkan.SyncManager.get () );
	m_RenderPassManager = static_cast< CRenderPassManager * >( m_Info.Vulkan.RenderPassManager.get () );
	m_PipelineManager = static_cast< CPipelineManager * >( m_Info.Vulkan.PipelineManager.get () );
	m_BufferManager = static_cast< CBufferManager * >( m_Info.Vulkan.BufferManager.get () );
	m_DescriptorManager = static_cast< CDescriptorManager * >( m_Info.Vulkan.DescriptorManager.get () );

	if (!m_SwapChainManager || !m_CommandManager || !m_SyncManager ||
		 !m_RenderPassManager || !m_PipelineManager || !m_BufferManager || !m_DescriptorManager)
		{
		LogError ( "Failed to cache Vulkan managers" );
		return false;
		}

		// Pass DescriptorManager to PipelineManager
	m_PipelineManager->SetDescriptorManager ( m_DescriptorManager );

	// Cache descriptor set layouts for quick access
	m_GlobalLayout = m_DescriptorManager->GetGlobalLayout ();
	m_PerObjectLayout = m_DescriptorManager->GetPerObjectLayout ();

	if (m_GlobalLayout == VK_NULL_HANDLE || m_PerObjectLayout == VK_NULL_HANDLE)
		{
		LogError ( "Failed to get default descriptor set layouts" );
		return false;
		}

	VkRenderPass renderPass = m_RenderPassManager->GetMainRenderPass ();
	if (!m_PipelineManager->RegisterDefaultPipelines ( renderPass ))
		{
		LogError ( "Failed to register default pipelines" );
		return false;
		}

	m_TrianglePipeline = m_PipelineManager->GetPipeline ( "TrianglePipeline" );
	m_TrianglePipelineLayout = m_PipelineManager->GetPipelineLayout ( "TriangleLayout" );
	LOG_DEBUG ( "TrianglePipeline: ", ( void * ) m_TrianglePipeline );
	LOG_DEBUG ( "TrianglePipelineLayout: ", ( void * ) m_TrianglePipelineLayout );
	if (m_TrianglePipeline == VK_NULL_HANDLE)
		{
		LogError ( "Failed to get triangle pipeline" );
		return false;
		}

	m_TriangleVertexBuffer = m_BufferManager->CreateTriangleVertexBuffer ();
	if (!m_TriangleVertexBuffer.IsValid ())
		{
		LogError ( "Failed to create triangle vertex buffer" );
		return false;
		}

		// Create uniform buffers
	if (!CreateUniformBuffers ())
		{
		LogError ( "Failed to create uniform buffers" );
		return false;
		}

		// Create descriptor sets
	if (!CreateDescriptorSets ())
		{
		LogError ( "Failed to create descriptor sets" );
		return false;
		}

	if (!CreateCommandBuffers ())
		{
		LogError ( "Failed to create command buffers" );
		return false;
		}

	if (!CreateSyncObjects ())
		{
		LogError ( "Failed to create sync objects" );
		return false;
		}

	LogDebug ( "Renderer initialized successfully" );
	return true;
	}

void CRenderer::Shutdown ()
	{
	LogDebug ( "Renderer shutting down..." );

	auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
	if (deviceMgr)
		{
		VkDevice device = deviceMgr->GetDevice ();
		vkDeviceWaitIdle ( device );

		// Cleanup frame resources
		CleanupFrameResources ();

		m_TrianglePipelineLayout = VK_NULL_HANDLE;
		}

	m_CommandBuffers.clear ();
	m_SyncObjectsCreated = false;

	// Clear cached managers
	m_SwapChainManager = nullptr;
	m_CommandManager = nullptr;
	m_SyncManager = nullptr;
	m_RenderPassManager = nullptr;
	m_PipelineManager = nullptr;
	m_BufferManager = nullptr;
	m_DescriptorManager = nullptr;

	// Shutdown Vulkan context
	m_Info.Vulkan.Shutdown ();

	LogDebug ( "Renderer shutdown complete" );
	}

const char * CRenderer::GetManagerName () const
	{
	return "RENDERER";
	}

bool CRenderer::RenderScene ()
	{
	try
		{
		uint32_t imageIndex;

		// Start frame
		if (!StartFrame ( imageIndex ))
			{
			LOG_WARN ( "[", GetManagerName (), "] StartFrame failed, will recreate resources" );

			// Wait for all operations to complete
			auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
			vkDeviceWaitIdle ( deviceMgr->GetDevice () );

			// Recreate resources
			if (!RecreateSwapChainResources ())
				{
				LogError ( "Failed to recreate swapchain resources" );
				return false;
				}

			if (!StartFrame ( imageIndex ))
				{
				LogError ( "Failed to start frame after recreation" );
				return false;
				}
			}

		if (!EndFrame ( imageIndex ))
			{
			LogDebug ( "EndFrame failed" );
			}

		return true;
		}
		catch (const std::exception & e)
			{
			LogError ( "Exception in RenderScene: ", e.what () );
			return false;
			}
		catch (...)
			{
			LogError ( "Unknown exception in RenderScene" );
			return false;
			}
	}

bool CRenderer::StartFrame ( uint32_t & ImageIndex )
	{
	uint32_t currentFrame = m_SyncManager->GetCurrentFrame ();

	VkFence fence = m_SyncManager->GetInFlightFence ( currentFrame );
	if (fence == VK_NULL_HANDLE)
		{
		LogError ( "Invalid fence for frame ", currentFrame );
		return false;
		}

	VkResult result = m_SyncManager->WaitForFence ( fence, UINT64_MAX );
	if (result != VK_SUCCESS)
		{
		LogError ( "Failed to wait for fence: ", static_cast< int >( result ) );
		return false;
		}

	m_SyncManager->ResetFence ( fence );

	VkSemaphore imageAvailableSemaphore = m_SyncManager->GetImageAvailableSemaphore ( currentFrame );
	if (!m_SwapChainManager->AcquireNextImage ( imageAvailableSemaphore, ImageIndex ))
		{
		return false;
		}

	if (m_SwapChainManager->IsSwapChainRecreated ())
		{
		if (RecreateSwapChainResources ())
			m_SwapChainManager->SetSwapChainRecreated ( false );
		}

	return true;
	}

bool CRenderer::EndFrame ( uint32_t ImageIndex )
	{
	uint32_t currentFrame = m_SyncManager->GetCurrentFrame ();


	// ПРОВЕРКА: Убедимся, что у нас есть валидные дескрипторы перед обновлением
	if (currentFrame >= m_FrameDescriptorSets.size () ||
		 m_FrameDescriptorSets[ currentFrame ].GlobalSet == VK_NULL_HANDLE)
		{
		LogError ( "EndFrame: Invalid descriptor set for frame ", currentFrame );
		return false;
		}

	// Update uniform buffers before recording commands
	UpdateUniformBuffers ( currentFrame );

	VkSemaphore imageAvailableSemaphore = m_SyncManager->GetImageAvailableSemaphore ( currentFrame );
	VkSemaphore renderFinishedSemaphore = m_SyncManager->GetRenderFinishedSemaphore ( currentFrame );
	VkFence fence = m_SyncManager->GetInFlightFence ( currentFrame );

	if (imageAvailableSemaphore == VK_NULL_HANDLE)
		{
		LogError ( "  imageAvailableSemaphore is NULL" );
		return false;
		}
	if (renderFinishedSemaphore == VK_NULL_HANDLE)
		{
		LogError ( "  renderFinishedSemaphore is NULL" );
		return false;
		}
	if (fence == VK_NULL_HANDLE)
		{
		LogError ( "  fence is NULL" );
		return false;
		}

	if (ImageIndex >= m_CommandBuffers.size ())
		{
		LogError ( "Image index out of range" );
		return false;
		}

	VkCommandBuffer commandBuffer = m_CommandBuffers[ ImageIndex ];
	if (commandBuffer == VK_NULL_HANDLE)
		{
		LogError ( "  commandBuffer is NULL" );
		return false;
		}

		// Record commands
	if (!RecordCommandBuffer ( commandBuffer, ImageIndex ))
		{
		LogError ( "Failed to record command buffer" );
		return false;
		}

		// Submit command buffer
	auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
	if (!deviceMgr)
		{
		LogError ( "  deviceMgr is NULL" );
		return false;
		}

	VkQueue graphicsQueue = deviceMgr->GetGraphicsQueue ();
	if (graphicsQueue == VK_NULL_HANDLE)
		{
		LogError ( "  graphicsQueue is NULL" );
		return false;
		}

	VkSubmitInfo submitInfo {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags waitStages [] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

	VkResult result = vkQueueSubmit ( graphicsQueue, 1, &submitInfo, fence );

	if (result != VK_SUCCESS)
		{
		LogError ( "Failed to submit command buffer: ", static_cast< int >( result ) );
		return false;
		}

	if (!m_SwapChainManager->Present ( renderFinishedSemaphore, ImageIndex ))
		{
		LogDebug ( "  Present failed" );
		return false;
		}

	m_SyncManager->NextFrame ();

	return true;
	}

bool CRenderer::CreateSyncObjects ()
	{
	if (m_SyncObjectsCreated) return true;

	uint32_t imageCount = m_SwapChainManager->GetImageCount ();
	if (!m_SyncManager->CreateFrameSyncObjects ( imageCount ))
		{
		LogError ( "Failed to create frame sync objects" );
		return false;
		}

	m_SyncObjectsCreated = true;
	LogDebug ( "Created sync objects for ", imageCount, " frames" );
	return true;
	}

bool CRenderer::CreateCommandBuffers ()
	{
	uint32_t imageCount = m_SwapChainManager->GetImageCount ();

	m_CommandBuffers = m_CommandManager->CreateCommandBuffers ( imageCount );
	if (m_CommandBuffers.size () != imageCount)
		{
		LogError ( "Failed to create command buffers" );
		return false;
		}

	return true;
	}

bool CRenderer::CreateUniformBuffers ()
	{
	uint32_t imageCount = m_SwapChainManager->GetImageCount ();
	m_FrameUniformBuffers.resize ( imageCount );

	LogDebug ( "Creating uniform buffers for ", imageCount, " frames..." );

	for (uint32_t i = 0; i < imageCount; i++)
		{
			// View/projection uniform buffer
		m_FrameUniformBuffers[ i ].ViewProjectionBuffer =
			m_BufferManager->CreateUniformBuffer ( VIEW_PROJ_BUFFER_SIZE );

		if (!m_FrameUniformBuffers[ i ].ViewProjectionBuffer.IsValid ())
			{
			LogError ( "Failed to create view/projection uniform buffer for frame ", i );
			return false;
			}

			// Dynamic object uniform buffer
		m_FrameUniformBuffers[ i ].ObjectBuffer =
			m_BufferManager->CreateBuffer (
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
			m_BufferManager->MapBuffer ( m_FrameUniformBuffers[ i ].ObjectBuffer );

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


bool CRenderer::CreateDescriptorSets ()
	{
	uint32_t imageCount = m_SwapChainManager->GetImageCount ();
	m_FrameDescriptorSets.resize ( imageCount );

	LogDebug ( "Creating descriptor sets for ", imageCount, " frames..." );

	// Проверяем, что пул существует
	VkDescriptorPool perFramePool = m_DescriptorManager->GetDescriptorPool ( "PerFramePool" );
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
			m_DescriptorManager->AllocateDescriptorSet ( "PerFramePool", m_GlobalLayout );

		if (m_FrameDescriptorSets[ i ].GlobalSet == VK_NULL_HANDLE)
			{
			LogError ( "Failed to allocate global descriptor set for frame ", i );

			// Пытаемся сбросить пул и попробовать снова
			LogDebug ( "  Resetting PerFramePool and retrying..." );
			m_DescriptorManager->ResetDescriptorPool ( "PerFramePool" );

			m_FrameDescriptorSets[ i ].GlobalSet =
				m_DescriptorManager->AllocateDescriptorSet ( "PerFramePool", m_GlobalLayout );

			if (m_FrameDescriptorSets[ i ].GlobalSet == VK_NULL_HANDLE)
				{
				LogError ( "Still failed to allocate global descriptor set after pool reset" );
				return false;
				}
			}

		LogDebug ( "  Updating global descriptor set for frame ", i,
				   " with buffer ", ( void * ) m_FrameUniformBuffers[ i ].ViewProjectionBuffer.Buffer );

		  // Update global descriptor with view/projection buffer
		m_DescriptorManager->UpdateBufferDescriptor (
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


void CRenderer::UpdateUniformBuffers ( uint32_t FrameIndex )
	{
	auto Info = CEngine::Get ().GetRenderInfo ();
	if (!Info->IsValid () || FrameIndex >= m_FrameUniformBuffers.size ()) return;

	auto & frameUB = m_FrameUniformBuffers[ FrameIndex ];

	// Update view/projection uniform buffer
	FViewProjectionUniform viewProj;
	viewProj.View = CEMath::ToGLM ( Info->Camera.GetViewMatrix () );
	viewProj.Projection = CEMath::ToGLM ( Info->Camera.GetProjectionMatrix () );

	m_BufferManager->UpdateUniformBuffer (
		frameUB.ViewProjectionBuffer,
		&viewProj,
		sizeof ( viewProj ) );

	// Update object uniform buffer with all meshes
	const auto & meshes = Info->RenderMeshes;
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

void CRenderer::CleanupFrameResources ()
	{
	auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
	if (!deviceMgr) return;

	VkDevice device = deviceMgr->GetDevice ();

	// Destroy uniform buffers
	for (auto & frame : m_FrameUniformBuffers)
		{
		if (frame.MappedObjectData)
			{
			m_BufferManager->UnmapBuffer ( frame.ObjectBuffer );
			}
		m_BufferManager->DestroyBuffer ( frame.ViewProjectionBuffer );
		m_BufferManager->DestroyBuffer ( frame.ObjectBuffer );
		}
	m_FrameUniformBuffers.clear ();

	// Descriptor sets will be freed when pools are reset/destroyed
	m_FrameDescriptorSets.clear ();
	}

bool CRenderer::RecordCommandBuffer ( VkCommandBuffer CommandBuffer, uint32_t ImageIndex )
	{
	auto Info = CEngine::Get ().GetRenderInfo ();
	if (!Info->HasInfo)
		{
		TriangleStub ( CommandBuffer, ImageIndex );
		vkCmdEndRenderPass ( CommandBuffer );
		m_CommandManager->EndCommandBuffer ( CommandBuffer );
		}
	else
		{
		RenderWorld ( CommandBuffer, ImageIndex );
		}
	return true;
	}


bool CRenderer::RecreateSwapChainResources ()
	{
	LogDebug ( "Recreating swapchain resources..." );

	auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
	if (!deviceMgr) return false;

	vkDeviceWaitIdle ( deviceMgr->GetDevice () );

	// Cleanup old frame resources
	CleanupFrameResources ();

	// Clear old command buffers
	if (!m_CommandBuffers.empty ())
		{
		m_CommandManager->FreeCommandBuffers ( m_CommandBuffers );
		m_CommandBuffers.clear ();
		}

		// Recreate render pass
	LogDebug ( "  Calling RenderPassManager->RecreateForSwapChain()..." );
	if (!m_RenderPassManager->RecreateForSwapChain ())
		{
		LogError ( "Failed to recreate render pass" );
		return false;
		}

		// Recreate uniform buffers
	if (!CreateUniformBuffers ())
		{
		LogError ( "Failed to recreate uniform buffers" );
		return false;
		}

		// Recreate descriptor sets
	if (!CreateDescriptorSets ())
		{
		LogError ( "Failed to recreate descriptor sets" );
		return false;
		}

		// ПРОВЕРКА: Убедимся, что дескрипторы созданы правильно
	for (uint32_t i = 0; i < m_FrameDescriptorSets.size (); i++)
		{
		if (m_FrameDescriptorSets[ i ].GlobalSet == VK_NULL_HANDLE)
			{
			LogError ( "RecreateSwapChainResources: GlobalSet for frame ", i, " is still null after creation!" );
			return false;
			}
		}

		// Recreate command buffers
	uint32_t imageCount = m_SwapChainManager->GetImageCount ();
	LogDebug ( "  Recreating command buffers for ", imageCount, " images..." );
	m_CommandBuffers = m_CommandManager->CreateCommandBuffers ( imageCount );

	if (m_CommandBuffers.size () != imageCount)
		{
		LogError ( "Failed to recreate command buffers" );
		return false;
		}

	LogDebug ( "Swapchain resources recreated successfully" );
	return true;
	}


void CRenderer::RenderWorld ( VkCommandBuffer CommandBuffer, uint32_t ImageIndex )
	{
	auto Info = CEngine::Get ().GetRenderInfo ();

	if (!Info->IsValid ())
		{
		LOG_WARN ( "RenderWorld called but no valid render info" );
		return;
		}

	VkExtent2D extent = m_SwapChainManager->GetExtent ();
	if (extent.width == 0 || extent.height == 0)
		{
		LogError ( "Invalid swapchain extent" );
		return;
		}

	m_CommandManager->BeginCommandBuffer ( CommandBuffer );


	VkRenderPassBeginInfo renderPassInfo {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_RenderPassManager->GetMainRenderPass ();
	renderPassInfo.framebuffer = m_RenderPassManager->GetFramebuffer ( ImageIndex );
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = extent;

	VkClearValue clearValues[ 2 ];
	clearValues[ 0 ].color = { {0.2f, 0.3f, 0.4f, 1.0f} };
	clearValues[ 1 ].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass ( CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );


	VkViewport viewport {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast< float >( extent.width );
	viewport.height = static_cast< float >( extent.height );
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport ( CommandBuffer, 0, 1, &viewport );

	VkRect2D scissor {};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;
	vkCmdSetScissor ( CommandBuffer, 0, 1, &scissor );


	RenderMeshes ( CommandBuffer, ImageIndex );
	RenderTerrain ( CommandBuffer, ImageIndex );
	RenderDebugWireFrame ( CommandBuffer, ImageIndex );

	vkCmdEndRenderPass ( CommandBuffer );
	m_CommandManager->EndCommandBuffer ( CommandBuffer );
	}


void CRenderer::RenderMeshes ( VkCommandBuffer CommandBuffer, uint32_t ImageIndex )
	{
	auto Info = CEngine::Get ().GetRenderInfo ();
	uint32_t currentFrame = m_SyncManager->GetCurrentFrame ();

	// Проверки
	if (currentFrame >= m_FrameDescriptorSets.size ())
		{
		LogError ( "RenderMeshes: currentFrame out of range" );
		return;
		}

	if (CommandBuffer == VK_NULL_HANDLE)
		{
		LogError ( "RenderMeshes: CommandBuffer is VK_NULL_HANDLE" );
		return;
		}

	VkDescriptorSet globalSet = m_FrameDescriptorSets[ currentFrame ].GlobalSet;
	if (globalSet == VK_NULL_HANDLE)
		{
		LogError ( "RenderMeshes: globalSet is VK_NULL_HANDLE for frame ", currentFrame );
		return;
		}



	const auto & meshes = Info->RenderMeshes;

	for (size_t i = 0; i < meshes.size (); i++)
		{
		const FMeshInfo & mesh = meshes[ i ];

		if (!mesh.IsValid ())
			{
			LOG_WARN ( "Skipping invalid mesh at index ", i );
			continue;
			}

		VkPipeline pipeline = m_PipelineManager->GetPipeline ( mesh.PipelineName );
		if (pipeline == VK_NULL_HANDLE)
			{
			LOG_ERROR ( "Pipeline not found for: ", mesh.PipelineName );
			continue;
			}

		VkPipelineLayout layout = m_PipelineManager->GetPipelineLayout ( mesh.PipelineName + "Layout" );
		if (layout == VK_NULL_HANDLE)
			{
			LOG_ERROR ( "Layout not found for: ", mesh.PipelineName );
			continue;
			}

		vkCmdBindPipeline ( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );


		VkDescriptorSet sets[ 1 ] = { globalSet };

		vkCmdBindDescriptorSets (
			CommandBuffer,
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
			CommandBuffer,
			layout,
			VK_SHADER_STAGE_VERTEX_BIT,  // Только vertex shader
			0,                            // offset
			sizeof ( modelPush ),            // size
			&modelPush );                   // data

		// Bind vertex buffers
		VkBuffer vertexBuffers [] = { mesh.VertexBuffer };
		VkDeviceSize offsets [] = { 0 };
		vkCmdBindVertexBuffers ( CommandBuffer, 0, 1, vertexBuffers, offsets );

		if (mesh.IndexBuffer != VK_NULL_HANDLE && mesh.IndexCount > 0)
			{
			vkCmdBindIndexBuffer ( CommandBuffer, mesh.IndexBuffer, 0, VK_INDEX_TYPE_UINT32 );
			vkCmdDrawIndexed ( CommandBuffer, mesh.IndexCount, 1, 0, 0, 0 );
			}
		else
			{
			vkCmdDraw ( CommandBuffer, mesh.VertexCount, 1, 0, 0 );
			}
		}
	}



void CRenderer::RenderDebugWireFrame ( VkCommandBuffer CommandBuffer, uint32_t ImageIndex )
	{
	auto Info = CEngine::Get ().GetRenderInfo ();
	uint32_t currentFrame = m_SyncManager->GetCurrentFrame ();

	if (!Info->bDrawCollisions || Info->DebugCollisions.empty ())
		{
		return;
		}

	if (CommandBuffer == VK_NULL_HANDLE)
		{
		LogError ( "RenderDebugWireFrame: CommandBuffer is NULL" );
		return;
		}

	VkPipeline wireframePipeline = m_PipelineManager->GetPipeline ( "WireframePipeline" );
	VkPipelineLayout wireframeLayout = m_PipelineManager->GetPipelineLayout ( "WireframePipelineLayout" );

	if (wireframePipeline == VK_NULL_HANDLE)
		{
		LogError ( "RenderDebugWireFrame: wireframePipeline is NULL" );
		return;
		}

	if (wireframeLayout == VK_NULL_HANDLE)
		{
		LogError ( "RenderDebugWireFrame: wireframeLayout is NULL" );
		return;
		}

	vkCmdBindPipeline ( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, wireframePipeline );

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
			CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			wireframeLayout,
			0, 1, sets,
			0, nullptr );
		}

	CWireframeGenerator generator;

	for (const auto & collision : Info->DebugCollisions)
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
			CommandBuffer,
			wireframeLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof ( wireframePush ),
			&wireframePush );

		FBuffer tempBuffer = m_BufferManager->CreateVertexBuffer ( vertices );

		if (tempBuffer.IsValid ())
			{
			VkBuffer vertexBuffers [] = { tempBuffer.Buffer };
			VkDeviceSize offsets [] = { 0 };
			vkCmdBindVertexBuffers ( CommandBuffer, 0, 1, vertexBuffers, offsets );
			vkCmdDraw ( CommandBuffer, static_cast< uint32_t >( vertices.size () ), 1, 0, 0 );

			// Важно: не забываем уничтожить временный буфер
			m_BufferManager->DestroyBuffer ( tempBuffer ); // Нужно добавить этот метод
			}
		}
	}

void CRenderer::RenderTerrain ( VkCommandBuffer CommandBuffer, uint32_t ImageIndex )
	{
	auto Info = CEngine::Get ().GetRenderInfo ();

	if (Info->Terrains.empty ())
		{
		LOG_DEBUG ( "No terrains to render" );
		return;
		}

	uint32_t currentFrame = m_SyncManager->GetCurrentFrame ();

	// Pipeline checks
	VkPipeline terrainPipeline = m_PipelineManager->GetPipeline ( "TerrainPipeline" );
	VkPipelineLayout terrainLayout = m_PipelineManager->GetPipelineLayout ( "TerrainLayout" );

	if (terrainPipeline == VK_NULL_HANDLE || terrainLayout == VK_NULL_HANDLE)
		{
		LOG_ERROR ( "Pipeline or layout NULL" );
		return;
		}

	vkCmdBindPipeline ( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, terrainPipeline );

	// Descriptor sets
	VkDescriptorSet globalSet = VK_NULL_HANDLE;
	if (currentFrame < m_FrameDescriptorSets.size ())
		globalSet = m_FrameDescriptorSets[ currentFrame ].GlobalSet;


	if (globalSet != VK_NULL_HANDLE)
		{
		VkDescriptorSet sets[ 1 ] = { globalSet };
		vkCmdBindDescriptorSets ( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								  terrainLayout, 0, 1, sets, 0, nullptr );

		}

	const auto & terrains = Info->Terrains;


	for (size_t i = 0; i < terrains.size (); i++)
		{

		const FTerrainRenderInfo & terrain = terrains[ i ];


		if (!terrain.IsValid ())
			{
			LOG_WARN ( "Invalid terrain, skipping" );
			continue;
			}

		if (terrain.VertexBuffer == VK_NULL_HANDLE)
			{
			LOG_ERROR ( "Vertex buffer NULL" );
			continue;
			}


		struct FTerrainPushConstants
			{
			glm::mat4x4 model;
			glm::vec4 terrainParams;
			} pushConstants;

		pushConstants.model = CEMath::ToGLM ( terrain.Model );
		pushConstants.terrainParams = glm::vec4 ( 1.0f, 1.0f, 0.001f, 1.0f );

		vkCmdPushConstants ( CommandBuffer, terrainLayout,
							 VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
							 0, sizeof ( pushConstants ), &pushConstants );



		// Bind vertex buffer
		VkBuffer vertexBuffers [] = { terrain.VertexBuffer };
		VkDeviceSize offsets [] = { 0 };
		vkCmdBindVertexBuffers ( CommandBuffer, 0, 1, vertexBuffers, offsets );


		// Draw
		if (terrain.IndexBuffer != VK_NULL_HANDLE && terrain.IndexCount > 0)
			{
			vkCmdBindIndexBuffer ( CommandBuffer, terrain.IndexBuffer, 0, VK_INDEX_TYPE_UINT32 );

			vkCmdDrawIndexed ( CommandBuffer, terrain.IndexCount, 1, 0, 0, 0 );

			}
		else
			{
			vkCmdDraw ( CommandBuffer, terrain.VertexCount, 1, 0, 0 );

			}
		}


	}


void CRenderer::TriangleStub ( VkCommandBuffer CommandBuffer, uint32_t ImageIndex )
	{
	LOG_DEBUG ( "========== TRIANGLE STUB DEBUG START ==========" );
	LOG_DEBUG ( "ImageIndex: ", ImageIndex );
	LOG_DEBUG ( "CommandBuffer: ", ( void * ) CommandBuffer );
	LOG_DEBUG ( "m_TrianglePipeline: ", ( void * ) m_TrianglePipeline );
	LOG_DEBUG ( "m_TrianglePipelineLayout: ", ( void * ) m_TrianglePipelineLayout );
	LOG_DEBUG ( "m_TriangleVertexBuffer.Buffer: ", ( void * ) m_TriangleVertexBuffer.Buffer );
	LOG_DEBUG ( "m_TriangleVertexBuffer.IsValid(): ", m_TriangleVertexBuffer.IsValid () );

	static int WarnCount = 0;
	if (WarnCount < 1)
		{
		LOG_WARN ( "Nothing to render Call Fallback Triangle" );
		WarnCount++;
		}

	if (CommandBuffer == VK_NULL_HANDLE)
		{
		LogError ( "RecordCommandBuffer: CommandBuffer is null" );
		return;
		}

	if (!m_TriangleVertexBuffer.IsValid ())
		{
		LogError ( "RecordCommandBuffer: m_TriangleVertexBuffer is invalid" );
		return;
		}

	if (m_TrianglePipeline == VK_NULL_HANDLE)
		{
		LogError ( "RecordCommandBuffer: m_TrianglePipeline is null" );
		return;
		}

	VkRenderPass renderPass = m_RenderPassManager->GetMainRenderPass ();
	LOG_DEBUG ( "RenderPass: ", ( void * ) renderPass );

	if (renderPass == VK_NULL_HANDLE)
		{
		LogError ( "RecordCommandBuffer: Main render pass is null" );
		return;
		}

	VkFramebuffer framebuffer = m_RenderPassManager->GetFramebuffer ( ImageIndex );
	LOG_DEBUG ( "Framebuffer: ", ( void * ) framebuffer );

	if (framebuffer == VK_NULL_HANDLE)
		{
		LogError ( "RecordCommandBuffer: Framebuffer for image ", ImageIndex, " is null" );
		return;
		}

	VkExtent2D extent = m_SwapChainManager->GetExtent ();
	LOG_DEBUG ( "Extent: ", extent.width, "x", extent.height );

	if (extent.width == 0 || extent.height == 0)
		{
		LogError ( "RecordCommandBuffer: SwapChain extent is invalid: ", extent.width, "x", extent.height );
		return;
		}

	m_CommandManager->BeginCommandBuffer ( CommandBuffer );

	// Begin render pass
	VkRenderPassBeginInfo renderPassInfo {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = framebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = extent;

	VkClearValue clearValues[ 2 ];
	clearValues[ 0 ].color = { {0.2f, 0.2f, 0.8f, 1.0f} };
	clearValues[ 1 ].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass ( CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
	LOG_DEBUG ( "Render pass begun" );

	// Bind pipeline
	vkCmdBindPipeline ( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_TrianglePipeline );
	LOG_DEBUG ( "Pipeline bound" );

	// Bind descriptor sets
	uint32_t currentFrame = m_SyncManager->GetCurrentFrame ();
	LOG_DEBUG ( "Current frame: ", currentFrame );

	VkDescriptorSet globalSet = VK_NULL_HANDLE;
	if (currentFrame < m_FrameDescriptorSets.size ())
		{
		globalSet = m_FrameDescriptorSets[ currentFrame ].GlobalSet;
		LOG_DEBUG ( "Global set from frame ", currentFrame, ": ", ( void * ) globalSet );
		}
	else
		{
		LOG_ERROR ( "currentFrame out of range: ", currentFrame, " >= ", m_FrameDescriptorSets.size () );
		}


	VkBuffer vertexBuffers [] = { m_TriangleVertexBuffer.Buffer };
	VkDeviceSize offsets [] = { 0 };
	vkCmdBindVertexBuffers ( CommandBuffer, 0, 1, vertexBuffers, offsets );
	LOG_DEBUG ( "Vertex buffer bound" );

	// Set viewport
	VkViewport viewport {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast< float >( extent.width );
	viewport.height = static_cast< float >( extent.height );
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport ( CommandBuffer, 0, 1, &viewport );
	LOG_DEBUG ( "Viewport set: ", viewport.width, "x", viewport.height );

	// Set scissor
	VkRect2D scissor {};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;
	vkCmdSetScissor ( CommandBuffer, 0, 1, &scissor );
	LOG_DEBUG ( "Scissor set" );

	// Draw triangle
	vkCmdDraw ( CommandBuffer, 3, 1, 0, 0 );
	LOG_DEBUG ( "Draw called with 3 vertices" );

	LOG_DEBUG ( "========== TRIANGLE STUB DEBUG END ==========" );
	}