#include "Render/Vulkan/Managers/SyncManager.h"
#include "Core/EngineInfo.h"
#include "Render/Vulkan/Managers/DeviceManager.h"

CSyncManager::CSyncManager ( FEngineInfo & inInfo )
    : IVulkanManager ( inInfo )
    {}

CSyncManager::~CSyncManager ()
    {
    Shutdown ();
    LogDebug ( GetManagerName (), " destroyed" );
    }

bool CSyncManager::Initialize ()
    {
    LogDebug ( "Initializing SyncManager..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr || !deviceMgr->IsInitialized ())
        {
        LogError ( "DeviceManager not initialized" );
        return false;
        }

    LogDebug ( "SyncManager initialized successfully" );
    m_bInitialized = true;
    return true;
    }

void CSyncManager::Shutdown ()
    {
    if (!m_bInitialized) return;

    LogDebug ( "Shutting down SyncManager..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (deviceMgr)
        {
        VkDevice device = deviceMgr->GetDevice ();

        // Destroy frame sync objects
        for (auto & frame : m_FrameSyncObjects)
            {
            if (frame.ImageAvailable != VK_NULL_HANDLE)
                {
                vkDestroySemaphore ( device, frame.ImageAvailable, nullptr );
                frame.ImageAvailable = VK_NULL_HANDLE;
                }
            if (frame.RenderFinished != VK_NULL_HANDLE)
                {
                vkDestroySemaphore ( device, frame.RenderFinished, nullptr );
                frame.RenderFinished = VK_NULL_HANDLE;
                }
            if (frame.InFlight != VK_NULL_HANDLE)
                {
                vkDestroyFence ( device, frame.InFlight, nullptr );
                frame.InFlight = VK_NULL_HANDLE;
                }
            }
        m_FrameSyncObjects.clear ();
        }

    m_FramesInFlight = 0;
    m_CurrentFrame = 0;
    m_bInitialized = false;
    LogDebug ( "SyncManager shutdown complete" );
    }

const char * CSyncManager::GetManagerName () const
    {
    return "SyncManager";
    }

VkSemaphore CSyncManager::CreateSemaphore ()
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkSemaphoreCreateInfo semaphoreInfo {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaphore;
    VkResult result = vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &semaphore );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create semaphore: ", static_cast< int >( result ) );
        return VK_NULL_HANDLE;
        }

    return semaphore;
    }

VkFence CSyncManager::CreateFence ( bool bCreateSignaled )
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (bCreateSignaled)
        {
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }

    VkFence fence;
    VkResult result = vkCreateFence ( device, &fenceInfo, nullptr, &fence );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create fence: ", static_cast< int >( result ) );
        return VK_NULL_HANDLE;
        }

    return fence;
    }

bool CSyncManager::CreateFrameSyncObjects ( uint32_t FramesInFlight )
    {
    LogDebug ( "Creating frame sync objects for ", FramesInFlight, " frames in flight" );

    if (FramesInFlight == 0)
        {
        LogError ( "Frames in flight cannot be 0" );
        return false;
        }

    // Clean up old objects if any
    if (!m_FrameSyncObjects.empty ())
        {
        auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
        VkDevice device = deviceMgr->GetDevice ();

        for (auto & frame : m_FrameSyncObjects)
            {
            if (frame.ImageAvailable != VK_NULL_HANDLE)
                vkDestroySemaphore ( device, frame.ImageAvailable, nullptr );
            if (frame.RenderFinished != VK_NULL_HANDLE)
                vkDestroySemaphore ( device, frame.RenderFinished, nullptr );
            if (frame.InFlight != VK_NULL_HANDLE)
                vkDestroyFence ( device, frame.InFlight, nullptr );
            }
        m_FrameSyncObjects.clear ();
        }

    m_FramesInFlight = FramesInFlight;
    m_FrameSyncObjects.resize ( FramesInFlight );

    for (uint32_t i = 0; i < FramesInFlight; i++)
        {
        // Create image available semaphore (signaled when swapchain image is ready)
        m_FrameSyncObjects[ i ].ImageAvailable = CreateSemaphore ();
        if (m_FrameSyncObjects[ i ].ImageAvailable == VK_NULL_HANDLE)
            {
            LogError ( "Failed to create image available semaphore for frame ", i );
            return false;
            }

        // Create render finished semaphore (signaled when rendering is complete)
        m_FrameSyncObjects[ i ].RenderFinished = CreateSemaphore ();
        if (m_FrameSyncObjects[ i ].RenderFinished == VK_NULL_HANDLE)
            {
            LogError ( "Failed to create render finished semaphore for frame ", i );
            return false;
            }

        // Create in-flight fence (signaled when frame is complete)
        m_FrameSyncObjects[ i ].InFlight = CreateFence ( true ); // Create signaled
        if (m_FrameSyncObjects[ i ].InFlight == VK_NULL_HANDLE)
            {
            LogError ( "Failed to create in-flight fence for frame ", i );
            return false;
            }
        }

    LogDebug ( "Successfully created ", FramesInFlight, " frame sync objects" );
    return true;
    }

VkSemaphore CSyncManager::GetImageAvailableSemaphore ( uint32_t FrameIndex ) const
    {
    if (FrameIndex >= m_FrameSyncObjects.size ())
        {
        LogError ( "Frame index ", FrameIndex, " out of range (max: ", m_FrameSyncObjects.size () - 1, ")" );
        return VK_NULL_HANDLE;
        }
    return m_FrameSyncObjects[ FrameIndex ].ImageAvailable;
    }

VkSemaphore CSyncManager::GetRenderFinishedSemaphore ( uint32_t FrameIndex ) const
    {
    if (FrameIndex >= m_FrameSyncObjects.size ())
        {
        LogError ( "Frame index ", FrameIndex, " out of range (max: ", m_FrameSyncObjects.size () - 1, ")" );
        return VK_NULL_HANDLE;
        }
    return m_FrameSyncObjects[ FrameIndex ].RenderFinished;
    }

VkFence CSyncManager::GetInFlightFence ( uint32_t FrameIndex ) const
    {
    if (FrameIndex >= m_FrameSyncObjects.size ())
        {
        LogError ( "Frame index ", FrameIndex, " out of range (max: ", m_FrameSyncObjects.size () - 1, ")" );
        return VK_NULL_HANDLE;
        }
    return m_FrameSyncObjects[ FrameIndex ].InFlight;
    }

VkResult CSyncManager::WaitForFence ( VkFence Fence, uint64_t Timeout )
    {
    if (Fence == VK_NULL_HANDLE)
        {
        LogError ( "Cannot wait for null fence" );
        return VK_ERROR_UNKNOWN;
        }

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    return vkWaitForFences ( device, 1, &Fence, VK_TRUE, Timeout );
    }

VkResult CSyncManager::WaitForFences ( const std::vector<VkFence> & Fences, bool bWaitAll, uint64_t Timeout )
    {
    if (Fences.empty ())
        {
        LogError ( "Cannot wait for empty fences list" );
        return VK_ERROR_UNKNOWN;
        }

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    return vkWaitForFences ( device, static_cast< uint32_t >( Fences.size () ),
                             Fences.data (), bWaitAll ? VK_TRUE : VK_FALSE, Timeout );
    }

void CSyncManager::ResetFence ( VkFence Fence )
    {
    if (Fence == VK_NULL_HANDLE)
        {
        LogError ( "Cannot reset null fence" );
        return;
        }

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkResetFences ( device, 1, &Fence );
    }

void CSyncManager::ResetFences ( const std::vector<VkFence> & Fences )
    {
    if (Fences.empty ())
        {
        LogError ( "Cannot reset empty fences list" );
        return;
        }

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkResetFences ( device, static_cast< uint32_t >( Fences.size () ), Fences.data () );
    }

void CSyncManager::DestroySemaphore ( VkSemaphore Semaphore )
    {
    if (Semaphore == VK_NULL_HANDLE) return;

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkDestroySemaphore ( device, Semaphore, nullptr );
    }

void CSyncManager::DestroyFence ( VkFence Fence )
    {
    if (Fence == VK_NULL_HANDLE) return;

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkDestroyFence ( device, Fence, nullptr );
    }