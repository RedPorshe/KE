#pragma once

#include "CoreMinimal.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <functional>


#include "Core/EngineInfo.h"

class KE_API CWindow
    {
    public:
        CWindow ( FEngineInfo & Info );
        virtual ~CWindow ();

        bool Initialize ();
        void Shutdown ();
        CWindow ( const CWindow & ) = delete;
        CWindow & operator=( const CWindow & ) = delete;
        VkSurfaceKHR CreateVulkanSurface () const;

        // Getters
        int GetWidth () const { return m_Width; }
        int GetHeight () const { return m_Height; }
        bool IsFullscreen () const { return m_bFullscreen; }
        bool IsInitialized () const { return bIsInitialized; }
        const std::string & GetTitle () const { return m_Title; }
        bool IsVSyncEnabled () const { return m_bIsVsyncEnabled; }

        // Window control
        void SetTitle ( const std::string & Title );
        void SetVSync ( bool bEnable );
        void SetFullscreen ( bool bFullscreen );
        void ShowCursor ( bool bShow );
        void SetSize ( int Width, int Height );
        float GetAspectRatio () const;
        bool IsValid () const { return m_info.WindowHandle != nullptr; }

        // Events
        void SetResizeCallback ( std::function<void ( int, int )> Callback ) { ResizeCallback = Callback; }
        void SetCloseCallback ( std::function<void ()> Callback ) { CloseCallback = Callback; }

        // Callback handlers (public для доступа из диспетчера)
        void HandleResize ( int width, int height );
        void HandleClose ();

    private:
        static void ErrorCallback ( int error, const char * description );

        FEngineInfo & m_info;

        int m_Width = 0;
        int m_Height = 0;
        bool m_bFullscreen = false;
        bool m_bIsResizable = false;
        bool m_bIsVsyncEnabled = false;
        bool bIsInitialized = false;
        std::string m_Title;

        // Callbacks
        std::function<void ( int, int )> ResizeCallback;
        std::function<void ()> CloseCallback;
    };