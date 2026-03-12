#include "Core/EngineInfo.h"
#include "Render/Vulkan/Managers/DeviceManager.h"

void FVulkanInfo::Shutdown ()
	{	
	VulkanContext.reset ();
	}

bool FEngineInfo::HasVulkanContext () const
	{
	return Vulkan.Instance != VK_NULL_HANDLE;
	}

bool FEngineInfo::HasSwapchain () const
	{
	return Vulkan.SwapChainManager != nullptr &&
		Vulkan.SwapChainManager->IsInitialized ();
	}

VkExtent2D FEngineInfo::GetCurrentExtent () const
	{

	if (HasSwapchain ())
		{
		return Vulkan.SwapChainManager->GetExtent ();
		}

	return { static_cast< uint32_t >( WindowInfo.Width ),
		static_cast< uint32_t >( WindowInfo.Height ) };

	}

float FEngineInfo::GetAspectRatio () const
	{
	VkExtent2D Extent = GetCurrentExtent ();
	return Extent.height > 0
		? static_cast< float >( Extent.width ) / static_cast< float >( Extent.height )
		: 16.0f / 9.0f;
	}
