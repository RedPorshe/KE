// Render/Vulkan/VulkanInterface.h
#pragma once
#include "CoreMinimal.h"
#include <vulkan/vulkan.h>

struct FEngineInfo;

class KE_API IVulkanManager
    {
    public:
        IVulkanManager ( FEngineInfo & Info ) : m_Info ( Info ), m_bInitialized ( false ) {}
        virtual ~IVulkanManager () = default;  
        virtual bool Initialize () = 0;
        virtual void Shutdown () = 0;
        virtual bool IsInitialized () const { return m_bInitialized; }
        virtual const char * GetManagerName () const = 0;

        
        virtual VkExtent2D GetExtent () { return VkExtent2D { 0, 0 }; }

    protected:
        FEngineInfo & m_Info;
        bool m_bInitialized;

        template<typename... Args>
        void LogDebug ( Args&&... args ) const
            {
            LOG_DEBUG ( "[", GetManagerName (), "] ", std::forward<Args> ( args )... );
            }

        template<typename... Args>
        void LogError ( Args&&... args ) const
            {
            LOG_ERROR ( "[", GetManagerName (), "] ", std::forward<Args> ( args )... );
            }
    };