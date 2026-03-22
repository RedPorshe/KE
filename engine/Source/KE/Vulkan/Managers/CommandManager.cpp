#include "KE/Vulkan/Managers/CommandManager.h"
#include "KE/Vulkan/Managers/DeviceManager.h"
#include "KE/Vulkan/VKinfo.h"

CommandManager::CommandManager ()
    {
    LogDebug ( "created" );
    }

CommandManager::~CommandManager ()
    {
    Shutdown ();
    }

bool CommandManager::Init ()
    {
    LogDebug ( "Initializing CommandManager..." );

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (!deviceMgr || !deviceMgr->IsInitialized ())
        {
        LogError ( "DeviceManager not initialized" );
        return false;
        }

    VkDevice device = deviceMgr->GetDevice ();
    if (device == VK_NULL_HANDLE)
        {
        LogError ( "Device is null" );
        return false;
        }

    uint32_t graphicsFamily = deviceMgr->GetGraphicsQueueFamily ();

    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicsFamily;

    VkResult result = vkCreateCommandPool ( device, &poolInfo, nullptr, &m_CommandPool );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create command pool: ", static_cast< int >( result ) );
        return false;
        }

    bIsInitialized = true;
    LogDebug ( "CommandManager initialized successfully" );
    return true;
    }

void CommandManager::Shutdown ()
    {
    LogDebug ( "Shutting down CommandManager..." );

    // Очищаем frame command buffers
    if (!m_frameCommandBuffers.empty ())
        {
        FreeCommandBuffers ( m_frameCommandBuffers );
        m_frameCommandBuffers.clear ();
        }

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (deviceMgr)
        {
        VkDevice device = deviceMgr->GetDevice ();

        if (m_CommandPool != VK_NULL_HANDLE)
            {
            vkDestroyCommandPool ( device, m_CommandPool, nullptr );
            m_CommandPool = VK_NULL_HANDLE;
            }
        }

    bIsInitialized = false;
    LogDebug ( "CommandManager shutdown complete" );
    }

const std::string & CommandManager::GetManagerName () const
    {
    static const std::string name = "Command Manager";
    return name;
    }

VkCommandBuffer CommandManager::CreateCommandBuffer ( VkCommandBufferLevel Level )
    {
    if (!bIsInitialized)
        {
        LogError ( "CommandManager not initialized" );
        return VK_NULL_HANDLE;
        }

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (!deviceMgr || !deviceMgr->IsInitialized ())
        {
        LogError ( "DeviceManager not available" );
        return VK_NULL_HANDLE;
        }

    VkDevice device = deviceMgr->GetDevice ();
    if (device == VK_NULL_HANDLE)
        {
        LogError ( "Device is null" );
        return VK_NULL_HANDLE;
        }

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

std::vector<VkCommandBuffer> CommandManager::CreateCommandBuffers ( uint32_t Count, VkCommandBufferLevel Level )
    {
    std::vector<VkCommandBuffer> commandBuffers;

    if (Count == 0)
        {
        LogError ( "Cannot create 0 command buffers" );
        return commandBuffers;
        }

    if (!bIsInitialized)
        {
        LogError ( "CommandManager not initialized" );
        return commandBuffers;
        }

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (!deviceMgr || !deviceMgr->IsInitialized ())
        {
        LogError ( "DeviceManager not available" );
        return commandBuffers;
        }

    VkDevice device = deviceMgr->GetDevice ();
    if (device == VK_NULL_HANDLE)
        {
        LogError ( "Device is null" );
        return commandBuffers;
        }

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

void CommandManager::FreeCommandBuffer ( VkCommandBuffer CommandBuffer )
    {
    if (CommandBuffer == VK_NULL_HANDLE) return;

    if (!bIsInitialized)
        {
        LogError ( "CommandManager not initialized" );
        return;
        }

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (!deviceMgr) return;

    VkDevice device = deviceMgr->GetDevice ();
    if (device == VK_NULL_HANDLE) return;

    vkFreeCommandBuffers ( device, m_CommandPool, 1, &CommandBuffer );
    }

void CommandManager::FreeCommandBuffers ( const std::vector<VkCommandBuffer> & CommandBuffers )
    {
    if (CommandBuffers.empty ()) return;

    if (!bIsInitialized)
        {
        LogError ( "CommandManager not initialized" );
        return;
        }

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (!deviceMgr) return;

    VkDevice device = deviceMgr->GetDevice ();
    if (device == VK_NULL_HANDLE) return;

    vkFreeCommandBuffers ( device, m_CommandPool,
                           static_cast< uint32_t >( CommandBuffers.size () ),
                           CommandBuffers.data () );
    }

void CommandManager::SetFrameCommandBuffers ( uint32_t FrameCount )
    {
        // Очищаем старые
    if (!m_frameCommandBuffers.empty ())
        {
        FreeCommandBuffers ( m_frameCommandBuffers );
        m_frameCommandBuffers.clear ();
        }

        // Создаем новые для каждого кадра swapchain
    m_frameCommandBuffers = CreateCommandBuffers ( FrameCount );
    LogDebug ( "Created ", FrameCount, " frame command buffers" );
    }

VkCommandBuffer CommandManager::GetFrameCommandBuffer ( uint32_t FrameIndex ) const
    {
    if (FrameIndex >= m_frameCommandBuffers.size ())
        {
        LogError ( "Frame index out of range: ", FrameIndex, " (max: ", m_frameCommandBuffers.size (), ")" );
        return VK_NULL_HANDLE;
        }
    return m_frameCommandBuffers[ FrameIndex ];
    }

void CommandManager::ResetFrameCommandBuffers ()
    {
    for (auto & cmdBuffer : m_frameCommandBuffers)
        {
        if (cmdBuffer != VK_NULL_HANDLE)
            {
            vkResetCommandBuffer ( cmdBuffer, 0 );
            }
        }
    }

void CommandManager::BeginCommandBuffer ( VkCommandBuffer CmdBuffer, VkCommandBufferUsageFlags Flags )
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

void CommandManager::EndCommandBuffer ( VkCommandBuffer CmdBuffer )
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

void CommandManager::RecordCommandBuffer ( VkCommandBuffer CmdBuffer, std::function<void ( VkCommandBuffer )> && RecordingFunc )
    {
    if (CmdBuffer == VK_NULL_HANDLE)
        {
        LogError ( "Cannot record null command buffer" );
        return;
        }

    BeginCommandBuffer ( CmdBuffer );
    RecordingFunc ( CmdBuffer );
    EndCommandBuffer ( CmdBuffer );
    }

VkCommandBuffer CommandManager::BeginSingleTimeCommands ()
    {
    VkCommandBuffer commandBuffer = CreateCommandBuffer ();

    if (commandBuffer != VK_NULL_HANDLE)
        {
        BeginCommandBuffer ( commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT );
        }

    return commandBuffer;
    }

void CommandManager::EndSingleTimeCommands ( VkCommandBuffer CmdBuffer )
    {
    if (CmdBuffer == VK_NULL_HANDLE) return;

    EndCommandBuffer ( CmdBuffer );

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (!deviceMgr) return;

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

void CommandManager::SubmitCommandBuffer ( VkCommandBuffer CmdBuffer, VkQueue Queue )
    {
    if (CmdBuffer == VK_NULL_HANDLE)
        {
        LogError ( "Cannot submit null command buffer" );
        return;
        }

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (!deviceMgr) return;

    VkQueue targetQueue = Queue;
    if (targetQueue == VK_NULL_HANDLE)
        {
        targetQueue = deviceMgr->GetGraphicsQueue ();
        }

    if (targetQueue == VK_NULL_HANDLE)
        {
        LogError ( "No valid queue for submission" );
        return;
        }

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &CmdBuffer;

    VkResult result = vkQueueSubmit ( targetQueue, 1, &submitInfo, VK_NULL_HANDLE );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to submit command buffer: ", static_cast< int >( result ) );
        }
    }

void CommandManager::ResetCommandPool ()
    {
    if (!bIsInitialized)
        {
        LogError ( "CommandManager not initialized" );
        return;
        }

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (!deviceMgr) return;

    VkDevice device = deviceMgr->GetDevice ();
    if (device == VK_NULL_HANDLE) return;

    VkResult result = vkResetCommandPool ( device, m_CommandPool, 0 );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to reset command pool: ", static_cast< int >( result ) );
        }
    }

void CommandManager::WaitIdle ()
    {
    if (!bIsInitialized) return;

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (!deviceMgr) return;

    VkDevice device = deviceMgr->GetDevice ();
    if (device == VK_NULL_HANDLE) return;

    vkDeviceWaitIdle ( device );
    }