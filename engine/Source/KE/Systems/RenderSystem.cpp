#include "KE/Systems/RenderSystem.h"
#include "KE/Systems/WindowSystem.h"
#include "KE/Vulkan/VulkanContext.h"  
#include "KE/Engine.h"
#include "KE/Vulkan/Managers/SwapchainManager.h"
#include "KE/Vulkan/Managers/CommandManager.h"
#include "KE/Vulkan/Managers/PipelineManager.h"
#include "KE/Vulkan/Managers/RenderPassManager.h"
#include "KE/Vulkan/Managers/BufferManager.h"
#include "KE/Vulkan/Managers/DeviceManager.h"
#include "KE/Vulkan/Managers/SyncManager.h"
#include "KE/Vulkan/VKinfo.h"
#include "KE/Vulkan/RenderInfo.h"

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
    }



void RenderSystem::Shutdown ()
    {
    LogInfo ( "Shutting down RenderSystem" );

   // Ждем завершения всех GPU операций перед очисткой
    auto * deviceMgr = static_cast< DeviceManager * >( m_vulkanContext->GetInfo ()->Managers.DeviceManager.get () );
    if (deviceMgr)
        {
        vkDeviceWaitIdle ( deviceMgr->GetDevice () );
        }

        // Очищаем вершинный буфер
    auto * bufferMgr = static_cast< BufferManager * >( m_vulkanContext->GetInfo ()->Managers.BufferManager.get () );
    if (bufferMgr && m_triangleVertexBuffer.IsValid ())
        {
        bufferMgr->DestroyBuffer ( m_triangleVertexBuffer );
        LogDebug ( "Triangle vertex buffer destroyed" );
        BuffMgr->Shutdown ();
        }

    m_vulkanContext->Shutdown ();
    m_vulkanContext.reset ();
    bIsInitialized = false;
    }

void RenderSystem::Update ( float DeltaTime )
    {
    if (!IsInitialized ()) return;

   
    if (m_RenderInfo.HasInfo && m_RenderInfo.IsValid ())
        {
        
        RenderScene ();
        }
    else
        {
       
        RenderTriangle ();
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

    bool allValid = Swapchain && CmdManager && PipelineMgr && RenderPassMgr && SyncMgr && BuffMgr;

    if (!allValid)
        {
        if (!Swapchain) LogError ( "SwapchainManager not available" );
        if (!CmdManager) LogError ( "CommandManager not available" );
        if (!PipelineMgr) LogError ( "PipelineManager not available" );
        if (!RenderPassMgr) LogError ( "RenderPassManager not available" );
        if (!SyncMgr) LogError ( "SyncManager not available" );
        if (!BuffMgr) LogError ( "BufferManager not available" );
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
            // TODO: Полная реализация рендера сцены
            // Здесь будет рендеринг:
            // - Мешей из m_RenderInfo.RenderMeshes
            // - Террейнов из m_RenderInfo.Terrains
            // - Debug коллизий из m_RenderInfo.DebugCollisions
            // - Wireframe террейнов из m_RenderInfo.TerrainWireframes
        static int tracecount = 0;
        if(tracecount <1)
            {
            LogTrace ( "Rendering scene with ",
                       m_RenderInfo.GetMeshCount (), " meshes, ",
                       m_RenderInfo.GetTerrainCount (), " terrains, ",
                       m_RenderInfo.GetDebugCollisionCount (), " debug collisions, ",
                       m_RenderInfo.GetTerrainWireframeCount (), " terrain wireframes" );
            tracecount++;
            }

        // Пока возвращаем успех, но ничего не рендерим
        // Полная реализация будет добавлена позже
        return true;
        }