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

	LogDebug ( "CommandManager initialized successfully" );
	bIsInitialized = true;
	return true;
	}

void CommandManager::Shutdown ()
	{
	LogDebug ( "Shutting down CommandManager..." );

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
	auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
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

std::vector<VkCommandBuffer> CommandManager::CreateCommandBuffers ( uint32_t Count, VkCommandBufferLevel Level )
	{
	std::vector<VkCommandBuffer> commandBuffers;

	if (Count == 0)
		{
		LogError ( "Cannot create 0 command buffers" );
		return commandBuffers;
		}

	auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
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

void CommandManager::FreeCommandBuffer ( VkCommandBuffer CommandBuffer )
	{
	if (CommandBuffer == VK_NULL_HANDLE) return;

	auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
	VkDevice device = deviceMgr->GetDevice ();

	vkFreeCommandBuffers ( device, m_CommandPool, 1, &CommandBuffer );
	}

void CommandManager::FreeCommandBuffers ( const std::vector<VkCommandBuffer> & CommandBuffers )
	{
	if (CommandBuffers.empty ()) return;

	for (auto & bufer : CommandBuffers)
		{
		FreeCommandBuffer ( bufer );
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

void CommandManager::ResetCommandPool ()
	{
	auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
	VkDevice device = deviceMgr->GetDevice ();

	VkResult result = vkResetCommandPool ( device, m_CommandPool, 0 );
	if (result != VK_SUCCESS)
		{
		LogError ( "Failed to reset command pool: ", static_cast< int >( result ) );
		}
	}
