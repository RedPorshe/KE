#include "KE/Systems/InputSystem.h"
#include "KE/GameFramework/Components/InputComponent.h"
#include "KE/GameFramework/Actors/Controllers/PlayerController.h"
#include "KE/Engine.h"
#include "KE/GameFramework/Actors/Pawns/Pawn.h"
#include <GLFW/glfw3.h>
#include <algorithm>

//=============================================================================
// Construction / Destruction
//=============================================================================

CInputSystem::CInputSystem ()
    {
    LogDebug ( "Created" );
    }

CInputSystem::~CInputSystem ()
    {
    Shutdown ();
    }

    //=============================================================================
    // IEngineSystem Implementation
    //=============================================================================

bool CInputSystem::PreInit ()
    {
    LogDebug ( "PreInit - Checking input system" );
    return true;
    }

bool CInputSystem::Init ()
    {
    LogDebug ( "Initializing InputSystem" );

    if (!GetEngine ())
        {
        LogError ( "Engine not set" );
        return false;
        }

    if (!CacheWindow ())
        {
        LogError ( "Failed to cache window handle" );
        return false;
        }

    bIsInitialized = true;
    LogInfo ( "Input system initialized successfully" );
    return true;
    }

void CInputSystem::Shutdown ()
    {
    LogDebug ( "Shutting down input system" );

    m_ActionBindings.clear ();
    m_AxisBindings.clear ();
    m_KeyStates.clear ();
    m_MouseButtonStates.clear ();
    m_InputComponents.clear ();

    bIsInitialized = false;
    LogDebug ( "Input system shutdown complete" );
    }

void CInputSystem::Update ( float DeltaTime )
    {
    if (!IsInitialized ()) return;

    if (!m_WindowHandle)
        {
        LogWarn ( "Window handle is null" );
        return;
        }

    if (glfwWindowShouldClose ( m_WindowHandle ))
        {
        return;
        }

        // Update key held time
    for (auto & [key, state] : m_KeyStates)
        {
        if (state.current)
            {
            state.heldTime += DeltaTime;
            }
        else
            {
            state.heldTime = 0.0f;
            }
        }

        // Update mouse button held time
    for (auto & [button, state] : m_MouseButtonStates)
        {
        if (state.current)
            {
            state.heldTime += DeltaTime;
            }
        else
            {
            state.heldTime = 0.0f;
            }
        }

    UpdateMouseState ();
    ProcessActions ( DeltaTime );
    ProcessAxes ( DeltaTime );

    m_ScrollDelta = FVector2D ( 0.0f );

    // Reset justPressed/justReleased flags
    for (auto & [key, state] : m_KeyStates)
        {
        state.justPressed = false;
        state.justReleased = false;
        }

    for (auto & [button, state] : m_MouseButtonStates)
        {
        state.justPressed = false;
        state.justReleased = false;
        }
    }

const std::string CInputSystem::GetSystemName () const
    {
    return "InputSystem";
    }

    //=============================================================================
    // Window Management
    //=============================================================================

bool CInputSystem::CacheWindow ()
    {
    CEngine * engine = GetEngine ();
    if (!engine)
        {
        LogError ( "Engine is null" );
        return false;
        }

    m_WindowHandle = engine->GetWindowHandle ();
    if (!m_WindowHandle)
        {
        LogError ( "Failed to get window handle from engine" );
        return false;
        }

    LogDebug ( "Window handle cached successfully" );
    return true;
    }

GLFWwindow * CInputSystem::GetWindow () const
    {
    return m_WindowHandle;
    }

void CInputSystem::SetWindow ( GLFWwindow * window )
    {
    m_WindowHandle = window;
    LogDebug ( "Window set" );
    }

    //=============================================================================
    // State Queries (int version)
    //=============================================================================

bool CInputSystem::IsKeyPressed ( int key ) const
    {
    auto it = m_KeyStates.find ( key );
    return it != m_KeyStates.end () && it->second.current;
    }

bool CInputSystem::IsKeyJustPressed ( int key ) const
    {
    auto it = m_KeyStates.find ( key );
    return it != m_KeyStates.end () && it->second.justPressed;
    }

bool CInputSystem::IsKeyJustReleased ( int key ) const
    {
    auto it = m_KeyStates.find ( key );
    return it != m_KeyStates.end () && it->second.justReleased;
    }

bool CInputSystem::IsKeyHeld ( int key ) const
    {
    auto it = m_KeyStates.find ( key );
    return it != m_KeyStates.end () && it->second.current && it->second.heldTime > 0.1f;
    }

bool CInputSystem::IsMouseButtonPressed ( int button ) const
    {
    auto it = m_MouseButtonStates.find ( button );
    return it != m_MouseButtonStates.end () && it->second.current;
    }

bool CInputSystem::IsMouseButtonJustPressed ( int button ) const
    {
    auto it = m_MouseButtonStates.find ( button );
    return it != m_MouseButtonStates.end () && it->second.justPressed;
    }

bool CInputSystem::IsMouseButtonJustReleased ( int button ) const
    {
    auto it = m_MouseButtonStates.find ( button );
    return it != m_MouseButtonStates.end () && it->second.justReleased;
    }

bool CInputSystem::IsMouseButtonHeld ( int button ) const
    {
    auto it = m_MouseButtonStates.find ( button );
    return it != m_MouseButtonStates.end () && it->second.current && it->second.heldTime > 0.1f;
    }

FVector2D CInputSystem::GetMousePosition () const
    {
    return m_MousePosition;
    }

FVector2D CInputSystem::GetMouseDelta () const
    {
    return m_MouseDelta;
    }

FVector2D CInputSystem::GetScrollDelta () const
    {
    return m_ScrollDelta;
    }

void CInputSystem::SetMouseCursorVisible ( bool visible )
    {
    if (m_WindowHandle)
        {
        glfwSetInputMode ( m_WindowHandle, GLFW_CURSOR,
                           visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED );
        }
    }

void CInputSystem::SetMousePosition ( const FVector2D & position )
    {
    if (m_WindowHandle)
        {
        glfwSetCursorPos ( m_WindowHandle, position.x, position.y );
        }
    }

    //=============================================================================
    // Binding Methods (int version - core)
    //=============================================================================

void CInputSystem::BindAction ( const std::string & actionName, int button, EInputEvent eventType,
                                InputActionDelegate delegate, CInputComponent * component )
    {
    ActionBinding binding;
    binding.button = button;
    binding.eventType = eventType;
    binding.delegate = delegate;
    binding.owner = component;
    m_ActionBindings[ actionName ] = binding;

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

    LogTrace ( "Bound action: ", actionName, " (", eventStr, ") to ", typeStr, ": ", button );
    }

void CInputSystem::BindAxis ( const std::string & axisName, int positiveKey, int negativeKey,
                              InputAxisDelegate delegate, CInputComponent * component )
    {
    AxisBinding binding;
    binding.positiveKey = positiveKey;
    binding.negativeKey = negativeKey;
    binding.bIsMouseAxis = false;
    binding.delegate = delegate;
    binding.owner = component;
    binding.value = 0.0f;
    m_AxisBindings[ axisName ] = binding;

    LogTrace ( "Bound axis: ", axisName, " to keys: ", positiveKey, " / ", negativeKey );
    }

void CInputSystem::BindMouseAxis ( const std::string & axisName, int mouseAxis,
                                   InputAxisDelegate delegate, CInputComponent * component )
    {
    AxisBinding binding;
    binding.positiveKey = -1;
    binding.negativeKey = -1;
    binding.mouseAxis = mouseAxis;
    binding.bIsMouseAxis = true;
    binding.delegate = delegate;
    binding.owner = component;
    binding.value = 0.0f;
    m_AxisBindings[ axisName ] = binding;

    const char * axisStr = "";
    if (mouseAxis == ToInt ( EMouseAxis::MouseX ))
        axisStr = "Mouse X";
    else if (mouseAxis == ToInt ( EMouseAxis::MouseY ))
        axisStr = "Mouse Y";
    else if (mouseAxis == ToInt ( EMouseAxis::MouseScroll ))
        axisStr = "Mouse Scroll";
    else
        axisStr = "Unknown";

    LogTrace ( "Bound mouse axis: ", axisName, " to ", axisStr );
    }

    //=============================================================================
    // Binding Methods (enum class version - wrappers)
    //=============================================================================

void CInputSystem::BindAction ( const std::string & actionName, EKeys key, EInputEvent eventType,
                                InputActionDelegate delegate, CInputComponent * component )
    {
    BindAction ( actionName, ToInt ( key ), eventType, delegate, component );
    }

void CInputSystem::BindAction ( const std::string & actionName, EMouseButtons button, EInputEvent eventType,
                                InputActionDelegate delegate, CInputComponent * component )
    {
    BindAction ( actionName, ToInt ( button ), eventType, delegate, component );
    }

void CInputSystem::BindAxis ( const std::string & axisName, EKeys positiveKey, EKeys negativeKey,
                              InputAxisDelegate delegate, CInputComponent * component )
    {
    BindAxis ( axisName, ToInt ( positiveKey ), ToInt ( negativeKey ), delegate, component );
    }

void CInputSystem::BindMouseAxis ( const std::string & axisName, EMouseAxis mouseAxis,
                                   InputAxisDelegate delegate, CInputComponent * component )
    {
    BindMouseAxis ( axisName, ToInt ( mouseAxis ), delegate, component );
    }

    //=============================================================================
    // Unbinding Methods
    //=============================================================================

void CInputSystem::UnbindAction ( const std::string & actionName, CInputComponent * component )
    {
    auto it = m_ActionBindings.find ( actionName );
    if (it != m_ActionBindings.end () && ( !component || it->second.owner == component ))
        {
        m_ActionBindings.erase ( it );
        LogTrace ( "Unbound action: ", actionName );
        }
    }

void CInputSystem::UnbindAxis ( const std::string & axisName, CInputComponent * component )
    {
    auto it = m_AxisBindings.find ( axisName );
    if (it != m_AxisBindings.end () && ( !component || it->second.owner == component ))
        {
        m_AxisBindings.erase ( it );
        LogTrace ( "Unbound axis: ", axisName );
        }
    }

void CInputSystem::UnbindAllForComponent ( CInputComponent * component )
    {
    if (!component) return;

    for (auto it = m_ActionBindings.begin (); it != m_ActionBindings.end ();)
        {
        if (it->second.owner == component)
            it = m_ActionBindings.erase ( it );
        else
            ++it;
        }

    for (auto it = m_AxisBindings.begin (); it != m_AxisBindings.end ();)
        {
        if (it->second.owner == component)
            it = m_AxisBindings.erase ( it );
        else
            ++it;
        }

    LogTrace ( "Unbound all for component: ", component->GetName () );
    }

    //=============================================================================
    // Component Registration
    //=============================================================================

void CInputSystem::RegisterInputComponent ( CInputComponent * Component )
    {
    if (Component && std::find ( m_InputComponents.begin (), m_InputComponents.end (), Component ) == m_InputComponents.end ())
        {
        m_InputComponents.push_back ( Component );
        LogTrace ( "Registered input component: ", Component->GetName () );
        }
    }

void CInputSystem::UnregisterInputComponent ( CInputComponent * Component )
    {
    auto it = std::find ( m_InputComponents.begin (), m_InputComponents.end (), Component );
    if (it != m_InputComponents.end ())
        {
        m_InputComponents.erase ( it );
        UnbindAllForComponent ( Component );
        LogTrace ( "Unregistered input component: ", Component->GetName () );
        }
    }

    //=============================================================================
    // Processing Methods
    //=============================================================================

void CInputSystem::ProcessControllerInput ( CController * Controller, float DeltaTime )
    {
    if (!Controller || !Controller->IsInputEnabled ()) return;

    CPawn * ControlledPawn = Controller->GetPawn ();
    if (!ControlledPawn || !ControlledPawn->IsInputEnabled ()) return;

    CInputComponent * InputComp = ControlledPawn->GetInputComponent ();
    if (InputComp == nullptr) return;

    InputComp->Tick ( DeltaTime );
    }

void CInputSystem::UpdateMouseState ()
    {
    for (auto & [button, state] : m_MouseButtonStates)
        {
        state.previous = state.current;
        state.current = glfwGetMouseButton ( m_WindowHandle, button ) == GLFW_PRESS;
        }

    m_MouseDelta = m_MousePosition - m_LastMousePosition;
    m_LastMousePosition = m_MousePosition;
    }

void CInputSystem::ProcessActions ( float DeltaTime )
    {
    for (const auto & [actionName, binding] : m_ActionBindings)
        {
        bool shouldTrigger = false;

        bool isMouseButton = ( binding.button >= GLFW_MOUSE_BUTTON_1 &&
                               binding.button <= GLFW_MOUSE_BUTTON_LAST );

        switch (binding.eventType)
            {
                case EInputEvent::IE_Pressed:
                    shouldTrigger = isMouseButton ?
                        IsMouseButtonJustPressed ( binding.button ) :
                        IsKeyJustPressed ( binding.button );
                    break;

                case EInputEvent::IE_Released:
                    shouldTrigger = isMouseButton ?
                        IsMouseButtonJustReleased ( binding.button ) :
                        IsKeyJustReleased ( binding.button );
                    break;

                case EInputEvent::IE_Repeat:
                    shouldTrigger = isMouseButton ?
                        IsMouseButtonHeld ( binding.button ) :
                        IsKeyHeld ( binding.button );
                    break;

                case EInputEvent::IE_DoubleClick:
                    // TODO: Implement double click detection
                    break;
            }

        if (shouldTrigger && binding.delegate)
            {
            binding.delegate ();
            }
        }
    }

void CInputSystem::ProcessAxes ( float DeltaTime )
    {
    for (auto & [axisName, binding] : m_AxisBindings)
        {
        float value = 0.0f;

        if (binding.bIsMouseAxis)
            {
            if (binding.mouseAxis == ToInt ( EMouseAxis::MouseX ))
                {
                value = m_MouseDelta.x * m_MouseSensitivity;
                }
            else if (binding.mouseAxis == ToInt ( EMouseAxis::MouseY ))
                {
                value = m_MouseDelta.y * m_MouseSensitivity;
                }
            else if (binding.mouseAxis == ToInt ( EMouseAxis::MouseScroll ))
                {
                value = m_ScrollDelta.y;
                }
            }
        else
            {
            if (binding.positiveKey != -1 && IsKeyPressed ( binding.positiveKey ))
                value += 1.0f;
            if (binding.negativeKey != -1 && IsKeyPressed ( binding.negativeKey ))
                value -= 1.0f;
            }

        float previousValue = binding.value;
        binding.value = value;

        if (value != previousValue || value != 0.0f)
            {
            binding.delegate ( value );
            }
        }
    }

    //=============================================================================
    // Callback Handlers
    //=============================================================================

void CInputSystem::HandleKey ( int key, int scancode, int action, int mods )
    {
    if (action == GLFW_PRESS)
        {
        m_KeyStates[ key ].previous = m_KeyStates[ key ].current;
        m_KeyStates[ key ].current = true;
        m_KeyStates[ key ].justPressed = true;
        m_KeyStates[ key ].justReleased = false;
        m_KeyStates[ key ].heldTime = 0.0f;
        }
    else if (action == GLFW_RELEASE)
        {
        m_KeyStates[ key ].previous = m_KeyStates[ key ].current;
        m_KeyStates[ key ].current = false;
        m_KeyStates[ key ].justPressed = false;
        m_KeyStates[ key ].justReleased = true;
        }
    }

void CInputSystem::HandleMouseButton ( int button, int action, int mods )
    {
    if (action == GLFW_PRESS)
        {
        m_MouseButtonStates[ button ].previous = m_MouseButtonStates[ button ].current;
        m_MouseButtonStates[ button ].current = true;
        m_MouseButtonStates[ button ].justPressed = true;
        m_MouseButtonStates[ button ].justReleased = false;
        m_MouseButtonStates[ button ].heldTime = 0.0f;
        }
    else if (action == GLFW_RELEASE)
        {
        m_MouseButtonStates[ button ].previous = m_MouseButtonStates[ button ].current;
        m_MouseButtonStates[ button ].current = false;
        m_MouseButtonStates[ button ].justPressed = false;
        m_MouseButtonStates[ button ].justReleased = true;
        }
    }

void CInputSystem::HandleMouseMove ( double xpos, double ypos )
    {
    m_MousePosition = FVector2D ( static_cast< float >( xpos ), static_cast< float >( ypos ) );
    }

void CInputSystem::HandleScroll ( double xoffset, double yoffset )
    {
    m_ScrollDelta = FVector2D ( static_cast< float >( xoffset ), static_cast< float >( yoffset ) );
    }