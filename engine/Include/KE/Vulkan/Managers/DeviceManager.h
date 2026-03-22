#pragma once
#include "KE/Vulkan/VKManager.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <set>

class DeviceManager final : public IVKManager
    {
    public:
        DeviceManager ();
        virtual ~DeviceManager () override;

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

        // Extended properties getters (Vulkan 1.1+)
        void GetPhysicalDeviceProperties2 ( VkPhysicalDeviceProperties2 & props2 ) const;
        void GetPhysicalDeviceFeatures2 ( VkPhysicalDeviceFeatures2 & features2 ) const;
        void GetPhysicalDeviceMemoryProperties2 ( VkPhysicalDeviceMemoryProperties2 & memProps2 ) const;
        void GetSurfaceCapabilities2 ( VkSurfaceKHR surface, VkSurfaceCapabilities2KHR & caps2 ) const;

        uint32_t FindMemoryType ( uint32_t TypeFilter, VkMemoryPropertyFlags Properties ) const;

    private:
        bool PickPhysicalDevice ();
        bool CreateLogicalDevice ();
        bool GetQueues ();
        bool IsDeviceSuitable ( VkPhysicalDevice device ) const;
        bool FindQueueFamilies ( VkPhysicalDevice device );
        bool CheckDeviceExtensionSupport ( VkPhysicalDevice device ) const;
        void AddDeviceFeatures ( VkPhysicalDeviceFeatures2 & features2 ) const;

        bool m_hasSwapchainSupport = false;
        std::vector<const char *> DeviceExtensions;

        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        uint32_t m_graphicsQueueFamily = 0;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        uint32_t m_presentQueueFamily = 0;
        VkQueue m_presentQueue = VK_NULL_HANDLE;

        friend class BufferManager;
        friend class RenderPassManager;
    };