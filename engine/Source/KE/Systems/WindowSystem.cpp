#include "KE/Systems/WindowSystem.h"
#include "KE/Systems/GLFWDispatcher.h"
#include <GLFW/glfw3.h>
#include "KE/Engine.h"

bool WindowSystem::PreInit ()
    {
    LogDebug ( "PreInit - Initializing GLFW" );

    if (!glfwInit ())
        {
        LogError ( "Failed to initialize GLFW" );
        return false;
        }

    return true;
    }

bool WindowSystem::Init ()
    {
    LogDebug ( "Init - Creating window: ", m_title, " (", Extent.Width, "x", Extent.Height, ")" );

    // Настройки для Vulkan
    glfwWindowHint ( GLFW_CLIENT_API, GLFW_NO_API );
    glfwWindowHint ( GLFW_RESIZABLE, GLFW_TRUE );

    m_WindowHandle = glfwCreateWindow (
        Extent.Width,
        Extent.Height,
        m_title.c_str (),
        bIsFullscreen ? glfwGetPrimaryMonitor () : nullptr,
        nullptr
    );

    if (!m_WindowHandle)
        {
        LogError ( "Failed to create window" );
        return false;
        }

        // Настраиваем диспетчер
    SetupCallbacks ();

    if (OnWindowCreated)
        {
        OnWindowCreated ( m_WindowHandle );
        }

    bIsInitialized = true;
    LogInfo ( "Window created successfully" );
    return true;
    }

void WindowSystem::SetupCallbacks ()
    {
        // Создаём и настраиваем диспетчер
    FGLFWDispatcher * dispatcher = new FGLFWDispatcher ();
    dispatcher->Window = this;

    // Получаем InputSystem из Engine
    auto engine = GetEngine ();
    if (engine)
        {
        dispatcher->InputSystem = engine->GetInputSystem ();
        }

    glfwSetWindowUserPointer ( m_WindowHandle, dispatcher );

    // Устанавливаем колбэки
    glfwSetKeyCallback ( m_WindowHandle, FGLFWDispatcher::KeyCallback );
    glfwSetMouseButtonCallback ( m_WindowHandle, FGLFWDispatcher::MouseButtonCallback );
    glfwSetCursorPosCallback ( m_WindowHandle, FGLFWDispatcher::CursorPositionCallback );
    glfwSetScrollCallback ( m_WindowHandle, FGLFWDispatcher::ScrollCallback );
    glfwSetWindowSizeCallback ( m_WindowHandle, FGLFWDispatcher::WindowSizeCallback );
    glfwSetWindowCloseCallback ( m_WindowHandle, FGLFWDispatcher::WindowCloseCallback );
    }

void WindowSystem::HandleResize ( int width, int height )
    {
    SetExtent ( width, height );
    LogDebug ( "Window resized: ", width, "x", height );

    // Уведомляем подписчиков (например, RenderSystem)
    if (OnResize)
        {
        OnResize ( width, height );
        }
    }

void WindowSystem::HandleClose ()
    {
        // Запрашиваем закрытие окна
    if (m_WindowHandle)
        {
        glfwSetWindowShouldClose ( m_WindowHandle, true );
        }
    }

void WindowSystem::Shutdown ()
    {
    LogDebug ( "Shutting down window system" );

    if (m_WindowHandle)
        {
            // Очищаем диспетчер
        FGLFWDispatcher * dispatcher = static_cast< FGLFWDispatcher * >( glfwGetWindowUserPointer ( m_WindowHandle ) );
        if (dispatcher)
            {
            delete dispatcher;
            glfwSetWindowUserPointer ( m_WindowHandle, nullptr );
            }

        glfwDestroyWindow ( static_cast< GLFWwindow * >( m_WindowHandle ) );
        m_WindowHandle = nullptr;
        }

    glfwTerminate ();
    bIsInitialized = false;
    }

void WindowSystem::Update ( float DeltaTime )
    {
    if (!IsInitialized ()) return;

    glfwPollEvents ();

    // Проверка изменения размера окна (резервный вариант, если колбэк не сработал)
    int width, height;
    glfwGetFramebufferSize ( static_cast< GLFWwindow * >( m_WindowHandle ), &width, &height );

    if (static_cast< uint32_t >( width ) != Extent.Width ||
         static_cast< uint32_t >( height ) != Extent.Height)
        {
        SetExtent ( static_cast< uint32_t >( width ), static_cast< uint32_t >( height ) );
        LogDebug ( "Window resized: ", Extent.Width, "x", Extent.Height );

        // Уведомляем подписчиков
        if (OnResize)
            {
            OnResize ( width, height );
            }
        }

        // Проверка на закрытие
    if (glfwWindowShouldClose ( static_cast< GLFWwindow * >( m_WindowHandle ) ))
        {
        LogInfo ( "Window close requested" );
        GetEngine ()->RequestShutdown ();
        }
    }

const std::string WindowSystem::GetSystemName () const
    {
    return "WindowSystem";
    }

void WindowSystem::ShowCursor ( bool show )
    {
    glfwSetInputMode ( static_cast< GLFWwindow * >( m_WindowHandle ),
                       GLFW_CURSOR,
                       show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED );
    }