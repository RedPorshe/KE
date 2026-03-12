#include "Actors/Controllers/AIController.h"
#include "Actors/Pawns/Pawn.h"
#include "Components/MovementComponent.h"

CAIController::CAIController ( CObject * inOwner, const std::string & inDisplayName )
    : Super ( inOwner, inDisplayName )
    {
    LOG_DEBUG ( "[AICONTROLLER] Created: ", GetName () );
    }

CAIController::~CAIController ()
    {}

void CAIController::ProcessPlayerInput ( float DeltaTime )
    {
        // AI не обрабатывает пользовательский ввод
        // Вся логика в Tick
    }

void CAIController::Tick ( float DeltaTime )
    {
    Super::Tick ( DeltaTime );

    if (!ControlledPawn) return;

    // Базовое AI принятие решений
    DecisionTimer += DeltaTime;
    if (DecisionTimer >= DecisionInterval)
        {
        DecisionTimer = 0.0f;
        MakeDecision ();
        }

        // Выполнение текущего поведения
    ExecuteBehavior ( DeltaTime );
    }

void CAIController::BeginPlay ()
    {
    Super::BeginPlay ();
    LOG_DEBUG ( "[AICONTROLLER] BeginPlay: ", GetName () );
    }

void CAIController::EndPlay ()
    {
    Super::EndPlay ();
    LOG_DEBUG ( "[AICONTROLLER] EndPlay: ", GetName () );
    }


void CAIController::MakeDecision ()
    {
        // Базовая логика принятия решений
        // Будет переопределяться в наследниках
    }

void CAIController::ExecuteBehavior ( float DeltaTime )
    {
        // Базовая логика выполнения поведения
        // Будет переопределяться в наследниках

    switch (BehaviorState)
        {
            case 0: // Idle
                // Стоим на месте
                ControlledPawn->SetVelocity ( FVector::Zero () );
                break;

            case 1: // Patrolling
                // Патрулирование - будет реализовано в наследниках
                break;

            case 2: // Chasing
                // Преследование цели
                if (TargetActor)
                    {
                        // Базовая логика движения к цели
                    FVector Direction = TargetActor->GetActorLocation () - ControlledPawn->GetActorLocation ();
                    Direction.Normalize ();

                    if (auto * Movement = ControlledPawn->GetMovementComponent ())
                        {
                        Movement->AddInputVector ( Direction, 1.0f, false );
                        }
                    }
                break;

            case 3: // Attacking
                // Атака - будет реализовано в наследниках
                break;

            default:
                break;
        }
    }