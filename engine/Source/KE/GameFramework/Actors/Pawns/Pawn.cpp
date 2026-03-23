#include "KE/GameFramework/Actors/Pawns/Pawn.h"
#include "KE/GameFramework/Actors/Controllers/PlayerController.h"
#include "KE/GameFramework/Components/TransformComponent.h"
#include "KE/GameFramework/Components/GravityComponent.h"
#include "KE/GameFramework/Components/MovementComponent.h"
#include "KE/GameFramework/Components/InputComponent.h"
#include "KE/GameFramework/Camera/CameraComponent.h"
#include "KE/GameFramework/World/World.h"

CPawn::CPawn ( CObject * inOwner, const std::string & inDisplayName )
	: Super ( inOwner, inDisplayName )
	{
	m_InputComponent = AddDefaultSubObject<CInputComponent> ( "InputComponent_" + GetName () );

	SetMovableState ( EMovableState::DYNAMIC );
	}

CPawn::~CPawn ()
	{
	if (Controller)
		{
		Controller->Unpossess ();
		Controller = nullptr;
		}
	}

void CPawn::SetController ( CController * NewController )
	{
	Controller = NewController;
	}

void CPawn::AddMovementInput ( const FVector & WorldDirection, float ScaleValue, bool bForce )
	{
	if (MovementComponent && IsInputEnabled ())
		{
		MovementComponent->AddInputVector ( WorldDirection, ScaleValue, bForce );
		}
	}

void CPawn::AddControllerYawInput ( float Val )
	{
	if (CEMath::IsZero ( Val )) return;

	if (bUseControllerRotaionYaw)
		{
		if (MovementComponent && IsInputEnabled ())
			{
			MovementComponent->AddYawInput ( Val );
			}
		}
	else
		{
		LOG_WARN ( "bUseControllerRotaionYaw ", bUseControllerRotaionYaw ? " true" : "false" );
		}
	}

void CPawn::AddControllerPitchInput ( float Val )
	{
	if (bUseControllerRotaionPitch)
		{
		if (MovementComponent && IsInputEnabled ())
			{
			float value = Val;
			value *= GetInputComponent ()->GetMouseSensevity ();
			MovementComponent->AddPitchInput ( value );
			}
		}
	else
		{
		float value = Val;
		value *= GetInputComponent ()->GetMouseSensevity ();
		FQuat rot ( FVector::Right (), CEMath::DegreesToRadians ( value ) );
		CCameraComponent * Cameraa = nullptr;

		// CSpringArmComponent * SpringArm = nullptr;

		bool FoundCamera = false;
		bool FoundSpringArm = false;
		for (auto comp : ActorComponents)
			{
			//if (CSpringArmComponent * spr = dynamic_cast< CSpringArmComponent * >( comp ))
			//	{
			//	FoundSpringArm = spr;
			//	FoundSpringArm = true;
			//	}
			if (CCameraComponent * cambj = dynamic_cast< CCameraComponent * >( comp ))
				{
				Cameraa = cambj;
				FoundCamera = true;				
				}
			if (FoundCamera || FoundSpringArm)
				{
				if (FoundCamera && !FoundSpringArm)
					{
					Cameraa->AddLocalRotation ( rot );
					}
				if (FoundSpringArm)
					{
				//	SpringArm->AddLocalRotation ( rot );
					}
				}
			}	
		}
	}

void CPawn::AddControllerRollInput ( float Val )
	{
	if (MovementComponent && IsInputEnabled ())
		{
		MovementComponent->AddRollInput ( Val );
		}
	}

bool CPawn::IsInputEnabled () const
	{
	return bInputEnabled && Controller && Controller->GetPawn () == this;
	}

void CPawn::ProcessPlayerInput ( float DeltaTime )
	{
	if (!Controller || !bInputEnabled) return;
	Controller->ProcessPlayerInput ( DeltaTime );
	}

void CPawn::Tick ( float DeltaTime )
	{
	Super::Tick ( DeltaTime );

	if (Controller)
		{
		ProcessPlayerInput ( DeltaTime );
		}
	}

void CPawn::BeginPlay ()
	{
	Super::BeginPlay ();
	LOG_DEBUG ( "[PAWN] BeginPlay: ", GetName () );
	if(MovementComponent)
	{
	bUseControllerRotaionPitch = MovementComponent->GetUseContollRotaionPitch ();
	bUseControllerRotaionRoll = MovementComponent->GetUseContollRotaionRoll ();
	bUseControllerRotaionYaw = MovementComponent->GetUseContollRotaionYaw ();
	}

	}

void CPawn::EndPlay ()
	{
	Super::EndPlay ();
	LOG_DEBUG ( "[PAWN] EndPlay: ", GetName () );

	if (m_InputComponent)
		{
		m_InputComponent->OnEndPlay ();
		}
	}

void CPawn::OnPossessed ( CController * NewController )
	{
	if (!NewController) return;
	SetController ( NewController );
	}

void CPawn::OnUnpossessed ( CController * OldController )
	{
	if (OldController && Controller == OldController)
		{
		SetController ( nullptr );
		}
	}

void CPawn::OnPossess ()
	{
	if (m_InputComponent)
		{
		SetupPlayerInputComponent ( m_InputComponent );
		}
	}

void CPawn::SetupPlayerInputComponent ( CInputComponent * InputComponent )
	{
	LOG_DEBUG ( "[PAWN] SetupPlayerInputComponent for: ", GetName () );
	m_InputComponent = InputComponent;
	}

void CPawn::SetAirControl ( float value )
	{
	if (MovementComponent)
		{
		MovementComponent->SetAirControl ( value );
		}
	}

float CPawn::GetAirControl () const
	{
	return MovementComponent ? MovementComponent->GetAirControl () : 0.0f;
	}

void CPawn::SetMaxAirSpeed ( float value )
	{
	if (MovementComponent)
		{
		MovementComponent->SetMaxAirSpeed ( value );
		}
	}

float CPawn::GetMaxAirSpeed () const
	{
	return MovementComponent ? MovementComponent->GetMaxAirSpeed () : 0.0f;
	}

void CPawn::SetGroundSpeed ( float value )
	{
	if (MovementComponent)
		{
		MovementComponent->SetMaxWalkSpeed ( value );
		}
	}

float CPawn::GetGroundSpeed () const
	{
	return MovementComponent ? MovementComponent->GetMaxWalkSpeed () : 0.0f;
	}