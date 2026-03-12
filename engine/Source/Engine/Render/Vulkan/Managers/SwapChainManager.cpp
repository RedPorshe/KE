#include "Render/Vulkan/Managers/SwapChainManager.h"
#include "Core/EngineInfo.h"
#include "Render/Vulkan/Managers/DeviceManager.h"
#include <algorithm>

CSwapChainManager::CSwapChainManager ( FEngineInfo & inInfo )
    : IVulkanManager ( inInfo )
    {}

CSwapChainManager::~CSwapChainManager ()
    {
    Shutdown ();
    LogDebug ( GetManagerName (), " destroyed" );
    }

bool CSwapChainManager::Initialize ()
    {
    LogDebug ( "Initializing SwapChainManager..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr || !deviceMgr->IsInitialized ())
        {
        LogError ( "DeviceManager not initialized" );
        return false;
        }

    try
        {
        CreateSwapChain ();
        CreateImageViews ();
        }
        catch (const std::exception & e)
            {
            LogError ( "Failed to create swapchain: ", e.what () );
            return false;
            }

        LogDebug ( "SwapChainManager initialized successfully" );
        m_bInitialized = true;
        return true;
    }

void CSwapChainManager::Shutdown ()
    {
    if (!m_bInitialized) return;

    LogDebug ( "Shutting down SwapChainManager..." );

    CleanupSwapChain ();

    m_bInitialized = false;
    LogDebug ( "SwapChainManager shutdown complete" );
    }

const char * CSwapChainManager::GetManagerName () const
    {
    return "SwapChainManager";
    }

VkExtent2D CSwapChainManager::GetExtent ()
    {
    return m_Extent;
    }

void CSwapChainManager::CreateSwapChain ( VkSwapchainKHR OldSwapchain )
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkPhysicalDevice physicalDevice = deviceMgr->GetPhysicalDevice ();
    VkDevice device = deviceMgr->GetDevice ();
    VkSurfaceKHR surface = m_Info.Vulkan.Surface;

    // Get surface capabilities
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( physicalDevice, surface, &m_Capabilities );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to get surface capabilities: ", static_cast< int >( result ) );
        throw std::runtime_error ( "Failed to get surface capabilities" );
        }

    // Get surface formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR ( physicalDevice, surface, &formatCount, nullptr );

    std::vector<VkSurfaceFormatKHR> formats ( formatCount );
    vkGetPhysicalDeviceSurfaceFormatsKHR ( physicalDevice, surface, &formatCount, formats.data () );

    // Get present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR ( physicalDevice, surface, &presentModeCount, nullptr );

    std::vector<VkPresentModeKHR> presentModes ( presentModeCount );
    vkGetPhysicalDeviceSurfacePresentModesKHR ( physicalDevice, surface, &presentModeCount, presentModes.data () );

    // Choose settings
    VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat ( formats );
    VkPresentModeKHR presentMode = ChoosePresentMode ( presentModes );
    m_Extent = ChooseExtent ( m_Capabilities );

    m_ImageFormat = surfaceFormat.format;

    // Determine image count
    uint32_t imageCount = m_Capabilities.minImageCount + 1;
    if (m_Capabilities.maxImageCount > 0 && imageCount > m_Capabilities.maxImageCount)
        {
        imageCount = m_Capabilities.maxImageCount;
        }

    LogDebug ( "Creating swapchain with ", imageCount, " images, extent: ",
               m_Extent.width, "x", m_Extent.height );

      // Create swapchain
    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = m_Extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Queue families
    uint32_t graphicsFamily = deviceMgr->GetGraphicsQueueFamily ();
    uint32_t presentFamily = deviceMgr->GetPresentQueueFamily ();
    uint32_t queueFamilyIndices [] = { graphicsFamily, presentFamily };

    if (graphicsFamily != presentFamily)
        {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
    else
        {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        }

    createInfo.preTransform = m_Info.VulkanCreateInfo.PreTransform;
    createInfo.compositeAlpha = m_Info.VulkanCreateInfo.CompositeAlpha;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = OldSwapchain;  // Use the passed old swapchain

    result = vkCreateSwapchainKHR ( device, &createInfo, nullptr, &m_SwapChain );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create swapchain: ", static_cast< int >( result ) );
        throw std::runtime_error ( "Failed to create swapchain" );
        }

    // Get images
    vkGetSwapchainImagesKHR ( device, m_SwapChain, &imageCount, nullptr );
    m_Images.resize ( imageCount );
    vkGetSwapchainImagesKHR ( device, m_SwapChain, &imageCount, m_Images.data () );

    LogDebug ( "Swapchain created successfully with ", imageCount, " images" );
    }

void CSwapChainManager::CreateImageViews ()
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    m_ImageViews.resize ( m_Images.size () );

    for (size_t i = 0; i < m_Images.size (); i++)
        {
        VkImageViewCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_Images[ i ];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_ImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView ( device, &createInfo, nullptr, &m_ImageViews[ i ] );
        if (result != VK_SUCCESS)
            {
            LogError ( "Failed to create image view for image ", i, ": ", static_cast< int >( result ) );
            throw std::runtime_error ( "Failed to create image view" );
            }
        }

    LogDebug ( "Created ", m_ImageViews.size (), " image views" );
    }

void CSwapChainManager::CleanupSwapChain ()
    {
    LogDebug ( "Cleaning up swapchain resources..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr) return;

    VkDevice device = deviceMgr->GetDevice ();
    if (device == VK_NULL_HANDLE) return;

    // Cleanup image views
    for (auto imageView : m_ImageViews)
        {
        if (imageView != VK_NULL_HANDLE)
            {
            vkDestroyImageView ( device, imageView, nullptr );
            }
        }
    m_ImageViews.clear ();

    // Cleanup swapchain
    if (m_SwapChain != VK_NULL_HANDLE)
        {
        vkDestroySwapchainKHR ( device, m_SwapChain, nullptr );
        m_SwapChain = VK_NULL_HANDLE;
        }

    // Clear images (they are owned by swapchain, so no need to destroy)
    m_Images.clear ();

    // Reset format and extent
    m_ImageFormat = VK_FORMAT_UNDEFINED;
    m_Extent = { 0, 0 };

    // Reset capabilities
    m_Capabilities = {};

    LogDebug ( "Swapchain resources cleaned up" );
    }

bool CSwapChainManager::AcquireNextImage ( VkSemaphore SignalSemaphore, uint32_t & ImageIndex )
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkResult result = vkAcquireNextImageKHR (
        device,
        m_SwapChain,
        UINT64_MAX,
        SignalSemaphore,
        VK_NULL_HANDLE,
        &ImageIndex );

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
        LogDebug ( "Swapchain out of date, recreating..." );
        RecreateSwapChain ();
        return false;
        }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
        LogError ( "Failed to acquire next image: ", static_cast< int >( result ) );
        return false;
        }

    return true;
    }

bool CSwapChainManager::Present ( VkSemaphore WaitSemaphore, uint32_t ImageIndex )
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr)
        {
        LogError ( "Present: deviceMgr is null" );
        return false;
        }

    VkQueue presentQueue = deviceMgr->GetPresentQueue ();
    if (presentQueue == VK_NULL_HANDLE)
        {
        LogError ( "Present: presentQueue is null" );
        return false;
        }

    if (m_SwapChain == VK_NULL_HANDLE)
        {
        LogError ( "Present: swapchain is null" );
        return false;
        }

    VkPresentInfoKHR presentInfo {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &WaitSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_SwapChain;
    presentInfo.pImageIndices = &ImageIndex;

    VkResult result = vkQueuePresentKHR ( presentQueue, &presentInfo );

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
        LogDebug ( "Swapchain out of date or suboptimal, recreating..." );
        RecreateSwapChain ();
        return false;
        }
    else if (result != VK_SUCCESS)
        {
        LogError ( "Failed to present image: ", static_cast< int >( result ) );
        return false;
        }

    return true;
    }

bool CSwapChainManager::RecreateSwapChain ()
    {
    LogDebug ( "Recreating swapchain..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr) return false;

    VkDevice device = deviceMgr->GetDevice ();

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( deviceMgr->GetPhysicalDevice (), m_Info.Vulkan.Surface, &capabilities );

    // Ждём, пока окно не будет восстановлено
    while (capabilities.currentExtent.width == 0 || capabilities.currentExtent.height == 0)
        {
            // Проверяем, не закрыто ли окно
        if (glfwWindowShouldClose ( m_Info.WindowHandle ))
            {
            return false;
            }

            // Проверяем состояние окна (иконфицировано/свёрнуто)
        if (glfwGetWindowAttrib ( m_Info.WindowHandle, GLFW_ICONIFIED ))
            {
                // Окно свёрнуто - ждём
            std::this_thread::sleep_for ( std::chrono::milliseconds ( 16 ) );
            }
        else
            {
                // Окно не свёрнуто, но размер 0 - возможно, ошибка
            LogDebug ( "  Window not iconified but size is 0, waiting..." );
            std::this_thread::sleep_for ( std::chrono::milliseconds ( 16 ) );
            }
        glfwPollEvents ();
            // Обновляем capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( deviceMgr->GetPhysicalDevice (), m_Info.Vulkan.Surface, &capabilities );
        }

        // Wait for device idle
    vkDeviceWaitIdle ( device );

    // Store old swapchain
    VkSwapchainKHR oldSwapchain = m_SwapChain;

    // Clean up views only (not the swapchain itself)
    for (auto imageView : m_ImageViews)
        {
        vkDestroyImageView ( device, imageView, nullptr );
        }
    m_ImageViews.clear ();
    m_Images.clear ();

    // Reset swapchain handle
    m_SwapChain = VK_NULL_HANDLE;

    // Recreate with old swapchain
    try
        {
        CreateSwapChain ( oldSwapchain );
        CreateImageViews ();
        }
        catch (const std::exception & e)
            {
            LogError ( "Failed to recreate swapchain: ", e.what () );

            // Restore old swapchain on failure
            if (m_SwapChain == VK_NULL_HANDLE)
                {
                m_SwapChain = oldSwapchain;
                }
            return false;
            }

        // Destroy old swapchain
        if (oldSwapchain != VK_NULL_HANDLE)
            {
            vkDestroySwapchainKHR ( device, oldSwapchain, nullptr );
            }

        LogDebug ( "Swapchain recreated successfully" );
        SetSwapChainRecreated ( true );
        return true;
    }

VkSurfaceFormatKHR CSwapChainManager::ChooseSurfaceFormat (
    const std::vector<VkSurfaceFormatKHR> & AvailableFormats ) const
    {
    // If only one format and it's undefined, use our preferred format
    if (AvailableFormats.size () == 1 && AvailableFormats[ 0 ].format == VK_FORMAT_UNDEFINED)
        {
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
        }

    // Try to find our preferred format
    for (const auto & format : AvailableFormats)
        {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
             format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
            return format;
            }
        }

    // Fall back to first available
    return AvailableFormats[ 0 ];
    }

VkPresentModeKHR CSwapChainManager::ChoosePresentMode (
    const std::vector<VkPresentModeKHR> & AvailableModes ) const
    {
    // Try to use mailbox (triple buffering) for best performance
    for (const auto & mode : AvailableModes)
        {
        if (mode == m_Info.VulkanCreateInfo.PreferredPresentMode)
            {
            return mode;
            }
        }

    // Fall back to FIFO (always available)
    return VK_PRESENT_MODE_FIFO_KHR;
    }

VkExtent2D CSwapChainManager::ChooseExtent ( const VkSurfaceCapabilitiesKHR & Capabilities ) const
    {
    // If current extent is already set, use it
    if (Capabilities.currentExtent.width != UINT32_MAX)
        {
        return Capabilities.currentExtent;
        }

    // Otherwise, choose extent within limits
    VkExtent2D extent = {
        static_cast< uint32_t >( m_Info.WindowInfo.Width ),
        static_cast< uint32_t >( m_Info.WindowInfo.Height )
        };

    extent.width = std::max ( Capabilities.minImageExtent.width,
                              std::min ( Capabilities.maxImageExtent.width, extent.width ) );
    extent.height = std::max ( Capabilities.minImageExtent.height,
                               std::min ( Capabilities.maxImageExtent.height, extent.height ) );

    return extent;
    }