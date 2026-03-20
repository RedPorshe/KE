#pragma once
#include "KE/Vulkan/VKManager.h"
#include <vulkan/vulkan.h>

class DeviceManager final: public IVKManager
	{
	public:
		DeviceManager ();
		virtual ~DeviceManager () override;

		// Inherited via IVKManager
		bool Init () override;
		void Shutdown () override;
		const std::string & GetManagerName () const override;

		VkPhysicalDevice GetPhysicalDevice () const { return m_physicalDevice; }
		VkDevice GetDevice () const { return m_device; }
		uint32_t GetGraphicsQueueFamily () const { return m_graphicsQueueFamily; }
		VkQueue GetGraphicsQueue () const { return m_graphicsQueue; }
		uint32_t GetPresentQueueFamily () const { return m_presentQueueFamily; }
		VkQueue GetPresentQueue () const { return m_presentQueue; }

		bool HasSwapchainSupport () const { return m_hasSwapchainSupport; }

	private:
		bool PickPhysicalDevice ();
		bool CreateLogicalDevice ();
		bool GetQueues ();

		bool IsDeviceSuitable ( VkPhysicalDevice device ) const;
		bool CheckDeviceExtensionSupport ( VkPhysicalDevice device ) const;
		bool FindQueueFamilies ( VkPhysicalDevice device );

		bool m_hasSwapchainSupport = false;
		uint32_t FindMemoryType ( uint32_t TypeFilter, VkMemoryPropertyFlags Properties ) const;
		TVector<const char *> DeviceExtensions;

		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		uint32_t m_graphicsQueueFamily = 0;
		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
		uint32_t m_presentQueueFamily = 0;
		VkQueue m_presentQueue = VK_NULL_HANDLE;
		friend class BufferManager;
	};