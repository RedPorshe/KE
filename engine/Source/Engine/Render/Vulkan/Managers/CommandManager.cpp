#include "Render/Vulkan/Managers/CommandManager.h"
#include "Core/EngineInfo.h"
#include "Render/Vulkan/Managers/DeviceManager.h"

CCommandManager::CCommandManager ( FEngineInfo & inInfo )
    : IVulkanManager ( inInfo )
    {}

CCommandManager::~CCommandManager ()
    {
    Shutdown ();
    LogDebug ( GetManagerName (), " destroyed" );
    }

bool CCommandManager::Initialize ()
    {
    LogDebug ( "Initializing CommandManager..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr || !deviceMgr->IsInitialized ())
        {
        LogError ( "DeviceManager not initialized" );
        return false;
        }

    VkDevice device = deviceMgr->GetDevice ();
    uint32_t graphicsFamily = deviceMgr->GetGraphicsQueueFamily ();

    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = m_Info.VulkanCreateInfo.CommandPoolFlags;
    poolInfo.queueFamilyIndex = graphicsFamily;

    VkResult result = vkCreateCommandPool ( device, &poolInfo, nullptr, &m_CommandPool );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create command pool: ", static_cast< int >( result ) );
        return false;
        }

    LogDebug ( "CommandManager initialized successfully" );
    m_bInitialized = true;
    return true;
    }

void CCommandManager::Shutdown ()
    {
    if (!m_bInitialized) return;

    LogDebug ( "Shutting down CommandManager..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (deviceMgr)
        {
        VkDevice device = deviceMgr->GetDevice ();

        if (m_CommandPool != VK_NULL_HANDLE)
            {
            vkDestroyCommandPool ( device, m_CommandPool, nullptr );
            m_CommandPool = VK_NULL_HANDLE;
            }
        }

    m_bInitialized = false;
    LogDebug ( "CommandManager shutdown complete" );
    }

const char * CCommandManager::GetManagerName () const
    {
    return "CommandManager";
    }

VkCommandBuffer CCommandManager::CreateCommandBuffer ( VkCommandBufferLevel Level )
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = Level;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VkResult result = vkAllocateCommandBuffers ( device, &allocInfo, &commandBuffer );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to allocate command buffer: ", static_cast< int >( result ) );
        return VK_NULL_HANDLE;
        }

    return commandBuffer;
    }

std::vector<VkCommandBuffer> CCommandManager::CreateCommandBuffers ( uint32_t Count, VkCommandBufferLevel Level )
    {
    std::vector<VkCommandBuffer> commandBuffers;

    if (Count == 0)
        {
        LogError ( "Cannot create 0 command buffers" );
        return commandBuffers;
        }

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    commandBuffers.resize ( Count );

    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = Level;
    allocInfo.commandBufferCount = Count;

    VkResult result = vkAllocateCommandBuffers ( device, &allocInfo, commandBuffers.data () );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to allocate command buffers: ", static_cast< int >( result ) );
        commandBuffers.clear ();
        }

    return commandBuffers;
    }

void CCommandManager::FreeCommandBuffer ( VkCommandBuffer CommandBuffer )
    {
    if (CommandBuffer == VK_NULL_HANDLE) return;

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkFreeCommandBuffers ( device, m_CommandPool, 1, &CommandBuffer );
    }

void CCommandManager::FreeCommandBuffers ( const std::vector<VkCommandBuffer> & CommandBuffers )
    {
    if (CommandBuffers.empty ()) return;

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkFreeCommandBuffers ( device, m_CommandPool,
                           static_cast< uint32_t >( CommandBuffers.size () ),
                           CommandBuffers.data () );
    }

void CCommandManager::BeginCommandBuffer ( VkCommandBuffer CmdBuffer, VkCommandBufferUsageFlags Flags )
    {
    if (CmdBuffer == VK_NULL_HANDLE)
        {
        LogError ( "Cannot begin null command buffer" );
        return;
        }

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = Flags;
    beginInfo.pInheritanceInfo = nullptr;

    VkResult result = vkBeginCommandBuffer ( CmdBuffer, &beginInfo );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to begin command buffer: ", static_cast< int >( result ) );
        }
    }

void CCommandManager::EndCommandBuffer ( VkCommandBuffer CmdBuffer )
    {
    if (CmdBuffer == VK_NULL_HANDLE)
        {
        LogError ( "Cannot end null command buffer" );
        return;
        }

    VkResult result = vkEndCommandBuffer ( CmdBuffer );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to end command buffer: ", static_cast< int >( result ) );
        }
    }

VkCommandBuffer CCommandManager::BeginSingleTimeCommands ()
    {
    VkCommandBuffer commandBuffer = CreateCommandBuffer ();

    if (commandBuffer != VK_NULL_HANDLE)
        {
        BeginCommandBuffer ( commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT );
        }

    return commandBuffer;
    }

void CCommandManager::EndSingleTimeCommands ( VkCommandBuffer CmdBuffer )
    {
    if (CmdBuffer == VK_NULL_HANDLE) return;

    EndCommandBuffer ( CmdBuffer );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();
    VkQueue graphicsQueue = deviceMgr->GetGraphicsQueue ();

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &CmdBuffer;

    VkResult result = vkQueueSubmit ( graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to submit single-time command buffer: ", static_cast< int >( result ) );
        }

    vkQueueWaitIdle ( graphicsQueue );

    FreeCommandBuffer ( CmdBuffer );
    }

void CCommandManager::ResetCommandPool ()
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkResult result = vkResetCommandPool ( device, m_CommandPool, 0 );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to reset command pool: ", static_cast< int >( result ) );
        }
    }