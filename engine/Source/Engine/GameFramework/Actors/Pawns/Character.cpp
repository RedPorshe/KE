#include "Actors/Pawns/Character.h"
#include "Actors/TerrainActor.h"
#include "Components/Meshes/StaticMeshComponent.h"
#include "Components/Meshes/TerrainMeshComponent.h"
#include "Components/Collisions/CapsuleComponent.h"
#include "Components/Collisions/BoxComponent.h"
#include "Components/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/GravityComponent.h"

#include "World/World.h"
#include "World/Level.h"

CCharacter::CCharacter ( CObject * inOwner, const std::string & DisplayName )
	: Super ( inOwner, DisplayName )
	{
	CreateCharacterMovementComponent ();
	SetMovableState ( EMovableState::DYNAMIC );

	Capsule = AddDefaultSubObject<CCapsuleComponent> ( "Capsule" );
	Capsule->SetRadius ( 4.f );
	Capsule->SetHalfHeight ( 9.f );
	Capsule->AttachTo ( RootComponent );
	Mesh = AddDefaultSubObject<CStaticMeshComponent/*CSkeletalMeshComponent*/> ( "Mesh" ); // TODO change to CSkeletalMeshComponent
	Mesh->AttachTo ( Capsule );
	}

void CCharacter::CreateCharacterMovementComponent ()
	{
	if (MovementComponent)
		{
		RemoveOwnedObject ( MovementComponent->GetName () );
		MovementComponent = nullptr;
		}

	MovementComponent = AddDefaultSubObject<CCharacterMovementComponent> ( "CharMov" );

	if (MovementComponent)
		{
		MovementComponent->SetOwnerPawn ( this );
		LOG_DEBUG ( "[CHARACTER] Movement component created: ", MovementComponent->GetName () );
		}
	else
		{
		LOG_ERROR ( "[CHARACTER] Failed to create movement component" );
		}
	if (CCharacterMovementComponent * CharMov = dynamic_cast< CCharacterMovementComponent * >( MovementComponent ))
		{
		CharMov->SetUseControllRotationYaw ( true );   // Разрешаем поворот по горизонтали
		CharMov->SetUseControllRotationPitch ( true ); // Разрешаем поворот по вертикали
		CharMov->SetUseControllRotationRoll ( false ); // Запрещаем крен
		}
	}

void CCharacter::BeginPlay ()
	{
	Super::BeginPlay ();
	for (auto & actor : GetWorld ()->GetCurrentLevel ()->GetActors ())
		{
		for (auto & comp : actor->GetActorComponents ())
			{
			if (CTerrainMeshComponent * terMesh = dynamic_cast< CTerrainMeshComponent * >( comp ))
				{
				terrainMesh = terMesh;
				break;
				}
			}
		}
	}

void CCharacter::Tick ( float DeltaTime )
	{
	Super::Tick ( DeltaTime );
	DebugInfo ( DeltaTime );
	}

void CCharacter::EndPlay ()
	{
	Super::EndPlay ();
	}

void CCharacter::SetupPlayerInputComponent ( CInputComponent * InputComponent )
	{
	Super::SetupPlayerInputComponent ( InputComponent );
	}


void CCharacter::StartJump ()
	{
	if (auto * CharMov = dynamic_cast< CCharacterMovementComponent * >( MovementComponent ))
		{
		CharMov->Jump ();
		}
	}

bool CCharacter::IsJumping () const
	{
	if (auto * CharMov = dynamic_cast< CCharacterMovementComponent * >( MovementComponent ))
		{
		return CharMov->IsJumping ();
		}
	return false;
	}

void CCharacter::OnComponentBeginOverlap ( CBaseCollisionComponent * other )
	{
	Super::OnComponentBeginOverlap ( other );
	auto otherActor = other->GetOwnerActor ();
	LOG_DEBUG ( "[CHARACTER] Overlaped with ", otherActor->GetName () );
	}

void CCharacter::OnComponentEndOverlap ( CBaseCollisionComponent * other )
	{
	Super::OnComponentEndOverlap ( other );
	auto otherActor = other->GetOwnerActor ();
	LOG_DEBUG ( "[CHARACTER] End Overlap with ", otherActor->GetName () );
	}

void CCharacter::OnComponentHit ( CBaseCollisionComponent * other )
	{
	auto otherActor = other->GetOwnerActor ();
	if (CTerrainActor * terra = dynamic_cast< CTerrainActor * >( otherActor )) return;


	}
 
void CCharacter::DebugInfo ( float dt )
	{

	}