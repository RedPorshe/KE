#include "Render/Window.h"
#include <iostream>

CWindow::CWindow ( FEngineInfo & Info ) : m_info ( Info ) {}

CWindow::~CWindow () {
    Shutdown ();
    }

bool CWindow::Initialize ()
    {
    static bool glfwInitialized = false;
    if (!glfwInitialized)
        {
        if (glfwInit () != GLFW_TRUE)
            {
            LOG_ERROR ( "Failed to init GLFW" );
            return false;
            }
        glfwInitialized = true;
        }

    glfwSetErrorCallback ( ErrorCallback );
    glfwWindowHint ( GLFW_CLIENT_API, GLFW_NO_API );

    if (m_info.WindowInfo.Fullscreen)
        {
        glfwWindowHint ( GLFW_DECORATED, GLFW_TRUE );
        glfwWindowHint ( GLFW_SCALE_TO_MONITOR, GLFW_TRUE );

        GLFWmonitor * primaryMonitor = glfwGetPrimaryMonitor ();
        const GLFWvidmode * mode = glfwGetVideoMode ( primaryMonitor );

        m_info.WindowHandle = glfwCreateWindow ( mode->width, mode->height,
                                                 m_info.WindowInfo.Title.c_str (),
                                                 primaryMonitor, nullptr );

        if (m_info.WindowHandle)
            {
            m_Width = mode->width;
            m_Height = mode->height;
            m_bFullscreen = true;
            }
        }
    else
        {
        glfwWindowHint ( GLFW_RESIZABLE, m_info.WindowInfo.Resizable ? GLFW_TRUE : GLFW_FALSE );
        glfwWindowHint ( GLFW_DECORATED, GLFW_TRUE );

        m_info.WindowHandle = glfwCreateWindow ( m_info.WindowInfo.Width, m_info.WindowInfo.Height,
                                                 m_info.WindowInfo.Title.c_str (), nullptr, nullptr );

        m_Width = m_info.WindowInfo.Width;
        m_Height = m_info.WindowInfo.Height;
        m_bFullscreen = false;
        }

    if (!m_info.WindowHandle)
        {
        LOG_ERROR ( "Failed to create GLFW window" );
        glfwTerminate ();
        return false;
        }

        // НЕ УСТАНАВЛИВАЕМ USER POINTER ЗДЕСЬ! Это будет делать Engine

    m_Title = m_info.WindowInfo.Title;
    m_bIsResizable = m_info.WindowInfo.Resizable;
    m_bIsVsyncEnabled = m_info.WindowInfo.VSync;
    bIsInitialized = true;

    glfwShowWindow ( m_info.WindowHandle );

    LOG_DEBUG ( "Window created successfully: ", m_Width, "x", m_Height );
    return true;
    }

void CWindow::Shutdown () {
   
    if (m_info.WindowHandle)
        {
        LOG_DEBUG ( "Destroying GLFW window..." );
        glfwDestroyWindow ( m_info.WindowHandle );
        m_info.WindowHandle = nullptr;
        }
    glfwTerminate ();
    bIsInitialized = false;
    }

VkSurfaceKHR CWindow::CreateVulkanSurface () const {
    if (!m_info.WindowHandle || !m_info.HasVulkanContext())
        {
        LOG_ERROR ( "Cannot create Vulkan surface: invalid window or instance" );
        return VK_NULL_HANDLE;
        }

    VkSurfaceKHR surface;
    VkResult result = glfwCreateWindowSurface ( m_info.Vulkan.Instance, m_info.WindowHandle, nullptr, &surface );

    if (result != VK_SUCCESS)
        {
        LOG_ERROR ( "Failed to create Vulkan surface: ", static_cast< int >( result ) );
        return VK_NULL_HANDLE;
        }

    LOG_DEBUG ( "Vulkan surface created successfully" );
    m_info.Vulkan.Surface = surface;
    return surface;
    }

void CWindow::SetTitle ( const std::string & Title ) {
    if (m_info.WindowHandle)
        {
        glfwSetWindowTitle ( m_info.WindowHandle, Title.c_str () );
        m_Title = Title;
        }
    }

void CWindow::SetVSync ( bool bEnable ) {
    m_bIsVsyncEnabled = bEnable;
    }

void CWindow::SetFullscreen ( bool bFullscreen ) {
    if (bFullscreen == m_bFullscreen) return;

    if (m_info.WindowHandle)
        {
        if (bFullscreen)
            {
            GLFWmonitor * monitor = glfwGetPrimaryMonitor ();
            const GLFWvidmode * mode = glfwGetVideoMode ( monitor );
            glfwSetWindowMonitor ( m_info.WindowHandle, monitor, 0, 0,
                                   mode->width, mode->height, mode->refreshRate );
            m_Width = mode->width;
            m_Height = mode->height;
            }
        else
            {
            glfwSetWindowMonitor ( m_info.WindowHandle, nullptr,
                                   m_info.WindowInfo.PosX, m_info.WindowInfo.PosY,
                                   m_info.WindowInfo.Width, m_info.WindowInfo.Height, 0 );
            m_Width = m_info.WindowInfo.Width;
            m_Height = m_info.WindowInfo.Height;
            }
        m_bFullscreen = bFullscreen;
        }
    }

void CWindow::SetSize ( int Width, int Height ) {
    if (m_info.WindowHandle && !m_bFullscreen)
        {
        glfwSetWindowSize ( m_info.WindowHandle, Width, Height );
        }
    else if (m_bFullscreen)
        {
        LOG_WARN ( "Cannot set window size in fullscreen mode" );
        }
    }

float CWindow::GetAspectRatio () const {
    return m_Height > 0 ? static_cast< float >( m_Width ) / static_cast< float >( m_Height ) : 0.0f;
    }

void CWindow::ShowCursor ( bool bShow ) {
    if (m_info.WindowHandle)
        {
        glfwSetInputMode ( m_info.WindowHandle, GLFW_CURSOR,
                           bShow ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED );
        }
    }

void CWindow::ErrorCallback ( int error, const char * description ) {
    LOG_ERROR ( "GLFW Error [", error, "]: ", description );
    }

    // Обработчики событий (вызываются из диспетчера)
void CWindow::HandleResize ( int width, int height ) {
    m_Width = width;
    m_Height = height;
    if (ResizeCallback)
        {
        ResizeCallback ( width, height );
        }
    }

void CWindow::HandleClose () {
    if (CloseCallback)
        {
        CloseCallback ();
        }
    }