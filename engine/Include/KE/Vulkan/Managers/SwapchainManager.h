#pragma once
#include "KE/Vulkan/VKManager.h"
#include <vulkan/vulkan.h>

class SwapchainManager final : public IVKManager
	{
	public:
		SwapchainManager ();
		virtual ~SwapchainManager () override;
				
		bool Init () override;
		void Shutdown () override;
		const std::string & GetManagerName () const override;

		const std::vector<VkImage> & GetImages () const { return m_images; }
		VkSwapchainKHR GetSwapchain () const { return m_swapchain; }
		VkFormat GetImageFormat () const { return m_imageFormat; }
		VkExtent2D GetExtent () const { return m_extent; }
		bool AcquireNextImage ( VkSemaphore SignalSemaphore, uint32_t & ImageIndex );
		bool Present ( VkSemaphore WaitSemaphore, uint32_t ImageIndex );
		bool RecreateSwapChain ();
		void CreateImageViews ();
		bool CreateSwapchain ( VkSwapchainKHR OldSwapchain = VK_NULL_HANDLE);
		bool GetImages ();
		uint32_t GetImagesCount ();
		void SetSwapChainRecreated ( bool val ) { bIsSwapchainRecreated = val; }
		void CleanupSwapChain ();
		TVector< VkImageView> GetImageViews () {return m_ImageViews	;}

		VkSurfaceFormatKHR ChooseSurfaceFormat ( const std::vector<VkSurfaceFormatKHR> & formats );
		VkPresentModeKHR ChoosePresentMode ( const std::vector<VkPresentModeKHR> & presentModes );
		VkExtent2D ChooseExtent ( const VkSurfaceCapabilitiesKHR & capabilities );

		VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
		VkFormat m_imageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D m_extent = { 0, 0 };
		std::vector<VkImage> m_images;
		TVector< VkImageView> m_ImageViews;
		VkSurfaceCapabilitiesKHR m_Capabilities = {};
		bool bIsSwapchainRecreated = false;

	};