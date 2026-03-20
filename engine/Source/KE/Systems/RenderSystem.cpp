#include "KE/Systems/RenderSystem.h"
#include "KE/Vulkan/VulkanContext.h"  
#include "KE/Engine.h"

RenderSystem::RenderSystem ()
    : m_vulkanContext ( MakeUnique<VulkanContext> () )
    {
    LogDebug ( "RenderSystem Created" );
    }

    
RenderSystem::~RenderSystem () = default;  

bool RenderSystem::PreInit ()
    {
    LogInfo ( "Checking vulkan support" );
    if (!m_vulkanContext->PreInit ())
        {
        LogError ( "Failed to preInit vulkan context" );
        return false;
        }
    return true;
    }

bool RenderSystem::Init ()
    {
    LogInfo ( "Initializing RenderSystem" );

    m_window = GetEngine ()->GetWindowHandle ();
    if (!m_window)
        {
        LogError ( "Failed to get window handle" );
        return false;
        }
    LogInfo ( "Window handle obtained successfully" );

    m_vulkanContext->SetWindow ( m_window );

    if (!m_vulkanContext->Init ())
        {
        LogError ( "Failed to init vulkan context" );
        return false;
        }

    bIsInitialized = true;
    return true;
    }

void RenderSystem::Shutdown ()
    {
    LogInfo ( "Shutting down RenderSystem" );
    m_vulkanContext->Shutdown ();
    m_vulkanContext.reset ();
    bIsInitialized = false;
    }

void RenderSystem::Update ( float DeltaTime )
    {
    if (!IsInitialized ()) return;

    static int count = 0;
    if (count < 5)
        {
        LogInfo ( "Updating with delta: ", DeltaTime, " ms" );
        count++;
        }

    m_vulkanContext->Update ( DeltaTime );
    }

const std::string RenderSystem::GetSystemName () const
    {
    return "RenderSystem";
    }

void RenderSystem::SetEngineName ( const std::string & inName )
    {
    m_vulkanContext->SetEngineName ( inName );
    }

void RenderSystem::SetAplicationName ( const std::string & inName )
    {
    m_vulkanContext->SetAplicationName ( inName );
    }
