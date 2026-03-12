#pragma once
#include "Render/Vulkan/VulkanInterface.h"
#include <vector>

struct FEngineInfo;

class KE_API CSwapChainManager final : public IVulkanManager
    {
    public:
        CSwapChainManager ( FEngineInfo & inInfo );
        virtual ~CSwapChainManager ();

        // IVulkanManager
        bool Initialize () override;
        void Shutdown () override;
        const char * GetManagerName () const override;
        VkExtent2D GetExtent () override;

        // Getters
        VkSwapchainKHR GetSwapChain () const { return m_SwapChain; }
        VkFormat GetImageFormat () const { return m_ImageFormat; }
        const std::vector<VkImage> & GetImages () const { return m_Images; }
        const std::vector<VkImageView> & GetImageViews () const { return m_ImageViews; }
        uint32_t GetImageCount () const { return static_cast< uint32_t >( m_Images.size () ); }
        bool IsSwapChainValid () const { return m_SwapChain != VK_NULL_HANDLE && !m_Images.empty (); }

        // Operations
        bool AcquireNextImage ( VkSemaphore SignalSemaphore, uint32_t & ImageIndex );
        bool Present ( VkSemaphore WaitSemaphore, uint32_t ImageIndex );
        bool RecreateSwapChain ();
        bool IsSwapChainRecreated () const { return bIsSwapchainRecreated; }
        void SetSwapChainRecreated ( bool val ) { bIsSwapchainRecreated = val; }
    private:
        void CreateSwapChain ( VkSwapchainKHR OldSwapchain = VK_NULL_HANDLE );
        void CreateImageViews ();
        void CleanupSwapChain ();
       
       
        VkSurfaceFormatKHR ChooseSurfaceFormat ( const std::vector<VkSurfaceFormatKHR> & AvailableFormats ) const;
        VkPresentModeKHR ChoosePresentMode ( const std::vector<VkPresentModeKHR> & AvailableModes ) const;
        VkExtent2D ChooseExtent ( const VkSurfaceCapabilitiesKHR & Capabilities ) const;

    private:
        // Core swapchain objects
        VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
        VkFormat m_ImageFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D m_Extent = { 0, 0 };
        bool bIsSwapchainRecreated = false;
        // Images and views
        std::vector<VkImage> m_Images;
        std::vector<VkImageView> m_ImageViews;

        // Cached for recreation
        VkSurfaceCapabilitiesKHR m_Capabilities = {};
    };