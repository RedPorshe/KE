#include "KE/Vulkan/Managers/SwapchainManager.h"
#include "KE/Vulkan/VKinfo.h"
#include "KE/Vulkan/Managers/DeviceManager.h"
#include "KE/Vulkan/Managers/InstanceManager.h"
#include <GLFW/glfw3.h>

SwapchainManager::SwapchainManager () :IVKManager ()
	{
	LogDebug ( "Created" );
	}

SwapchainManager::~SwapchainManager ()
	{
	Shutdown ();
	}

bool SwapchainManager::Init ()
	{
	LogDebug ( "Initializing Swapchain Manager" );

	if (!m_info || !m_info->Managers.DeviceManager)
		{
		LogError ( "Device Manager not available" );
		return false;
		}

	if (!CreateSwapchain ())
		{
		LogError ( "Failed to create swapchain" );
		return false;
		}

	if (!GetImages ())
		{
		LogError ( "Failed to get swapchain images" );
		return false;
		}
	bIsInitialized = true;
	LogInfo ( "Swapchain created successfully - Images: ", m_images.size () );
	return true;
	}

void SwapchainManager::CreateImageViews ()
	{
	auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
	VkDevice device = deviceMgr->GetDevice ();

	m_ImageViews.resize ( m_images.size () );

	for (size_t i = 0; i < m_images.size (); i++)
		{
		VkImageViewCreateInfo createInfo {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_images[ i ];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_imageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView ( device, &createInfo, nullptr, &m_ImageViews[ i ] );
		if (result != VK_SUCCESS)
			{
			LogError ( "Failed to create image view for image ", i, ": ", static_cast< int >( result ) );
			throw std::runtime_error ( "Failed to create image view" );
			}
		}

	LogDebug ( "Created ", m_ImageViews.size (), " image views" );
	}

void SwapchainManager::Shutdown ()
	{
	LogDebug ( "Shutting down SwapChainManager..." );

	CleanupSwapChain ();

	bIsInitialized = false;
	LogDebug ( "SwapChainManager shutdown complete" );
	}

const std::string & SwapchainManager::GetManagerName () const
	{
	static const std::string name = "Swapchain Manager";
	return name;
	}

bool SwapchainManager::CreateSwapchain ( VkSwapchainKHR OldSwapchain )
	{
	auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
	VkPhysicalDevice physicalDevice = deviceMgr->GetPhysicalDevice ();
	VkDevice device = deviceMgr->GetDevice ();

	VkSurfaceKHR surface = VK_NULL_HANDLE;
	if (InstanceManager * inst = dynamic_cast< InstanceManager * >( m_info->Managers.InstanceManager.get () ))
		{
		surface = inst->GetSurface ();
		}

	// Get surface capabilities
	VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( physicalDevice, surface, &m_Capabilities );
	if (result != VK_SUCCESS)
		{
		LogError ( "Failed to get surface capabilities: ", static_cast< int >( result ) );
		throw std::runtime_error ( "Failed to get surface capabilities" );
		}

	// Get surface formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR ( physicalDevice, surface, &formatCount, nullptr );

	std::vector<VkSurfaceFormatKHR> formats ( formatCount );
	vkGetPhysicalDeviceSurfaceFormatsKHR ( physicalDevice, surface, &formatCount, formats.data () );

	// Get present modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR ( physicalDevice, surface, &presentModeCount, nullptr );

	std::vector<VkPresentModeKHR> presentModes ( presentModeCount );
	vkGetPhysicalDeviceSurfacePresentModesKHR ( physicalDevice, surface, &presentModeCount, presentModes.data () );

	// Choose settings
	VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat ( formats );
	VkPresentModeKHR presentMode = ChoosePresentMode ( presentModes );
	m_extent = ChooseExtent ( m_Capabilities );

	m_imageFormat = surfaceFormat.format;

	// Determine image count
	uint32_t imageCount = m_Capabilities.minImageCount + 1;
	if (m_Capabilities.maxImageCount > 0 && imageCount > m_Capabilities.maxImageCount)
		{
		imageCount = m_Capabilities.maxImageCount;
		}

	LogDebug ( "Creating swapchain with ", imageCount, " images, extent: ",
			   m_extent.width, "x", m_extent.height );

	  // Create swapchain
	VkSwapchainCreateInfoKHR createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = m_extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Queue families
	uint32_t graphicsFamily = deviceMgr->GetGraphicsQueueFamily ();
	uint32_t presentFamily = deviceMgr->GetPresentQueueFamily ();
	uint32_t queueFamilyIndices [] = { graphicsFamily, presentFamily };

	if (graphicsFamily != presentFamily)
		{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
	else
		{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		}

	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = OldSwapchain;  // Use the passed old swapchain

	result = vkCreateSwapchainKHR ( device, &createInfo, nullptr, &m_swapchain );
	if (result != VK_SUCCESS)
		{
		LogError ( "Failed to create swapchain: ", static_cast< int >( result ) );
		throw std::runtime_error ( "Failed to create swapchain" );
		}

	// Get images
	vkGetSwapchainImagesKHR ( device, m_swapchain, &imageCount, nullptr );
	m_images.resize ( imageCount );
	vkGetSwapchainImagesKHR ( device, m_swapchain, &imageCount, m_images.data () );

	LogDebug ( "Swapchain created successfully with ", imageCount, " images" );
	return true;
	}

bool SwapchainManager::GetImages ()
	{
	VkDevice  device = VK_NULL_HANDLE;
	if (DeviceManager * dvcmngr = dynamic_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () ))
		{
		device = dvcmngr->GetDevice ();
		}
	if (device == VK_NULL_HANDLE)
		{
		LogError ( "Device not created" );
		return false;
		}

	uint32_t imageCount;
	vkGetSwapchainImagesKHR ( device, m_swapchain, &imageCount, nullptr );

	m_images.resize ( imageCount );
	vkGetSwapchainImagesKHR ( device, m_swapchain, &imageCount, m_images.data () );

	LogDebug ( "Got ", imageCount, " swapchain images" );
	return true;
	}

VkSurfaceFormatKHR SwapchainManager::ChooseSurfaceFormat ( const std::vector<VkSurfaceFormatKHR> & formats )
	{
	for (const auto & format : formats)
		{
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			 format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
			return format;
			}
		}
	return formats[ 0 ];
	}

VkPresentModeKHR SwapchainManager::ChoosePresentMode ( const std::vector<VkPresentModeKHR> & presentModes )
	{
	for (const auto & mode : presentModes)
		{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
			return mode;
			}
		}
	return VK_PRESENT_MODE_FIFO_KHR;
	}

VkExtent2D SwapchainManager::ChooseExtent ( const VkSurfaceCapabilitiesKHR & capabilities )
	{
	if (capabilities.currentExtent.width != UINT32_MAX)
		{
		return capabilities.currentExtent;
		}

	VkExtent2D extent = { 1024, 768 };  // Взять из WindowSystem

	extent.width = std::max ( capabilities.minImageExtent.width,
							  std::min ( capabilities.maxImageExtent.width, extent.width ) );
	extent.height = std::max ( capabilities.minImageExtent.height,
							   std::min ( capabilities.maxImageExtent.height, extent.height ) );

	return extent;
	}

void SwapchainManager::CleanupSwapChain ()
	{
	LogDebug ( "Cleaning up swapchain resources..." );

	auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
	if (!deviceMgr) return;

	auto device = deviceMgr->GetDevice ();
	if (device == VK_NULL_HANDLE) return;


	for (auto imageView : m_ImageViews)
		{
		if (imageView != VK_NULL_HANDLE)
			{
			vkDestroyImageView ( device, imageView, nullptr );
			}
		}
	m_ImageViews.clear ();

	// Cleanup swapchain
	if (m_swapchain != VK_NULL_HANDLE)
		{
		vkDestroySwapchainKHR ( device, m_swapchain, nullptr );
		m_swapchain = VK_NULL_HANDLE;
		}

	// Clear images (they are owned by swapchain, so no need to destroy)
	m_images.clear ();


	m_imageFormat = VK_FORMAT_UNDEFINED;
	m_extent = { 0, 0 };

	// Reset capabilities
	m_Capabilities = {};

	LogDebug ( "Swapchain resources cleaned up" );
	}

bool SwapchainManager::AcquireNextImage ( VkSemaphore SignalSemaphore, uint32_t & ImageIndex )
	{
	auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
	VkDevice device = deviceMgr->GetDevice ();

	VkResult result = vkAcquireNextImageKHR (
		device,
		m_swapchain,
		UINT64_MAX,
		SignalSemaphore,
		VK_NULL_HANDLE,
		&ImageIndex );

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
		LogDebug ( "Swapchain out of date, recreating..." );
		RecreateSwapChain ();
		return false;
		}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
		LogError ( "Failed to acquire next image: ", static_cast< int >( result ) );
		return false;
		}

	return true;
	}

bool SwapchainManager::Present ( VkSemaphore WaitSemaphore, uint32_t ImageIndex )
	{
	auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
	if (!deviceMgr)
		{
		LogError ( "Present: deviceMgr is null" );
		return false;
		}

	VkQueue presentQueue = deviceMgr->GetPresentQueue ();
	if (presentQueue == VK_NULL_HANDLE)
		{
		LogError ( "Present: presentQueue is null" );
		return false;
		}

	if (m_swapchain == VK_NULL_HANDLE)
		{
		LogError ( "Present: swapchain is null" );
		return false;
		}

	VkPresentInfoKHR presentInfo {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &WaitSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.pImageIndices = &ImageIndex;

	VkResult result = vkQueuePresentKHR ( presentQueue, &presentInfo );


	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
		LogDebug ( "Swapchain out of date or suboptimal, recreating..." );
		RecreateSwapChain ();
		return false;
		}
	else if (result != VK_SUCCESS)
		{
		LogError ( "Failed to present image: ", static_cast< int >( result ) );
		return false;
		}

	return true;
	}

bool SwapchainManager::RecreateSwapChain ()
	{
	LogDebug ( "Recreating swapchain..." );

	auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
	if (!deviceMgr) return false;

	VkDevice device = deviceMgr->GetDevice ();
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	if (InstanceManager * inst = dynamic_cast< InstanceManager * >( m_info->Managers.InstanceManager.get () ))
		{
		surface = inst->GetSurface ();
		}

	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( deviceMgr->GetPhysicalDevice (), surface, &capabilities );

	// Ждём, пока окно не будет восстановлено
	while (capabilities.currentExtent.width == 0 || capabilities.currentExtent.height == 0)
		{
			// Проверяем, не закрыто ли окно
		if (glfwWindowShouldClose ( m_info->Window ))
			{
			return false;
			}

			// Проверяем состояние окна (иконфицировано/свёрнуто)
		if (glfwGetWindowAttrib ( m_info->Window, GLFW_ICONIFIED ))
			{
				// Окно свёрнуто - ждём
			std::this_thread::sleep_for ( std::chrono::milliseconds ( 16 ) );
			}
		else
			{
				// Окно не свёрнуто, но размер 0 - возможно, ошибка
			LogDebug ( "  Window not iconified but size is 0, waiting..." );
			std::this_thread::sleep_for ( std::chrono::milliseconds ( 16 ) );
			}
		glfwPollEvents ();
			// Обновляем capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( deviceMgr->GetPhysicalDevice (), surface, &capabilities );
		}

		// Wait for device idle
	vkDeviceWaitIdle ( device );

	// Store old swapchain
	VkSwapchainKHR oldSwapchain = m_swapchain;

	// Clean up views only (not the swapchain itself)
	for (auto imageView : m_ImageViews)
		{
		vkDestroyImageView ( device, imageView, nullptr );
		}
	m_ImageViews.clear ();
	m_images.clear ();

	// Reset swapchain handle
	m_swapchain = VK_NULL_HANDLE;

	// Recreate with old swapchain
	try
		{
		CreateSwapchain ( oldSwapchain );
		CreateImageViews ();
		}
		catch (const std::exception & e)
			{
			LogError ( "Failed to recreate swapchain: ", e.what () );

			// Restore old swapchain on failure
			if (m_swapchain == VK_NULL_HANDLE)
				{
				m_swapchain = oldSwapchain;
				}
			return false;
			}

		// Destroy old swapchain
		if (oldSwapchain != VK_NULL_HANDLE)
			{
			vkDestroySwapchainKHR ( device, oldSwapchain, nullptr );
			}

		LogDebug ( "Swapchain recreated successfully" );
		SetSwapChainRecreated ( true );
		return true;
	}