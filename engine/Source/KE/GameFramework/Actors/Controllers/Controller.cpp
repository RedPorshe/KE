#include "KE/GameFramework/Actors/Controllers/Controller.h"
#include "KE/GameFramework/Actors/Pawns/Pawn.h"
#include "KE/GameFramework/Actors/HUD.h"
#include "KE/GameFramework/GameMode.h"
#include "KE/GameFramework/Components/TransformComponent.h"
#include "KE/Systems/InputSystem.h"
#include "KE/GameFramework/Components/Collisions/BaseCollisionComponent.h"
#include "KE/GameFramework/Components/GravityComponent.h"

CController::CController ( CObject * inOwner, const std::string & inDisplayName )
    : Super ( inOwner, inDisplayName )
    {
    LOG_DEBUG ( "[CONTROLLER] Created: ", GetName () );

    // Контроллеры не имеют физического тела - удаляем гравитацию
    if (m_Gravity)
        {
        DestroyGravity ();
        RemoveOwnedObject ( m_Gravity->GetName () );
        m_Gravity = nullptr;
        }
    }

CController::~CController ()
    {
    if (ControlledPawn)
        {
        Unpossess ();
        }
    m_HUD = nullptr;
    OwningGameMode = nullptr;
    ViewTarget = nullptr;
    }

void CController::Possess ( CPawn * PawnToPossess )
    {
    if (!PawnToPossess)
        {
        LOG_ERROR ( "[CONTROLLER] Cannot possess null pawn!" );
        return;
        }

    if (ControlledPawn == PawnToPossess)
        {
        LOG_WARN ( "[CONTROLLER] Already possessing pawn: ", PawnToPossess->GetName () );
        return;
        }

    if (ControlledPawn)
        {
        Unpossess ();
        }

    ControlledPawn = PawnToPossess;
    ControlledPawn->SetController ( this );
    ControlledPawn->OnPossessed ( this );
    ControlledPawn->SetInputEnabled ( true );
    ViewTarget = ControlledPawn;
    ControlledPawn->OnPossess ();

    LOG_DEBUG ( "[CONTROLLER] Possessed pawn: ", ControlledPawn->GetName (),
                " for controller: ", GetName () );
    }

void CController::Unpossess ()
    {
    if (!ControlledPawn) return;

    LOG_DEBUG ( "[CONTROLLER] Unpossessing pawn: ", ControlledPawn->GetName (),
                " from controller: ", GetName () );

    ControlledPawn->SetInputEnabled ( false );
    ControlledPawn->SetController ( nullptr );
    ControlledPawn = nullptr;
    ViewTarget = nullptr;
    }

CHUD * CController::SpawnHUD ( CHUD * InHUD )
    {
    if (!InHUD)
        {
        LOG_ERROR ( "[CONTROLLER] Cannot spawn null HUD!" );
        return nullptr;
        }

    if (m_HUD)
        {
        LOG_WARN ( "[CONTROLLER] Controller already has HUD: ", m_HUD->GetName (),
                   " - replacing with: ", InHUD->GetName () );
        }

    SetHUD ( InHUD );

    if (m_HUD)
        {
        m_HUD->SetOwnerController ( this );
        LOG_DEBUG ( "[CONTROLLER] Spawned HUD: ", m_HUD->GetName () );
        }

    return m_HUD;
    }

void CController::SetHUD ( CHUD * inHUD )
    {
    if (inHUD == nullptr)
        {
        LOG_ERROR ( "[CONTROLLER] Cannot set null HUD!" );
        return;
        }

    m_HUD = inHUD;
    m_HUD->SetOwnerController ( this );
    }

void CController::SetOwningGameMode ( CGameMode * inGameMode )
    {
    OwningGameMode = inGameMode;
    }

FVector CController::GetViewLocation () const
    {
    if (ViewTarget)
        {
        return ViewTarget->GetActorLocation ();
        }
    else if (ControlledPawn)
        {
        return ControlledPawn->GetActorLocation ();
        }

    return const_cast< CController * >( this )->GetActorLocation ();
    }

FQuat CController::GetViewRotation () const
    {
    if (ViewTarget)
        {
        return ViewTarget->GetActorRotationQuat ();
        }
    else if (ControlledPawn)
        {
        return ControlledPawn->GetActorRotationQuat ();
        }

    return const_cast< CController * >( this )->GetActorRotationQuat ();
    }

void CController::Tick ( float DeltaTime )
    {
    Super::Tick ( DeltaTime );
    // ProcessPlayerInput вызывается в наследниках
    }

void CController::BeginPlay ()
    {
    Super::BeginPlay ();
    LOG_DEBUG ( "[CONTROLLER] BeginPlay: ", GetName () );
    }

void CController::EndPlay ()
    {
    Super::EndPlay ();
    LOG_DEBUG ( "[CONTROLLER] EndPlay: ", GetName () );

    if (ControlledPawn)
        {
        Unpossess ();
        }
    }