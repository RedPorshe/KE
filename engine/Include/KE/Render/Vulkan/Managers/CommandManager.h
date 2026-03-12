#pragma once
#include "Render/Vulkan/VulkanInterface.h"
#include <vector>

struct FEngineInfo;

class KE_API  CCommandManager final : public IVulkanManager
    {
    public:
        CCommandManager ( FEngineInfo & inInfo );
        virtual ~CCommandManager ();

        // IVulkanManager
        bool Initialize () override;
        void Shutdown () override;
        const char * GetManagerName () const override;

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