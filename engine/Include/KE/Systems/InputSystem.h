#pragma once
#include "CoreMinimal.h"
#include "KE/EngineObject.h"
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <functional>
#include <vector>
#include <glm/glm.hpp>
#include "Core/KeyDefines.h"

// Forward declarations
class CController;
class CInputComponent;
class CEngine;

// Делегаты
using InputActionDelegate = std::function<void ()>;
using InputAxisDelegate = std::function<void ( float )>;
using InputRepeatDelegate = std::function<void ()>;

class KE_API CInputSystem : public IEngineSystem
    {
    public:
        CInputSystem ();
        virtual ~CInputSystem ();

        // IEngineSystem overrides
        bool PreInit () override;
        bool Init () override;
        void Shutdown () override;
        void Update ( float DeltaTime ) override;
        const std::string GetSystemName () const override;

        // GLFW window association
        GLFWwindow * GetWindow () const;
        void SetWindow ( GLFWwindow * window );

        // Input state queries
        bool IsKeyPressed ( EKeys key ) const { return IsKeyPressed ( ToInt ( key ) ); }
        bool IsKeyJustPressed ( EKeys key ) const { return IsKeyJustPressed ( ToInt ( key ) ); }
        bool IsKeyJustReleased ( EKeys key ) const { return IsKeyJustReleased ( ToInt ( key ) ); }
        bool IsKeyHeld ( EKeys key ) const { return IsKeyHeld ( ToInt ( key ) ); }

        bool IsMouseButtonPressed ( EMouseButtons button ) const { return IsMouseButtonPressed ( ToInt ( button ) ); }
        bool IsMouseButtonJustPressed ( EMouseButtons button ) const { return IsMouseButtonJustPressed ( ToInt ( button ) ); }
        bool IsMouseButtonJustReleased ( EMouseButtons button ) const { return IsMouseButtonJustReleased ( ToInt ( button ) ); }
        bool IsMouseButtonHeld ( EMouseButtons button ) const { return IsMouseButtonHeld ( ToInt ( button ) ); }

        // Original methods (for internal use)
        bool IsKeyPressed ( int key ) const;
        bool IsKeyJustPressed ( int key ) const;
        bool IsKeyJustReleased ( int key ) const;
        bool IsKeyHeld ( int key ) const;

        bool IsMouseButtonPressed ( int button ) const;
        bool IsMouseButtonJustPressed ( int button ) const;
        bool IsMouseButtonJustReleased ( int button ) const;
        bool IsMouseButtonHeld ( int button ) const;

        FVector2D GetMousePosition () const;
        FVector2D GetMouseDelta () const;
        FVector2D GetScrollDelta () const;

        // Mouse control
        void SetMouseCursorVisible ( bool visible );
        void SetMousePosition ( const FVector2D & position );
        void SetMouseSensitivity ( float Sensitivity ) { m_MouseSensitivity = Sensitivity; }
        float GetMouseSensitivity () const { return m_MouseSensitivity; }

        // Controller input processing
        void ProcessControllerInput ( CController * Controller, float DeltaTime );

        // Binding methods with enum class
        void BindAction ( const std::string & actionName, EKeys key, EInputEvent eventType,
                          InputActionDelegate delegate, CInputComponent * component = nullptr );

        void BindAction ( const std::string & actionName, EMouseButtons button, EInputEvent eventType,
                          InputActionDelegate delegate, CInputComponent * component = nullptr );

        void BindAxis ( const std::string & axisName, EKeys positiveKey, EKeys negativeKey,
                        InputAxisDelegate delegate, CInputComponent * component = nullptr );

        void BindMouseAxis ( const std::string & axisName, EMouseAxis mouseAxis,
                             InputAxisDelegate delegate, CInputComponent * component = nullptr );

          // Original binding methods (for backward compatibility)
        void BindAction ( const std::string & actionName, int button, EInputEvent eventType,
                          InputActionDelegate delegate, CInputComponent * component = nullptr );

        void BindAxis ( const std::string & axisName, int positiveKey, int negativeKey,
                        InputAxisDelegate delegate, CInputComponent * component = nullptr );

        void BindMouseAxis ( const std::string & axisName, int mouseAxis,
                             InputAxisDelegate delegate, CInputComponent * component = nullptr );

        void UnbindAction ( const std::string & actionName, CInputComponent * component = nullptr );
        void UnbindAxis ( const std::string & axisName, CInputComponent * component = nullptr );
        void UnbindAllForComponent ( CInputComponent * component );

        // Component registration
        void RegisterInputComponent ( CInputComponent * Component );
        void UnregisterInputComponent ( CInputComponent * Component );

        // Callback handlers
        void HandleKey ( int key, int scancode, int action, int mods );
        void HandleMouseButton ( int button, int action, int mods );
        void HandleMouseMove ( double xpos, double ypos );
        void HandleScroll ( double xoffset, double yoffset );

    private:
        struct KeyState
            {
            bool current = false;
            bool previous = false;
            bool justPressed = false;
            bool justReleased = false;
            float heldTime = 0.0f;
            };

        struct MouseButtonState
            {
            bool current = false;
            bool previous = false;
            bool justPressed = false;
            bool justReleased = false;
            float heldTime = 0.0f;
            };

        struct ActionBinding
            {
            int button;
            EInputEvent eventType;
            InputActionDelegate delegate;
            CInputComponent * owner = nullptr;
            };

        struct AxisBinding
            {
            int positiveKey = -1;
            int negativeKey = -1;
            int mouseAxis = -1;
            bool bIsMouseAxis = false;
            InputAxisDelegate delegate;
            CInputComponent * owner = nullptr;
            float value = 0.0f;
            };

        void UpdateMouseState ();
        void ProcessActions ( float DeltaTime );
        void ProcessAxes ( float DeltaTime );
        bool CacheWindow ();

        GLFWwindow * m_WindowHandle = nullptr;
       
        float m_MouseSensitivity = 0.1f;

        // Input state
        std::unordered_map<int, KeyState> m_KeyStates;
        std::unordered_map<int, MouseButtonState> m_MouseButtonStates;
        FVector2D m_MousePosition = FVector2D ( 0.0f );
        FVector2D m_LastMousePosition = FVector2D ( 0.0f );
        FVector2D m_MouseDelta = FVector2D ( 0.0f );
        FVector2D m_ScrollDelta = FVector2D ( 0.0f );

        // Input bindings
        std::unordered_map<std::string, ActionBinding> m_ActionBindings;
        std::unordered_map<std::string, AxisBinding> m_AxisBindings;

        // Registered input components
        std::vector<CInputComponent *> m_InputComponents;

        friend class CInputComponent;
    };