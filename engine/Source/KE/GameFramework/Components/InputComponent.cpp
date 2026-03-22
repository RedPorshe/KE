#include "KE/GameFramework/Components/InputComponent.h"
#include "KE/GameFramework/Actors/Pawns/Pawn.h"
#include "KE/GameFramework/Actors/Controllers/PlayerController.h"
#include "KE/Engine.h"
#include "KE/Systems/InputSystem.h"
#include <GLFW/glfw3.h>
#include <algorithm>

CInputComponent::CInputComponent ( CObject * inOwner, const std::string & inDisplayName )
    : Super ( inOwner, inDisplayName )
    {}

CInputComponent::~CInputComponent ()
    {
    UnbindAll ();
    }

void CInputComponent::InitComponent ()
    {
    Super::InitComponent ();

    auto * inputSystem = GetInputSystem ();
    if (inputSystem)
        {
        inputSystem->RegisterInputComponent ( this );
        m_mouseSensevity = inputSystem->GetMouseSensitivity ();
        }
    }

void CInputComponent::Tick ( float DeltaTime )
    {
    Super::Tick ( DeltaTime );
    }

void CInputComponent::OnBeginPlay ()
    {
    Super::OnBeginPlay ();
    
    }

void CInputComponent::OnEndPlay ()
    {
    UnbindAll ();
    }



    // ========== Input query methods ==========

bool CInputComponent::IsKeyPressed ( int key ) const
    {
    auto * inputSystem = GetInputSystem ();
    return inputSystem ? inputSystem->IsKeyPressed ( key ) : false;
    }

bool CInputComponent::IsKeyJustPressed ( int key ) const
    {
    auto * inputSystem = GetInputSystem ();
    return inputSystem ? inputSystem->IsKeyJustPressed ( key ) : false;
    }

bool CInputComponent::IsKeyJustReleased ( int key ) const
    {
    auto * inputSystem = GetInputSystem ();
    return inputSystem ? inputSystem->IsKeyJustReleased ( key ) : false;
    }

bool CInputComponent::IsKeyHeld ( int key ) const
    {
    auto * inputSystem = GetInputSystem ();
    return inputSystem ? inputSystem->IsKeyHeld ( key ) : false;
    }

bool CInputComponent::IsMouseButtonPressed ( int button ) const
    {
    auto * inputSystem = GetInputSystem ();
    return inputSystem ? inputSystem->IsMouseButtonPressed ( button ) : false;
    }

bool CInputComponent::IsMouseButtonJustPressed ( int button ) const
    {
    auto * inputSystem = GetInputSystem ();
    return inputSystem ? inputSystem->IsMouseButtonJustPressed ( button ) : false;
    }

bool CInputComponent::IsMouseButtonJustReleased ( int button ) const
    {
    auto * inputSystem = GetInputSystem ();
    return inputSystem ? inputSystem->IsMouseButtonJustReleased ( button ) : false;
    }

bool CInputComponent::IsMouseButtonHeld ( int button ) const
    {
    auto * inputSystem = GetInputSystem ();
    return inputSystem ? inputSystem->IsMouseButtonHeld ( button ) : false;
    }

FVector2D CInputComponent::GetMousePosition () const
    {
    auto * inputSystem = GetInputSystem ();
    return inputSystem ? inputSystem->GetMousePosition () : FVector2D ( 0.0f );
    }

FVector2D CInputComponent::GetMouseDelta () const
    {
    auto * inputSystem = GetInputSystem ();
    return inputSystem ? inputSystem->GetMouseDelta () : FVector2D ( 0.0f );
    }

FVector2D CInputComponent::GetScrollDelta () const
    {
    auto * inputSystem = GetInputSystem ();
    return inputSystem ? inputSystem->GetScrollDelta () : FVector2D ( 0.0f );
    }

    // ========== Binding methods (int версии - ядро) ==========

void CInputComponent::BindAction ( const std::string & actionName, int button, EInputEvent eventType,
                                   std::function<void ()> callback )
    {
    auto * inputSystem = GetInputSystem ();
    if (inputSystem)
        {
        inputSystem->BindAction ( actionName, button, eventType, callback, this );
        m_BoundActions.push_back ( actionName );

        const char * typeStr = ( button >= GLFW_MOUSE_BUTTON_1 && button <= GLFW_MOUSE_BUTTON_LAST ) ?
            "mouse button" : "key";
        const char * eventStr = "";
        switch (eventType)
            {
                case EInputEvent::IE_Pressed: eventStr = "Pressed"; break;
                case EInputEvent::IE_Released: eventStr = "Released"; break;
                case EInputEvent::IE_Repeat: eventStr = "Repeat"; break;
                case EInputEvent::IE_DoubleClick: eventStr = "DoubleClick"; break;
            }

        LOG_DEBUG ( "[INPUTCOMPONENT] Bound action: ", actionName, " (", eventStr, ") to ",
                    typeStr, ": ", button );
        }
    }

void CInputComponent::BindAxis ( const std::string & axisName, int positiveKey, int negativeKey,
                                 std::function<void ( float )> callback )
    {
    auto * inputSystem = GetInputSystem ();
    if (inputSystem)
        {
        inputSystem->BindAxis ( axisName, positiveKey, negativeKey, callback, this );
        m_BoundAxes.push_back ( axisName );
        LOG_DEBUG ( "[INPUTCOMPONENT] Bound axis: ", axisName, " to keys: ",
                    positiveKey, " / ", negativeKey );
        }
    }

void CInputComponent::BindMouseAxis ( const std::string & axisName, int mouseAxis,
                                      std::function<void ( float )> callback )
    {
    auto * inputSystem = GetInputSystem ();
    if (inputSystem)
        {
        inputSystem->BindMouseAxis ( axisName, mouseAxis, callback, this );
        m_BoundAxes.push_back ( axisName );

        const char * axisStr = "";
        if (mouseAxis == ToInt ( EMouseAxis::MouseX ))
            axisStr = "Mouse X";
        else if (mouseAxis == ToInt ( EMouseAxis::MouseY ))
            axisStr = "Mouse Y";
        else if (mouseAxis == ToInt ( EMouseAxis::MouseScroll ))
            axisStr = "Mouse Scroll";
        else
            axisStr = "Unknown";

        LOG_DEBUG ( "[INPUTCOMPONENT] Bound mouse axis: ", axisName, " to ", axisStr );
        }
    }

    // ========== Специализированные методы для мыши ==========

void CInputComponent::BindMouseAxisXWithSensitivity ( const std::string & axisName,
                                                      std::function<void ( float )> callback,
                                                      float sensitivity )
    {
    BindMouseAxisX ( axisName, [ callback, sensitivity ] ( float Value )
                     {
                     callback ( Value * sensitivity );
                     } );
    }

void CInputComponent::BindMouseAxisYWithSensitivity ( const std::string & axisName,
                                                      std::function<void ( float )> callback,
                                                      float sensitivity )
    {
    BindMouseAxisY ( axisName, [ callback, sensitivity ] ( float Value )
                     {
                     callback ( Value * sensitivity );
                     } );
    }

    // ========== Управление биндингами ==========

void CInputComponent::UnbindAction ( const std::string & actionName )
    {
    auto * inputSystem = GetInputSystem ();
    if (inputSystem)
        {
        inputSystem->UnbindAction ( actionName, this );

        auto it = std::find ( m_BoundActions.begin (), m_BoundActions.end (), actionName );
        if (it != m_BoundActions.end ())
            {
            m_BoundActions.erase ( it );
            }
        }
    }

void CInputComponent::UnbindAxis ( const std::string & axisName )
    {
    auto * inputSystem = GetInputSystem ();
    if (inputSystem)
        {
        inputSystem->UnbindAxis ( axisName, this );

        auto it = std::find ( m_BoundAxes.begin (), m_BoundAxes.end (), axisName );
        if (it != m_BoundAxes.end ())
            {
            m_BoundAxes.erase ( it );
            }
        }
    }

void CInputComponent::UnbindAll ()
    {
    auto * inputSystem = GetInputSystem ();
    if (inputSystem)
        {
        inputSystem->UnbindAllForComponent ( this );
        }

    m_BoundActions.clear ();
    m_BoundAxes.clear ();
    LOG_DEBUG ( "[INPUTCOMPONENT] Unbound all actions and axes" );
    }

    // ========== Mouse control ==========

void CInputComponent::SetMouseCursorVisible ( bool visible )
    {
    auto * inputSystem = GetInputSystem ();
    if (inputSystem)
        {
        inputSystem->SetMouseCursorVisible ( visible );
        }
    }

void CInputComponent::SetMousePosition ( const FVector2D & position )
    {
    auto * inputSystem = GetInputSystem ();
    if (inputSystem)
        {
        inputSystem->SetMousePosition ( position );
        }
    }

void CInputComponent::SetMouseSensitivity ( float sensitivity )
    {
    auto * inputSystem = GetInputSystem ();
    if (inputSystem)
        {
        inputSystem->SetMouseSensitivity ( sensitivity );
        }
    }

    // ========== Getters ==========

CPawn * CInputComponent::GetOwningPawn () const
    {
    return dynamic_cast< CPawn * >( GetOwner () );
    }

CInputSystem * CInputComponent::GetInputSystem () const
    {
    return CEngine::Get().GetInputSystem();
    }

    // ========== Debug methods ==========

void CInputComponent::PrintBoundActions () const
    {
    LOG_DEBUG ( "[INPUTCOMPONENT] Bound actions for ", GetName (), ":" );
    for (const auto & action : m_BoundActions)
        {
        LOG_DEBUG ( "  - ", action );
        }
    }

void CInputComponent::PrintBoundAxes () const
    {
    LOG_DEBUG ( "[INPUTCOMPONENT] Bound axes for ", GetName (), ":" );
    for (const auto & axis : m_BoundAxes)
        {
        LOG_DEBUG ( "  - ", axis );
        }
    }