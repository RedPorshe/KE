#include "KE/GameFramework/Actors/Controllers/PlayerController.h"
#include "KE/Engine.h"
#include "KE/Systems/InputSystem.h"

CPlayerController::CPlayerController ( CObject * inOwner, const std::string & inDisplayName )
    : Super ( inOwner, inDisplayName )
    {
    LOG_DEBUG ( "[PLAYERCONTROLLER] Created: ", GetName () );
    }

CPlayerController::~CPlayerController ()
    {}

void CPlayerController::ProcessPlayerInput ( float DeltaTime )
    {
    if (!bInputEnabled || !ControlledPawn) return;

    auto * InputSystem = CEngine::Get().GetInputSystem();
    if (InputSystem)
        {
        InputSystem->ProcessControllerInput ( this, DeltaTime );
        }

        // Обновляем позицию мыши если нужно
    if (bShowMouseCursor)
        {
            // Получить позицию мыши из системы ввода
            // MousePosition = InputSystem->GetMousePosition();
        }
    }

void CPlayerController::BeginPlay ()
    {
    Super::BeginPlay ();
    LOG_DEBUG ( "[PLAYERCONTROLLER] BeginPlay: ", GetName () );
    }

void CPlayerController::EndPlay ()
    {
    Super::EndPlay ();
    LOG_DEBUG ( "[PLAYERCONTROLLER] EndPlay: ", GetName () );
    }