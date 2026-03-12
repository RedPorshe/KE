#pragma once
#include "Render/Vulkan/VulkanInterface.h"
#include <vulkan/vulkan.h>

struct FEngineInfo;

class KE_API CVulkanContext final : public IVulkanManager
    {
    public:
        CVulkanContext ( FEngineInfo & Info );
        virtual ~CVulkanContext ();

        bool Initialize () override;
        void Shutdown () override;
        const char * GetManagerName () const override;

        // Getters for created objects
        VkInstance GetInstance () const; 
        VkSurfaceKHR GetSurface () const; 

    private:
        bool CreateInstance ();
        bool CreateSurface ();
        bool CheckValidationLayerSupport () const;

    private:
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
    };