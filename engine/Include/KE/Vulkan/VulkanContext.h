#pragma once
#include "CoreMinimal.h"

// Forward declaration вместо include
struct GLFWwindow;
struct VkInfo;

class KE_API VulkanContext
    {
    public:
        VulkanContext ();
        virtual ~VulkanContext ();

        void SetWindow ( GLFWwindow * window ) { m_window = window; }
        bool PreInit ();
        bool Init ();
        void Update ( float Deltatime );
        void Shutdown ();
        void SetEngineName ( const std::string & inName ); 
        void SetAplicationName ( const std::string & inName );
    private:
        GLFWwindow * m_window = nullptr;
        bool bIsInitialized = false;
        VkInfo * m_info = nullptr;
    };