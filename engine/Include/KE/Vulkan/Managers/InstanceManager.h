#pragma once
#include "KE/Vulkan/VKManager.h"
#include <vulkan/vulkan.h>

class KE_API InstanceManager final : public IVKManager
    {
    public:
        InstanceManager ();
        virtual ~InstanceManager () override;

        bool Init () override;
        void Shutdown () override;
        const std::string & GetManagerName () const override;

        VkInstance GetInstance () const { return m_Instance; }
        VkSurfaceKHR GetSurface () const { return surface; }
        bool HasSurface () const { return surface != VK_NULL_HANDLE; }
    private:
        bool CreateInstance ();
        bool CreateSurface ();
        friend VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback (
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData,
            void * pUserData );

        VkSurfaceKHR surface = VK_NULL_HANDLE;

#ifdef _DEBUG
        bool CreateDebugMessenger ();
#endif

        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

#ifdef _DEBUG
        bool bIsValidationEnabled = true;
#else
        bool bIsValidationEnabled = false;
#endif
    };