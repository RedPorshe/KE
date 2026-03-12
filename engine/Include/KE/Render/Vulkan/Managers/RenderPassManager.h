#pragma once
#include "Render/Vulkan/VulkanInterface.h"
#include <vector>

struct FEngineInfo;

struct KE_API FRenderPassInfo
    {
    VkFormat ColorFormat = VK_FORMAT_UNDEFINED;
    VkFormat DepthFormat = VK_FORMAT_UNDEFINED;
    VkSampleCountFlagBits Samples = VK_SAMPLE_COUNT_1_BIT;
    VkAttachmentLoadOp LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkImageLayout InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout FinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    };

class KE_API CRenderPassManager final : public IVulkanManager
    {
    public:
        CRenderPassManager ( FEngineInfo & Info );
        virtual ~CRenderPassManager ();

        // IVulkanManager
        bool Initialize () override;
        void Shutdown () override;
        const char * GetManagerName () const override;

        // Main render pass
        VkRenderPass GetMainRenderPass () const { return m_MainRenderPass; }
        const std::vector<VkFramebuffer> & GetFramebuffers () const { return m_Framebuffers; }
        VkFramebuffer GetFramebuffer ( uint32_t Index ) const;

        // Render pass creation
        VkRenderPass CreateRenderPass ( const FRenderPassInfo & Info );

        // Framebuffer creation
        VkFramebuffer CreateFramebuffer ( VkRenderPass RenderPass,
                                          const std::vector<VkImageView> & Attachments,
                                          uint32_t Width, uint32_t Height );

          // Swap chain dependent resources
        bool CreateSwapChainResources ();
        void DestroySwapChainResources ();

        // Depth resources
        VkImageView GetDepthImageView () const { return m_DepthImageView; }
        VkImage GetDepthImage () const { return m_DepthImage; }
        bool HasDepthAttachment () const { return m_DepthImageView != VK_NULL_HANDLE; }
        bool RecreateForSwapChain ();
    private:
        void CreateMainRenderPass ();
        void CreateFramebuffers ();
        void CreateDepthResources ();
        void DestroyDepthResources ();

        bool CreateDepthImage ();
        bool CreateDepthImageView ();

    private:
        // Main render pass
        VkRenderPass m_MainRenderPass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> m_Framebuffers;

        // Depth resources
        VkImage m_DepthImage = VK_NULL_HANDLE;
        VkDeviceMemory m_DepthImageMemory = VK_NULL_HANDLE;
        VkImageView m_DepthImageView = VK_NULL_HANDLE;

        // Format info
        VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;

        // Cached for recreation
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
    };