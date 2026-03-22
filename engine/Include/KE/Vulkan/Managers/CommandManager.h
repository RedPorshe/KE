#pragma once
#include "KE/Vulkan/VKManager.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <vector>

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

        // Frame command buffers (for swapchain)
        void SetFrameCommandBuffers ( uint32_t FrameCount );
        VkCommandBuffer GetFrameCommandBuffer ( uint32_t FrameIndex ) const;
        void ResetFrameCommandBuffers ();
        uint32_t GetFrameCommandBufferCount () const { return static_cast< uint32_t >( m_frameCommandBuffers.size () ); }

        // Command recording helpers
        void BeginCommandBuffer ( VkCommandBuffer CmdBuffer, VkCommandBufferUsageFlags Flags = 0 );
        void EndCommandBuffer ( VkCommandBuffer CmdBuffer );

        // Convenience method for recording commands
        void RecordCommandBuffer ( VkCommandBuffer CmdBuffer, std::function<void ( VkCommandBuffer )> && RecordingFunc );

        // One-time submit commands
        VkCommandBuffer BeginSingleTimeCommands ();
        void EndSingleTimeCommands ( VkCommandBuffer CmdBuffer );

        // Submit command buffer to queue
        void SubmitCommandBuffer ( VkCommandBuffer CmdBuffer, VkQueue Queue = VK_NULL_HANDLE );

        // Reset
        void ResetCommandPool ();

        // Wait idle
        void WaitIdle ();

    private:
        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> m_frameCommandBuffers;
    };