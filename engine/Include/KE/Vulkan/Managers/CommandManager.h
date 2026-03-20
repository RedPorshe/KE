#pragma once
#include "KE/Vulkan/VKManager.h"
#include <vulkan/vulkan.h>

class CommandManager final : public IVKManager
	{
	public:
        CommandManager ();
        virtual ~CommandManager () override;

		// Inherited via IVKManager
		bool Init () override;
		void Shutdown () override;
		const std::string & GetManagerName () const override;
        // Command pool
        VkCommandPool GetCommandPool () const { return m_CommandPool; }

        // Command buffer creation
        VkCommandBuffer CreateCommandBuffer ( VkCommandBufferLevel Level = VK_COMMAND_BUFFER_LEVEL_PRIMARY );
        std::vector<VkCommandBuffer> CreateCommandBuffers ( uint32_t Count,
                                                            VkCommandBufferLevel Level = VK_COMMAND_BUFFER_LEVEL_PRIMARY );
        void FreeCommandBuffer ( VkCommandBuffer CommandBuffer );
        void FreeCommandBuffers ( const std::vector<VkCommandBuffer> & CommandBuffers );

        // Command recording helpers
        void BeginCommandBuffer ( VkCommandBuffer CmdBuffer, VkCommandBufferUsageFlags Flags = 0 );
        void EndCommandBuffer ( VkCommandBuffer CmdBuffer );

        // One-time submit commands
        VkCommandBuffer BeginSingleTimeCommands ();
        void EndSingleTimeCommands ( VkCommandBuffer CmdBuffer );

        // Reset
        void ResetCommandPool ();

    private:
        VkCommandPool m_CommandPool = VK_NULL_HANDLE;

	};