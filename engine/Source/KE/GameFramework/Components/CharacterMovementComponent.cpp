#include "KE/GameFramework/Components/CharacterMovementComponent.h"
#include "KE/GameFramework/Components/GravityComponent.h"
#include "KE/GameFramework/Actors/Pawns/Character.h"
#include "KE/GameFramework/Camera/CameraComponent.h"
#include "KE/GameFramework/Components/TransformComponent.h"
#include "KE/GameFramework/Components/Collisions/CapsuleComponent.h"

CCharacterMovementComponent::CCharacterMovementComponent ( CObject * inOwner, const std::string & Name )
	: Super ( inOwner, Name )
	{}

void CCharacterMovementComponent::InitComponent ()
	{
	Super::InitComponent ();
	}

void CCharacterMovementComponent::Tick ( float DeltaTime )
	{
	Super::Tick ( DeltaTime );

	if (bIsGrounded)
		{
		CurrentJumpCount = 0;
		bIsJumping = false;
		}
	}

void CCharacterMovementComponent::OnBeginPlay ()
	{
	Super::OnBeginPlay ();
	}

bool CCharacterMovementComponent::CanJump () const
	{
	if (!OwnerPawn || !OwnerPawn->IsInputEnabled ())
		return false;

	return bIsGrounded || CurrentJumpCount < MaxJumpCount;
	}

void CCharacterMovementComponent::ProcessMovementInput ( float DeltaTime )
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
	NewHorizontal.y = CurrentVelocity.y;
	OwnerPawn->SetVelocity ( NewHorizontal );
	MovementInputAccumulator = FVector::Zero ();
	bHasMovementInput = false;

	}

void CCharacterMovementComponent::ProcessRotationInput ( float DeltaTime )
	{
	if (!bHasRotationInput) return;
	if (!CEMath::IsZero ( YawInputAccumulator ))
		{
		FQuat rot ( FVector::Up (), CEMath::DegreesToRadians ( YawInputAccumulator ) );
		OwnerPawn->RotateActor ( rot );
		YawInputAccumulator = 0.0f;
		}
	if (!CEMath::IsZero ( RollInputAccumulator ))
		{
		FQuat rot ( FVector::Right (), CEMath::DegreesToRadians ( RollInputAccumulator ) );
		OwnerPawn->RotateActor ( rot );
		RollInputAccumulator = 0.0f;
		}
	if (!CEMath::IsZero ( PitchInputAccumulator ))
		{
		FQuat rot ( FVector::Right (), CEMath::DegreesToRadians ( PitchInputAccumulator ) );
		OwnerPawn->RotateActor ( rot );
		PitchInputAccumulator = 0.0f;
		}
	
	bHasRotationInput = false;
	}

void CCharacterMovementComponent::Jump ()
	{
	if (!CanJump ()) return;

	if (auto * Gravity = OwnerPawn->GetGravityComponent ())
		{
		float neededVelocity = Gravity->GetJumpVelocity ( JumpHeight );

		if (!bIsGrounded)
			{
			neededVelocity *= AirJumpMultiplier;
			}

		Gravity->SetVerticalVelocity ( neededVelocity );
		CurrentJumpCount++;
		bIsJumping = true;
		}
	}

void CCharacterMovementComponent::StopJumping ()
	{
	bIsJumping = false;
	}