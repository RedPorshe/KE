#include "KE/GameFramework/Components/MovementComponent.h"
#include "KE/GameFramework/Actors/Pawns/Pawn.h"
#include "KE/GameFramework/Components/GravityComponent.h"
#include "Utils/Math/CE_MathHelpers.h"

CMovementComponent::CMovementComponent ( CObject * inOwner, const std::string & inName )
    : Super ( inOwner, inName )
    , MovementInputAccumulator ( FVector::Zero () )
    , YawInputAccumulator ( 0.0f )
    , PitchInputAccumulator ( 0.0f )
    , RollInputAccumulator ( 0.0f )
    , bHasMovementInput ( false )
    , bHasRotationInput ( false )
    {}

void CMovementComponent::InitComponent ()
    {
    Super::InitComponent ();
    }

void CMovementComponent::Tick ( float DeltaTime )
    {
    Super::Tick ( DeltaTime );

    if (!OwnerPawn) return;

    // Обновляем состояние на земле из GravityComponent
    if (auto * Gravity = OwnerPawn->GetGravityComponent ())
        {
        bIsGrounded = Gravity->IsGrounded ();
        }

    ProcessMovementInput ( DeltaTime );
    ProcessRotationInput ( DeltaTime );
    }

void CMovementComponent::OnBeginPlay ()
    {
    Super::OnBeginPlay ();
    }

void CMovementComponent::AddInputVector ( const FVector & WorldDirection, float ScaleValue, bool bForce )
    {
    FVector Input = WorldDirection * ScaleValue;

    if (bForce)
        {
        MovementInputAccumulator = Input;
        }
    else
        {
        MovementInputAccumulator += Input;
        }

    bHasMovementInput = true;
    }

void CMovementComponent::AddPitchInput ( float Value )
    {
    if (CEMath::IsZero ( Value )) return;
    PitchInputAccumulator += Value;
    bHasRotationInput = true;
    }

void CMovementComponent::AddYawInput ( float Value )
    {
    if (CEMath::IsZero ( Value )) return;
    YawInputAccumulator += Value;
    bHasRotationInput = true;
    }

void CMovementComponent::AddRollInput ( float Value )
    {
    if (CEMath::IsZero ( Value )) return;
    RollInputAccumulator += Value;
    bHasRotationInput = true;
    }
 

void CMovementComponent::ProcessMovementInput ( float DeltaTime )
    {
    if (!bHasMovementInput || !OwnerPawn) return;

    FVector InputDirection = MovementInputAccumulator;
    float InputMagnitude = InputDirection.Length ();

    if (InputMagnitude > 1.0f)
        {
        InputDirection.Normalize ();
        InputMagnitude = 1.0f;
        }

    float CurrentMaxSpeed = bIsGrounded ? MaxWalkSpeed : MaxAirSpeed;
    float CurrentControl = bIsGrounded ? 1.0f : AirControl;

    FVector DesiredVelocity = InputDirection * CurrentMaxSpeed * InputMagnitude * CurrentControl;
    FVector CurrentVelocity = OwnerPawn->GetVelocity ();

    // Интерполируем только горизонтальную составляющую
    FVector HorizontalVelocity = CurrentVelocity;
    HorizontalVelocity.y = 0.0f;

    FVector DesiredHorizontal = DesiredVelocity;
    DesiredHorizontal.y = 0.0f;

    float Rate;
    if (InputMagnitude > 0.1f)  // Есть ввод - разгоняемся
        {
        Rate = bIsGrounded ? AccelerationRate : AccelerationRate * 0.5f;
        }
    else  // Нет ввода - тормозим
        {
        Rate = bIsGrounded ? DecelerationRate : DecelerationRate * 0.5f;
        }
    float Factor = 1.0f - std::exp ( -Rate * DeltaTime );

    FVector NewHorizontal = HorizontalVelocity + ( DesiredHorizontal - HorizontalVelocity ) * Factor;

    // Сохраняем вертикальную скорость
    NewHorizontal.y = CurrentVelocity.y;
    OwnerPawn->SetVelocity ( NewHorizontal );

    MovementInputAccumulator = FVector::Zero ();
    bHasMovementInput = false;
    }


void CMovementComponent::ProcessRotationInput ( float DeltaTime )
    {
    if (!bHasRotationInput || !OwnerPawn) return;


    if (YawInputAccumulator != 0.0f)
        {
        FQuat YawRotation ( FVector::Up (), YawInputAccumulator );
        OwnerPawn->RotateActor ( YawRotation, true );
        }

    if (PitchInputAccumulator != 0.0f)
        {
        FQuat PitchRotation ( FVector::Right (), PitchInputAccumulator );
        OwnerPawn->RotateActor ( PitchRotation, true );
        }

    if (RollInputAccumulator != 0.0f)
        {
        FQuat RollRotation ( FVector::Forward (), RollInputAccumulator );
        OwnerPawn->RotateActor ( RollRotation, true );
        }

    YawInputAccumulator = 0.0f;
    PitchInputAccumulator = 0.0f;
    RollInputAccumulator = 0.0f;
    bHasRotationInput = false;
    }