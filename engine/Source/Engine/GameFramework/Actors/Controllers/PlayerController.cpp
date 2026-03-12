#include "Actors/Controllers/PlayerController.h"
#include "Core/InputSystem.h"

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

    auto * InputSystem = CInputSystem::GetInstance ();
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