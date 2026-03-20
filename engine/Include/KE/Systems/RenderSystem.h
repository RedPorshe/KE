#pragma once
#include "KE/EngineObject.h"

struct GLFWwindow;

// Forward declaration
class VulkanContext;

class KE_API RenderSystem : public IEngineSystem
    {
    public:
        RenderSystem ();
        virtual ~RenderSystem ();  // Объявление, но не = default

        bool PreInit () override;
        bool Init () override;
        void Shutdown () override;
        void Update ( float DeltaTime ) override;
        void SetWindow ( GLFWwindow * inWindow ) { m_window = inWindow; }
        const std::string GetSystemName () const override;
        void SetEngineName ( const std::string & inName ); 
        void SetAplicationName ( const std::string & inName );

    private:
        TUniquePtr<VulkanContext> m_vulkanContext;
      
        GLFWwindow * m_window = nullptr;
    };