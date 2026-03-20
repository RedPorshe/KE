#pragma once
#include "CoreMinimal.h"

struct VkInfo;

class KE_API IVKManager
    {
    public:
        IVKManager () = default;
        virtual ~IVKManager () = default;

        virtual bool Init () = 0;
        virtual void Shutdown () = 0;  

        void SetInfoPtr ( VkInfo * inInfo ) { m_info = inInfo; }
      

        bool IsInitialized () const { return bIsInitialized; }
        virtual const std::string & GetManagerName () const = 0;

    protected:
        VkInfo * m_info = nullptr;  
        bool bIsInitialized = false;

       
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

        template<typename... Args>
        void LogInfo ( Args&&... args ) const
            {
            LOG_INFO ( "[", GetManagerName (), "] ", std::forward<Args> ( args )... );
            }

        template<typename... Args>
        void LogWarn ( Args&&... args ) const
            {
            LOG_WARN ( "[", GetManagerName (), "] ", std::forward<Args> ( args )... );
            }

        template<typename... Args>
        void LogTrace ( Args&&... args ) const
            {
            LOG_TRACE ( "[", GetManagerName (), "] ", std::forward<Args> ( args )... );
            }
    };