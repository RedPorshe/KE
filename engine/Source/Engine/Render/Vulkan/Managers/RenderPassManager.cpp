#include "Render/Vulkan/Managers/RenderPassManager.h"
#include "Core/EngineInfo.h"
#include "Render/Vulkan/Managers/DeviceManager.h"
#include "Render/Vulkan/Managers/SwapChainManager.h"

CRenderPassManager::CRenderPassManager ( FEngineInfo & Info )
    : IVulkanManager ( Info )
    {}

CRenderPassManager::~CRenderPassManager ()
    {
    Shutdown ();
    LogDebug ( GetManagerName (), " destroyed" );
    }

bool CRenderPassManager::Initialize ()
    {
    LogDebug ( "Initializing RenderPassManager..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr || !deviceMgr->IsInitialized ())
        {
        LogError ( "DeviceManager not initialized" );
        return false;
        }

    auto * swapChainMgr = static_cast< CSwapChainManager * >( m_Info.Vulkan.SwapChainManager.get () );
    if (!swapChainMgr || !swapChainMgr->IsInitialized ())
        {
        LogError ( "SwapChainManager not initialized" );
        return false;
        }

    m_DepthFormat = m_Info.VulkanCreateInfo.DepthFormat;

    try
        {
        CreateMainRenderPass ();
        CreateDepthResources ();
        CreateFramebuffers ();
        }
        catch (const std::exception & e)
            {
            LogError ( "Failed to create render pass resources: ", e.what () );
            return false;
            }

        LogDebug ( "RenderPassManager initialized successfully" );
        m_bInitialized = true;
        return true;
    }

void CRenderPassManager::Shutdown ()
    {
    if (!m_bInitialized) return;

    LogDebug ( "Shutting down RenderPassManager..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (deviceMgr)
        {
        VkDevice device = deviceMgr->GetDevice ();

        // Destroy framebuffers
        for (auto framebuffer : m_Framebuffers)
            {
            if (framebuffer != VK_NULL_HANDLE)
                {
                vkDestroyFramebuffer ( device, framebuffer, nullptr );
                }
            }
        m_Framebuffers.clear ();

        // Destroy depth resources
        DestroyDepthResources ();

        // Destroy main render pass
        if (m_MainRenderPass != VK_NULL_HANDLE)
            {
            vkDestroyRenderPass ( device, m_MainRenderPass, nullptr );
            m_MainRenderPass = VK_NULL_HANDLE;
            }
        }

    m_Width = 0;
    m_Height = 0;
    m_bInitialized = false;
    LogDebug ( "RenderPassManager shutdown complete" );
    }

const char * CRenderPassManager::GetManagerName () const
    {
    return "RenderPassManager";
    }

VkFramebuffer CRenderPassManager::GetFramebuffer ( uint32_t Index ) const
    {
    if (Index >= m_Framebuffers.size ())
        {
        LogError ( "Framebuffer index ", Index, " out of range (max: ", m_Framebuffers.size () - 1, ")" );
        return VK_NULL_HANDLE;
        }

    VkFramebuffer fb = m_Framebuffers[ Index ];
    if (fb == VK_NULL_HANDLE)
        {
        LogError ( "Framebuffer at index ", Index, " is null" );
        }

    return fb;
    }

bool CRenderPassManager::RecreateForSwapChain ()
    {
    LogDebug ( "Recreating render pass for new swapchain..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr) return false;

    VkDevice device = deviceMgr->GetDevice ();

    // Ждём пока GPU закончит работу
    vkDeviceWaitIdle ( device );

    // Уничтожаем старые ресурсы
    DestroySwapChainResources ();

    // Уничтожаем старый render pass
    if (m_MainRenderPass != VK_NULL_HANDLE)
        {
        vkDestroyRenderPass ( device, m_MainRenderPass, nullptr );
        m_MainRenderPass = VK_NULL_HANDLE;
        }

    try
        {
            // Создаём новый render pass с актуальным форматом из swapchain
        CreateMainRenderPass ();

        // Создаём новые depth resources
        CreateDepthResources ();

        // Создаём новые framebuffers
        CreateFramebuffers ();
        }
        catch (const std::exception & e)
            {
            LogError ( "Failed to recreate render pass: ", e.what () );
            return false;
            }

        LogDebug ( "Render pass recreated successfully" );
        return true;
    }

void CRenderPassManager::CreateMainRenderPass ()
    {
    LogDebug ( "Creating MAIN render pass..." ); // Добавьте эту строку

    auto * swapChainMgr = static_cast< CSwapChainManager * >( m_Info.Vulkan.SwapChainManager.get () );
    if (!swapChainMgr)
        {
        LogError ( "CreateMainRenderPass: SwapChainManager is null" );
        throw std::runtime_error ( "SwapChainManager is null" );
        }

    VkFormat colorFormat = swapChainMgr->GetImageFormat ();
    VkExtent2D extent = swapChainMgr->GetExtent (); // Получаем текущий размер!

    LogDebug ( "  Creating render pass for size: ", extent.width, "x", extent.height,
               ", format: ", static_cast< int >( colorFormat ) );

    // Color attachment
    VkAttachmentDescription colorAttachment {};
    colorAttachment.format = colorFormat;
    colorAttachment.samples = m_Info.VulkanCreateInfo.MsaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth attachment
    VkAttachmentDescription depthAttachment {};
    depthAttachment.format = m_DepthFormat;
    depthAttachment.samples = m_Info.VulkanCreateInfo.MsaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Subpass
    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // Subpass dependencies
    VkSubpassDependency dependency {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

// Attachments array
    std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast< uint32_t >( attachments.size () );
    renderPassInfo.pAttachments = attachments.data ();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkResult result = vkCreateRenderPass ( device, &renderPassInfo, nullptr, &m_MainRenderPass );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create render pass: ", static_cast< int >( result ) );
        throw std::runtime_error ( "Failed to create render pass" );
        }

    LogDebug ( "Main render pass created successfully" );
    }

void CRenderPassManager::CreateDepthResources ()
    {
    if (!CreateDepthImage ())
        {
        throw std::runtime_error ( "Failed to create depth image" );
        }

    if (!CreateDepthImageView ())
        {
        throw std::runtime_error ( "Failed to create depth image view" );
        }
    }

bool CRenderPassManager::CreateDepthImage ()
    {
    auto * swapChainMgr = static_cast< CSwapChainManager * >( m_Info.Vulkan.SwapChainManager.get () );
    m_Width = swapChainMgr->GetExtent ().width;
    m_Height = swapChainMgr->GetExtent ().height;

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkImageCreateInfo imageInfo {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_Width;
    imageInfo.extent.height = m_Height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = m_DepthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = m_Info.VulkanCreateInfo.MsaaSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateImage ( device, &imageInfo, nullptr, &m_DepthImage );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create depth image: ", static_cast< int >( result ) );
        return false;
        }

    // Allocate memory for depth image
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements ( device, m_DepthImage, &memRequirements );

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = deviceMgr->FindMemoryType (
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    result = vkAllocateMemory ( device, &allocInfo, nullptr, &m_DepthImageMemory );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to allocate depth image memory: ", static_cast< int >( result ) );
        vkDestroyImage ( device, m_DepthImage, nullptr );
        m_DepthImage = VK_NULL_HANDLE;
        return false;
        }

    vkBindImageMemory ( device, m_DepthImage, m_DepthImageMemory, 0 );

    LogDebug ( "Depth image created: ", m_Width, "x", m_Height );
    return true;
    }

bool CRenderPassManager::CreateDepthImageView ()
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkImageViewCreateInfo viewInfo {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_DepthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_DepthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    // Check if format has stencil component
    if (m_DepthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         m_DepthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
        {
        viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

    VkResult result = vkCreateImageView ( device, &viewInfo, nullptr, &m_DepthImageView );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create depth image view: ", static_cast< int >( result ) );
        return false;
        }

    LogDebug ( "Depth image view created" );
    return true;
    }

void CRenderPassManager::DestroyDepthResources ()
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr) return;

    VkDevice device = deviceMgr->GetDevice ();

    if (m_DepthImageView != VK_NULL_HANDLE)
        {
        vkDestroyImageView ( device, m_DepthImageView, nullptr );
        m_DepthImageView = VK_NULL_HANDLE;
        }

    if (m_DepthImage != VK_NULL_HANDLE)
        {
        vkDestroyImage ( device, m_DepthImage, nullptr );
        m_DepthImage = VK_NULL_HANDLE;
        }

    if (m_DepthImageMemory != VK_NULL_HANDLE)
        {
        vkFreeMemory ( device, m_DepthImageMemory, nullptr );
        m_DepthImageMemory = VK_NULL_HANDLE;
        }
    }

void CRenderPassManager::CreateFramebuffers ()
    {
    auto * swapChainMgr = static_cast< CSwapChainManager * >( m_Info.Vulkan.SwapChainManager.get () );
    if (!swapChainMgr)
        {
        LogError ( "CreateFramebuffers: SwapChainManager is null" );
        throw std::runtime_error ( "SwapChainManager is null" );
        }

    const auto & imageViews = swapChainMgr->GetImageViews ();
    VkExtent2D extent = swapChainMgr->GetExtent ();

    if (imageViews.empty ())
        {
        LogError ( "CreateFramebuffers: No image views from swapchain" );
        throw std::runtime_error ( "No image views" );
        }

    LogDebug ( "  Creating framebuffers with extent: ", extent.width, "x", extent.height );
    LogDebug ( "  Number of image views: ", imageViews.size () );

    // Уничтожаем старые framebuffers если есть
    for (auto fb : m_Framebuffers)
        {
        if (fb != VK_NULL_HANDLE)
            {
            auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
            vkDestroyFramebuffer ( deviceMgr->GetDevice (), fb, nullptr );
            }
        }
    m_Framebuffers.clear ();

    // Создаем новые framebuffers
    m_Framebuffers.resize ( imageViews.size () );

    for (size_t i = 0; i < imageViews.size (); i++)
        {
        std::vector<VkImageView> attachments = {
            imageViews[ i ],
            m_DepthImageView
            };

        VkFramebufferCreateInfo framebufferInfo {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_MainRenderPass;
        framebufferInfo.attachmentCount = static_cast< uint32_t > ( attachments.size () );
        framebufferInfo.pAttachments = attachments.data ();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
        VkDevice device = deviceMgr->GetDevice ();

        VkResult result = vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &m_Framebuffers[ i ] );
        if (result != VK_SUCCESS)
            {
            LogError ( "Failed to create framebuffer for image ", i, ": ", static_cast< int >( result ) );
            throw std::runtime_error ( "Failed to create framebuffer" );
            }

        LogDebug ( "    Created framebuffer ", i, ": ", ( void * ) m_Framebuffers[ i ] );
        }

    LogDebug ( "  Created ", m_Framebuffers.size (), " framebuffers" );
    }

bool CRenderPassManager::CreateSwapChainResources ()
    {
    LogDebug ( "RenderPassManager recreating swapchain resources..." );
    LogDebug ( "  Current framebuffers count: ", m_Framebuffers.size () );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    // Получаем новый размер из swapchain
    auto * swapChainMgr = static_cast< CSwapChainManager * >( m_Info.Vulkan.SwapChainManager.get () );
    if (swapChainMgr)
        {
        VkExtent2D extent = swapChainMgr->GetExtent ();
        m_Width = extent.width;
        m_Height = extent.height;
        LogDebug ( "  New extent from swapchain: ", m_Width, "x", m_Height );
        }

    // ПРИНУДИТЕЛЬНО уничтожаем старые ресурсы
    DestroySwapChainResources ();

    // Уничтожаем старый render pass
    if (m_MainRenderPass != VK_NULL_HANDLE)
        {
        vkDestroyRenderPass ( device, m_MainRenderPass, nullptr );
        m_MainRenderPass = VK_NULL_HANDLE;
        }

    try
        {
        // Создаем НОВЫЙ render pass (на всякий случай)
        CreateMainRenderPass ();

        // Создаем новые depth resources с новым размером
        CreateDepthResources ();

        // Создаем новые framebuffers с новым размером
        CreateFramebuffers ();
        }
        catch (const std::exception & e)
            {
            LogError ( "Failed to recreate swapchain resources: ", e.what () );
            return false;
            }

        LogDebug ( "  New framebuffers count: ", m_Framebuffers.size () );
        if (m_Framebuffers.size () > 0)
            {
            LogDebug ( "  New framebuffer 0: ", ( void * ) m_Framebuffers[ 0 ] );
            }

        LogDebug ( "RenderPassManager swapchain resources recreated successfully" );
        return true;
    }

void CRenderPassManager::DestroySwapChainResources ()
    {
    LogDebug ( "Destroying swapchain resources..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr) return;

    VkDevice device = deviceMgr->GetDevice ();

    // Destroy framebuffers
    for (auto framebuffer : m_Framebuffers)
        {
        if (framebuffer != VK_NULL_HANDLE)
            {
            vkDestroyFramebuffer ( device, framebuffer, nullptr );
            }
        }
    m_Framebuffers.clear ();

    // Destroy depth resources
    DestroyDepthResources ();

    LogDebug ( "Swapchain resources destroyed" );
    }

VkRenderPass CRenderPassManager::CreateRenderPass ( const FRenderPassInfo & Info )
    {
    // Color attachment
    VkAttachmentDescription colorAttachment {};
    colorAttachment.format = Info.ColorFormat;
    colorAttachment.samples = Info.Samples;
    colorAttachment.loadOp = Info.LoadOp;
    colorAttachment.storeOp = Info.StoreOp;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = Info.InitialLayout;
    colorAttachment.finalLayout = Info.FinalLayout;

    VkAttachmentReference colorAttachmentRef {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth attachment (if valid format)
    VkAttachmentDescription depthAttachment {};
    VkAttachmentReference depthAttachmentRef {};
    bool hasDepth = Info.DepthFormat != VK_FORMAT_UNDEFINED;

    if (hasDepth)
        {
        depthAttachment.format = Info.DepthFormat;
        depthAttachment.samples = Info.Samples;
        depthAttachment.loadOp = Info.LoadOp;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

    // Subpass
    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    if (hasDepth)
        {
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        }

    // Subpass dependencies
    VkSubpassDependency dependency {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

// Attachments array
    std::vector<VkAttachmentDescription> attachments = { colorAttachment };
    if (hasDepth)
        {
        attachments.push_back ( depthAttachment );
        }

    VkRenderPassCreateInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast< uint32_t >( attachments.size () );
    renderPassInfo.pAttachments = attachments.data ();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkRenderPass renderPass;
    VkResult result = vkCreateRenderPass ( device, &renderPassInfo, nullptr, &renderPass );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create custom render pass: ", static_cast< int >( result ) );
        return VK_NULL_HANDLE;
        }

    return renderPass;
    }

VkFramebuffer CRenderPassManager::CreateFramebuffer ( VkRenderPass RenderPass,
                                                      const std::vector<VkImageView> & Attachments,
                                                      uint32_t Width, uint32_t Height )
    {
    VkFramebufferCreateInfo framebufferInfo {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = RenderPass;
    framebufferInfo.attachmentCount = static_cast< uint32_t >( Attachments.size () );
    framebufferInfo.pAttachments = Attachments.data ();
    framebufferInfo.width = Width;
    framebufferInfo.height = Height;
    framebufferInfo.layers = 1;

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkFramebuffer framebuffer;
    VkResult result = vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create custom framebuffer: ", static_cast< int >( result ) );
        return VK_NULL_HANDLE;
        }

    return framebuffer;
    }