#pragma once

#include "KE/GameFramework/Components/BaseComponent.h"
#include <functional>
#include <glm/glm.hpp>
#include "Core/KeyDefines.h"  // Включаем KeyDefines вместо InputSystem

class CPawn;
class CInputSystem;

class KE_API CInputComponent : public CBaseComponent
    {
    CHUDDO_DECLARE_CLASS ( CInputComponent, CBaseComponent );

    public:
        CInputComponent ( CObject * inOwner = nullptr, const std::string & inDisplayName = "InputComponent" );
        virtual ~CInputComponent ();

        virtual void InitComponent () override;
        virtual void Tick ( float DeltaTime ) override;
        virtual void OnBeginPlay () override;
        void OnEndPlay ();

        // ========== Input query methods (int версии) ==========
        bool IsKeyPressed ( int key ) const;
        bool IsKeyJustPressed ( int key ) const;
        bool IsKeyJustReleased ( int key ) const;
        bool IsKeyHeld ( int key ) const;

        bool IsMouseButtonPressed ( int button ) const;
        bool IsMouseButtonJustPressed ( int button ) const;
        bool IsMouseButtonJustReleased ( int button ) const;
        bool IsMouseButtonHeld ( int button ) const;

        // ========== Input query methods (enum class версии) ==========
        bool IsKeyPressed ( EKeys key ) const { return IsKeyPressed ( ToInt ( key ) ); }
        bool IsKeyJustPressed ( EKeys key ) const { return IsKeyJustPressed ( ToInt ( key ) ); }
        bool IsKeyJustReleased ( EKeys key ) const { return IsKeyJustReleased ( ToInt ( key ) ); }
        bool IsKeyHeld ( EKeys key ) const { return IsKeyHeld ( ToInt ( key ) ); }

        bool IsMouseButtonPressed ( EMouseButtons button ) const { return IsMouseButtonPressed ( ToInt ( button ) ); }
        bool IsMouseButtonJustPressed ( EMouseButtons button ) const { return IsMouseButtonJustPressed ( ToInt ( button ) ); }
        bool IsMouseButtonJustReleased ( EMouseButtons button ) const { return IsMouseButtonJustReleased ( ToInt ( button ) ); }
        bool IsMouseButtonHeld ( EMouseButtons button ) const { return IsMouseButtonHeld ( ToInt ( button ) ); }
        float GetMouseSensevity () const { return m_mouseSensevity; }
        FVector2D GetMousePosition () const;
        FVector2D GetMouseDelta () const;
        FVector2D GetScrollDelta () const;

        // ========== Binding methods (int версии - ядро) ==========
        void BindAction ( const std::string & actionName, int button, EInputEvent eventType,
                          std::function<void ()> callback );

        void BindAxis ( const std::string & axisName, int positiveKey, int negativeKey,
                        std::function<void ( float )> callback );

        void BindMouseAxis ( const std::string & axisName, int mouseAxis,
                             std::function<void ( float )> callback );

          // ========== Binding methods (enum class версии - обертки) ==========
        void BindAction ( const std::string & actionName, EKeys key, EInputEvent eventType,
                          std::function<void ()> delegate )
            {
            BindAction ( actionName, ToInt ( key ), eventType, delegate );
            }

        void BindAction ( const std::string & actionName, EMouseButtons button, EInputEvent eventType,
                          std::function<void ()> delegate )
            {
            BindAction ( actionName, ToInt ( button ), eventType, delegate );
            }

        void BindAxis ( const std::string & axisName, EKeys positiveKey, EKeys negativeKey,
                        std::function<void ( float )> delegate )
            {
            BindAxis ( axisName, ToInt ( positiveKey ), ToInt ( negativeKey ), delegate );
            }

        void BindMouseAxis ( const std::string & axisName, EMouseAxis mouseAxis,
                             std::function<void ( float )> delegate )
            {
            BindMouseAxis ( axisName, ToInt ( mouseAxis ), delegate );
            }

            // ========== Специализированные методы для мыши (для удобства) ==========
        void BindMouseAxisX ( const std::string & axisName, std::function<void ( float )> callback )
            {
            BindMouseAxis ( axisName, EMouseAxis::MouseX, callback );
            }

        void BindMouseAxisY ( const std::string & axisName, std::function<void ( float )> callback )
            {
            BindMouseAxis ( axisName, EMouseAxis::MouseY, callback );
            }

        void BindMouseScroll ( const std::string & axisName, std::function<void ( float )> callback )
            {
            BindMouseAxis ( axisName, EMouseAxis::MouseScroll, callback );
            }

            // ========== Утилиты для биндинга с чувствительностью ==========
        void BindMouseAxisXWithSensitivity ( const std::string & axisName,
                                             std::function<void ( float )> callback,
                                             float sensitivity = 0.1f );

        void BindMouseAxisYWithSensitivity ( const std::string & axisName,
                                             std::function<void ( float )> callback,
                                             float sensitivity = 0.1f );

          // ========== Управление биндингами ==========
        void UnbindAction ( const std::string & actionName );
        void UnbindAxis ( const std::string & axisName );
        void UnbindAll ();

        // ========== Mouse control ==========
        void SetMouseCursorVisible ( bool visible );
        void SetMousePosition ( const FVector2D & position );
        void SetMouseSensitivity ( float sensitivity );

        // ========== Getters ==========
        CPawn * GetOwningPawn () const;
        CInputSystem * GetInputSystem () const;

        // ========== Debug ==========
        void PrintBoundActions () const;
        void PrintBoundAxes () const;
        float GetLookSpeed () const { return LookSpeed; }
    protected:
        std::vector<std::string> m_BoundActions;
        std::vector<std::string> m_BoundAxes;
        float m_mouseSensevity = 0.f;
        float LookSpeed = 300.f;
    };

REGISTER_CLASS_FACTORY ( CInputComponent );