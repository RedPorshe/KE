#pragma once
#include "Render/Vulkan/VulkanInterface.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <set>

struct FEngineInfo;

class KE_API CDeviceManager final : public IVulkanManager
    {
    public:
        CDeviceManager ( FEngineInfo & info );
        virtual ~CDeviceManager ();

        // IVulkanManager implementation
        virtual bool Initialize () override;
        virtual void Shutdown () override;
        virtual const char * GetManagerName () const override;

        // Getters
        VkDevice GetDevice () const { return m_Device; }
        VkPhysicalDevice GetPhysicalDevice () const { return m_PhysicalDevice; }

        VkQueue GetGraphicsQueue () const { return m_GraphicsQueue; }
        VkQueue GetPresentQueue () const { return m_PresentQueue; }

        uint32_t GetGraphicsQueueFamily () const { return m_GraphicsFamily; }
        uint32_t GetPresentQueueFamily () const { return m_PresentFamily; }

        // Memory utilities
        uint32_t FindMemoryType ( uint32_t TypeFilter, VkMemoryPropertyFlags Properties ) const;

    private:
        // Physical device selection
        bool PickPhysicalDevice ();
        bool IsDeviceSuitable ( VkPhysicalDevice Device ) const;
        bool CheckDeviceExtensionSupport ( VkPhysicalDevice Device ) const;
        bool FindQueueFamilies ( VkPhysicalDevice Device );

        // Logical device creation
        bool CreateLogicalDevice ();

    private:
        // Physical device
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

        // Logical device
        VkDevice m_Device = VK_NULL_HANDLE;

        // Queue families
        uint32_t m_GraphicsFamily = UINT32_MAX;
        uint32_t m_PresentFamily = UINT32_MAX;

        // Queues
        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;
    };