#pragma once

#include "CoreMinimal.h"
#include <GLFW/glfw3.h>
#include "Core/InputSystem.h"
#include "Render/Window.h"


// Единый диспетчер для всех GLFW колбэков
struct FGLFWDispatcher
    {
    CWindow * Window = nullptr;
    CInputSystem * InputSystem = nullptr;

    // Колбэк для клавиатуры
    static void KeyCallback ( GLFWwindow * window, int key, int scancode, int action, int mods )
        {
        FGLFWDispatcher * dispatcher = static_cast< FGLFWDispatcher * >( glfwGetWindowUserPointer ( window ) );
        if (dispatcher && dispatcher->InputSystem)
            {
            dispatcher->InputSystem->HandleKey ( key, scancode, action, mods );
            }
        }

        // Колбэк для мыши
    static void MouseButtonCallback ( GLFWwindow * window, int button, int action, int mods )
        {
        FGLFWDispatcher * dispatcher = static_cast< FGLFWDispatcher * >( glfwGetWindowUserPointer ( window ) );
        if (dispatcher && dispatcher->InputSystem)
            {
            dispatcher->InputSystem->HandleMouseButton ( button, action, mods );
            }
        }

        // Колбэк для позиции курсора
    static void CursorPositionCallback ( GLFWwindow * window, double xpos, double ypos )
        {
        FGLFWDispatcher * dispatcher = static_cast< FGLFWDispatcher * >( glfwGetWindowUserPointer ( window ) );
        if (dispatcher && dispatcher->InputSystem)
            {
            dispatcher->InputSystem->HandleMouseMove ( xpos, ypos );
            }
        }

        // Колбэк для скролла
    static void ScrollCallback ( GLFWwindow * window, double xoffset, double yoffset )
        {
        FGLFWDispatcher * dispatcher = static_cast< FGLFWDispatcher * >( glfwGetWindowUserPointer ( window ) );
        if (dispatcher && dispatcher->InputSystem)
            {
            dispatcher->InputSystem->HandleScroll ( xoffset, yoffset );
            }
        }

        // Колбэк для изменения размера окна
    static void WindowSizeCallback ( GLFWwindow * window, int width, int height )
        {
        FGLFWDispatcher * dispatcher = static_cast< FGLFWDispatcher * >( glfwGetWindowUserPointer ( window ) );
        if (dispatcher && dispatcher->Window)
            {
            dispatcher->Window->HandleResize ( width, height );
            }
        }

        // Колбэк для закрытия окна
    static void WindowCloseCallback ( GLFWwindow * window )
        {
        FGLFWDispatcher * dispatcher = static_cast< FGLFWDispatcher * >( glfwGetWindowUserPointer ( window ) );
        if (dispatcher && dispatcher->Window)
            {
            dispatcher->Window->HandleClose ();
            }
        }
    };