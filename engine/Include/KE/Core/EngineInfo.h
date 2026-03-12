#pragma once
#include "CoreMinimal.h"
#include "Render/Vulkan/VulkanInterface.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <memory>



struct FWindowInfo
	{
	std::string Title = "Game Window";
	int Width = 1280;
	int Height = 720;
	bool Fullscreen = false;
	bool Resizable = true;
	bool VSync = true;
	int PosX = -1;
	int PosY = -1;
	bool UseVulkan = true;
	};

// Структура для хранения параметров Vulkan
struct FVulkanInfo
	{

	VkInstance Instance = VK_NULL_HANDLE;
	VkSurfaceKHR Surface = VK_NULL_HANDLE;


	std::vector<const char *> InstanceExtensions;
	std::vector<const char *> InstanceLayers;


	std::unique_ptr<IVulkanManager> VulkanContext;
	std::unique_ptr<IVulkanManager> DeviceManager;
	std::unique_ptr<IVulkanManager> SwapChainManager;
	std::unique_ptr<IVulkanManager> CommandManager;
	std::unique_ptr<IVulkanManager> SyncManager;
	std::unique_ptr<IVulkanManager> DescriptorManager;
	std::unique_ptr<IVulkanManager> PipelineManager;
	std::unique_ptr<IVulkanManager> BufferManager;
	std::unique_ptr<IVulkanManager> RenderPassManager;

	// Shutdown method
	void Shutdown ();

	};


struct FVulkanContextCreateInfo
	{
	std::vector<const char *> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	// Validation
#ifdef _DEBUG
	bool bEnableValidationLayers = true;
#else
	bool bEnableValidationLayers = true;
#endif // _DEBUG

	VkPresentModeKHR PreferredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	uint32_t DesiredImageCount = 3;  // Triple buffering
	VkSurfaceTransformFlagBitsKHR PreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	VkCompositeAlphaFlagBitsKHR CompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	VkFormat DepthFormat = VK_FORMAT_D32_SFLOAT;

	VkCommandPoolCreateFlags CommandPoolFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkSampleCountFlagBits MsaaSamples = VK_SAMPLE_COUNT_1_BIT;
	};

#include "Render/Vulkan/Managers/DeviceManager.h"

// Главная структура EngineInfo
struct FEngineInfo
	{
	// Engine info
	std::string EngineName = "ChuddoEngine";

	// Window info
	FWindowInfo WindowInfo;
	GLFWwindow * WindowHandle = nullptr;

	// Vulkan info
	FVulkanInfo Vulkan;
	FVulkanContextCreateInfo VulkanCreateInfo;

	// Helper methods
	bool HasVulkanContext () const;
	bool HasSwapchain () const;

	VkExtent2D GetCurrentExtent () const;
	// Get aspect ratio
	float GetAspectRatio () const;

	};