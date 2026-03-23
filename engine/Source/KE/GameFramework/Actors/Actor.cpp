#include "KE/GameFramework/Actors/Actor.h"
#include "KE/GameFramework/World/Level.h"
#include "KE/GameFramework/World/World.h"
#include "KE/GameFramework/GameInstance.h"
#include "KE/Vulkan/RenderInfo.h"
#include "KE/GameFramework/Components/Collisions/TerrainComponent.h"
#include "KE/GameFramework/Components/BaseComponent.h"
#include "KE/GameFramework/Components/Meshes/BaseMeshComponent.h"
#include "KE/GameFramework/Components/Meshes/TerrainMeshComponent.h"
#include "KE/GameFramework/Components/SceneComponent.h"
#include "KE/GameFramework/Components/Collisions/BaseCollisionComponent.h"
#include "KE/GameFramework/Components/Collisions/BoxComponent.h"
#include "KE/GameFramework/Components/Collisions/CapsuleComponent.h"
#include "KE/GameFramework/Components/Collisions/ConeComponent.h"
#include "KE/GameFramework/Components/Collisions/CylinderComponent.h"
#include "KE/GameFramework/Components/Collisions/SphereComponent.h"
#include "KE/GameFramework/Components/GravityComponent.h"
#include "KE/Vulkan/Managers/WireframePipeline.h"

CActor::CActor ( CObject * owner, const std::string & inName )
	: CObject ( owner, inName )
	{
	RootComponent = AddDefaultSubObject<CTransformComponent> ( inName + "_Transform" );
	m_Gravity = AddDefaultSubObject<CGravityComponent> ( GetName () + "_Gravity" );

	if (RootComponent)
		{
		RootComponent->SetCollisionEnabled ( bIsCollisionEnabled );
		}
	}

CActor::~CActor ()
	{
	ActorComponents.clear ();
	}

void CActor::BeginPlay ()
	{
	if (ActorStartedBeginPlay ()) return;
	

	LOG_DEBUG ( "[ACTOR] BeginPlay: ", GetName () );
	if (GetActorLocation ().IsZero ())
		{
		if (GetRootComponent () && GetRootComponent ()->IdDirty ())
			{
			LOG_WARN ( "[", GetName (), "] Transform on begin play Dirty updating transform" );
			GetRootComponent ()->UpdateTransform ();
			LOG_DEBUG ( "[", GetName (), "] Transform updated" );
			}
		}
	for (auto comp : ActorComponents)
		{
		comp->OnBeginPlay ();
		if (CTransformComponent * transform = dynamic_cast< CTransformComponent * >( comp ))
			{
			transform->UpdateTransform ();
			}
		}

	if (GetRootComponent ())
		{
		this->GetRootComponent ()->SetCollisionEnabled ( bIsCollisionEnabled );
		this->GetRootComponent ()->UpdateTransform ();
		}

	bIsStarted = true;
	}

void CActor::Tick ( float deltaTime )
	{
	if (IsPendingToDestroy ()) return;

	if (RootComponent->IdDirty ()) RootComponent->UpdateTransform ();
	UpdatePhysics ( deltaTime );

	
	if (bIsLerpingLocation && RootComponent)
		{
		LocationLerpAlpha += deltaTime * LerpSpeed;

		if (LocationLerpAlpha >= 1.0f)
			{
			LocationLerpAlpha = 1.0f;
			bIsLerpingLocation = false;
			bIsMoving = false;
			RootComponent->SetLocation ( TargetLocation );
			}
		else
			{
			FVector currentLocation = FVector::Lerp (
				LerpStartLocation,
				TargetLocation,
				LocationLerpAlpha
			);
			RootComponent->SetLocation ( currentLocation );
			}
		}

		
	if (bIsLerpingRotation && RootComponent)
		{
		RotationLerpAlpha += deltaTime * LerpSpeed;

		if (RotationLerpAlpha >= 1.0f)
			{
			RotationLerpAlpha = 1.0f;
			bIsLerpingRotation = false;
			RootComponent->SetRotation ( TargetRotation );
			}
		else
			{
			FQuat currentRotation = FQuat::Slerp (
				LerpStartRotation,
				TargetRotation,
				RotationLerpAlpha
			);
			currentRotation.Normalize ();
			RootComponent->SetRotation ( currentRotation );
			}
		}

		// Обновляем компоненты
	for (auto comp : ActorComponents)
		{
		if (comp && comp->GetOwner () == this)
			{
			comp->Tick ( deltaTime );
			}
		}

	DebugInfo ( deltaTime );
	}

void CActor::EndPlay ()
	{
	LOG_DEBUG ( "[ACTOR] EndPlay: ", GetName () );
	for (auto comp : ActorComponents)
		{
		comp->OnEndPlay ();
		}
	}

void CActor::DebugInfo ( float deltaTime )
	{
		// Пусто для переопределения
	}

FRenderCollection CActor::GetRenderInfo () const
	{
	FRenderCollection Collection;

	
	for (auto * comp : ActorComponents)
		{
		if (CBaseMeshComponent * mesh = dynamic_cast< CBaseMeshComponent * >( comp ))
			{
			if (mesh->IsReadyForRender ())
				{
				if (CTerrainMeshComponent * terrain = dynamic_cast< CTerrainMeshComponent * >( mesh ))
					{
					FTerrainRenderInfo terrainInfo = terrain->GetTerrainInfo ();
					if (terrainInfo.IsValid ())
						{
						Collection.Terrains.push_back ( terrainInfo );
						}
					}
				else
					{
					FMeshInfo meshInfo = mesh->GetMeshInfo ();
					if (meshInfo.IsValid ())
						{
						Collection.Meshes.push_back ( meshInfo );
						}
					}
				}
			}
		}

	
	if (m_bDrawCollisions)
		{
		auto ProcessCollisionComponent = [ & ] ( const CBaseCollisionComponent * collision )
			{
			if (!collision || !collision->IsCollisionEnabled ()) return;

			FVector worldLoc = collision->GetWorldLocation ();
			FQuat worldRot = collision->GetRotationQuat ();
			FVector WorldScale = collision->GetScale ();
			FVector debugColor = FVector ( 0.0f, 1.0f, 0.0f );

			if (collision->ShouldBlockWith ( collision ))
				debugColor = FVector ( 1.0f, 0.0f, 0.0f );
			else if (collision->ShouldOverlapWith ( collision ))
				debugColor = FVector ( 0.0f, 0.0f, 1.0f );

			switch (collision->GetShapeType ())
				{
					case ECollisionShape::SPHERE:
						{
						if (auto * sphere = dynamic_cast< const CSphereComponent * >( collision ))
							{
							FCollisionDebugInfo Info {};
							Info.CreateSphere ( worldLoc,
												sphere->GetRadius (),
												debugColor );
							Info.WorldScale = WorldScale;
							Collection.DebugCollisions.push_back ( Info );
							}
						break;
						}
					case ECollisionShape::BOX:
						{
						if (auto * box = dynamic_cast< const CBoxComponent * >( collision ))
							{
							FCollisionDebugInfo Info {};
							Info.WorldScale = WorldScale;
							Info.CreateBox (
								worldLoc,
								worldRot,
								box->GetHalfExtents (),
								debugColor
							);
							Collection.DebugCollisions.push_back ( Info );
							}
						break;
						}
					case ECollisionShape::CAPSULE:
						{
						if (auto * capsule = dynamic_cast< const CCapsuleComponent * >( collision ))
							{
							FCollisionDebugInfo Info {};
							Info.WorldScale = WorldScale;
							Info.CreateCapsule (
								worldLoc,
								worldRot,
								capsule->GetRadius (),
								capsule->GetHalfHeight (),
								debugColor
							);
							Collection.DebugCollisions.push_back ( Info );
							}
						break;
						}
					case ECollisionShape::CYLINDER:
						{
						if (auto * cylinder = dynamic_cast< const CCylinderComponent * >( collision ))
							{
							FCollisionDebugInfo Info {};
							Info.WorldScale = WorldScale;
							Info.CreateCylinder (
								worldLoc,
								worldRot,
								cylinder->GetRadius (),
								cylinder->GetHeight (),
								debugColor
							);
							Collection.DebugCollisions.push_back ( Info );
							}
						break;
						}
					case ECollisionShape::CONE:
						{
						if (auto * cone = dynamic_cast< const CConeComponent * >( collision ))
							{
							FCollisionDebugInfo Info {};
							Info.WorldScale = WorldScale;
							Info.CreateCone (
								worldLoc,
								worldRot,
								cone->GetRadius (),
								cone->GetHeight (),
								debugColor
							);
							Collection.DebugCollisions.push_back ( Info );
							}
						break;
						}
					case ECollisionShape::TERRAIN:
						{
						if (auto * terrain = dynamic_cast< const CTerrainComponent * >( collision ))
							{
							FTerrainDebugInfo debugInfo;
							CWireframeGenerator::GenerateTerrainWireframe (
								debugInfo.WireframeVertices,
								terrain,
								FVector ( 0.7f, 0.7f, 0.7f ),
								false
							);
							debugInfo.Width = terrain->GetTerrainData ().Width;
							debugInfo.Height = terrain->GetTerrainData ().Height;
							debugInfo.CellSize = terrain->GetTerrainData ().CellSize;

							if (debugInfo.IsValid ())
								{
								Collection.TerrainWireframes.push_back ( debugInfo );
								}
							}
						break;
						}
					default:
						break;
				}
			};

			// Прямые коллизионные компоненты
		for (auto * comp : ActorComponents)
			{
			if (auto * collision = dynamic_cast< CBaseCollisionComponent * >( comp ))
				{
				ProcessCollisionComponent ( collision );
				}
			}

			// Дочерние коллизионные компоненты
		for (auto * comp : ActorComponents)
			{
			if (auto * transform = dynamic_cast< CTransformComponent * >( comp ))
				{
				for (auto * childComp : transform->GetChildTransformComponents ())
					{
					if (auto * collision = dynamic_cast< CBaseCollisionComponent * >( childComp ))
						{
						ProcessCollisionComponent ( collision );
						}
					}
				}
			}
		}

	return Collection;
	}

CLevel * CActor::GetLevel () const
	{
	if (GetWorld () == nullptr) return nullptr;
	return GetWorld ()->GetCurrentLevel ();
	}

CWorld * CActor::GetWorld () const
	{
	return CGameInstance::Get ().GetWorld ();
	}

void CActor::InitializeAllComponents ()
	{
	for (auto comp : ActorComponents)
		{
		comp->InitComponent ();
		}
	}

std::vector<FMeshInfo> CActor::GetRenderMeshes () const
	{
	std::vector<FMeshInfo> RenderMeshes;

	for (auto * comp : ActorComponents)
		{
		if (CBaseMeshComponent * mesh = dynamic_cast< CBaseMeshComponent * >( comp ))
			{
			if (mesh->IsReadyForRender ())
				{
				RenderMeshes.push_back ( mesh->GetMeshInfo () );
				}
			}
		}

	return RenderMeshes;
	}

void CActor::SetRootComponent ( CTransformComponent * NewRoot )
	{
	if (!NewRoot)
		{
		LOG_WARN ( "Cannot set nullptr as RootComponent" );
		return;
		}

	if (NewRoot->GetOwner () != this)
		{
		LOG_WARN ( "RootComponent must belong to this actor" );
		return;
		}

	if (RootComponent == NewRoot) return;

	CTransformComponent * OldRoot = RootComponent;

	if (OldRoot)
		{
		FVector WorldLocation = OldRoot->GetLocation ();
		FQuat WorldRotation = OldRoot->GetRotationQuat ();
		FVector WorldScale = OldRoot->GetScale ();

		OldRoot->AttachTo ( NewRoot );

		NewRoot->SetTransform ( FTransform ( WorldLocation, WorldRotation, WorldScale ) );

		OldRoot->SetRelativeLocation ( FVector::Zero () );
		OldRoot->SetRelativeRotation ( FQuat::Identity () );
		OldRoot->SetRelativeScale ( FVector::One () );

		RootComponent = NewRoot;
		}
	}

void CActor::Destroy ()
	{
	if (bIsPendingToDestroy)
		{
		LOG_WARN ( "Actor: ", GetName (), " already marked to destroy" );
		return;
		}

	auto level = GetLevel ();
	if (level)
		{
		level->DestroyActor ( GetName () );
		}
	}

void CActor::SetPendingToDestroy ()
	{
	if (bIsPendingToDestroy)
		{
		LOG_WARN ( "Actor: ", GetName (), " already marked to destroy" );
		return;
		}

	LOG_DEBUG ( "Actor:", GetName (), " is marked to destroy" );
	bIsPendingToDestroy = true;
	}

CBaseComponent * CActor::AddDefaultSubObject ( const std::string & className, const std::string & desiredDisplayName )
	{
	auto CompToAdd = CObject::AddSubObjectByClass ( className, desiredDisplayName );
	if (CBaseComponent * compToReturn = dynamic_cast< CBaseComponent * >( CompToAdd ))
		{
		return compToReturn;
		}
	return nullptr;
	}

FVector CActor::GetActorLocation ()
	{
	return RootComponent ? RootComponent->GetLocation () : FVector::Zero ();
	}

FVector CActor::GetActorRotation ()
	{
	FVector RotVec = GetActorRotationQuat ().GetEulerAngles ();
	return FVector (
		CEMath::RadiansToDegrees ( RotVec.x ),
		CEMath::RadiansToDegrees ( RotVec.y ),
		CEMath::RadiansToDegrees ( RotVec.z )
	);
	}

FVector CActor::GetActorScale ()
	{
	return RootComponent ? RootComponent->GetScale () : FVector::Zero ();
	}

FQuat CActor::GetActorRotationQuat ()
	{
	return RootComponent ? RootComponent->GetRotationQuat () : FQuat::Identity ();
	}

FVector CActor::GetActorLocation () const
	{
	return RootComponent ? RootComponent->GetLocation () : FVector::Zero ();
	}

FVector CActor::GetActorRotation () const
	{
	FVector RotVec = GetActorRotationQuat ().GetEulerAngles ();
	return FVector (
		CEMath::RadiansToDegrees ( RotVec.x ),
		CEMath::RadiansToDegrees ( RotVec.y ),
		CEMath::RadiansToDegrees ( RotVec.z )
	);
	}

FVector CActor::GetActorScale () const
	{
	return RootComponent ? RootComponent->GetScale () : FVector::Zero ();
	}

FQuat CActor::GetActorRotationQuat () const
	{
	return RootComponent ? RootComponent->GetRotationQuat () : FQuat::Identity ();
	}

FVector CActor::GetActorForwardVector ()
	{
	return RootComponent ? ( RootComponent->GetRotationQuat () * FVector::Forward () ) : FVector::Forward ();
	}

FVector CActor::GetActorRightVector ()
	{
	return RootComponent ? ( RootComponent->GetRotationQuat () * FVector::Right () ) : FVector::Right ();
	}

FVector CActor::GetActorUpVector ()
	{
	return RootComponent ? ( RootComponent->GetRotationQuat () * FVector::Up () ) : FVector::Up ();
	}

FVector CActor::GetActorForwardVector () const
	{
	return RootComponent ? ( RootComponent->GetRotationQuat () * FVector::Forward () ) : FVector::Forward ();
	}

FVector CActor::GetActorRightVector () const
	{
	return RootComponent ? ( RootComponent->GetRotationQuat () * FVector::Right () ) : FVector::Right ();
	}

FVector CActor::GetActorUpVector () const
	{
	return RootComponent ? ( RootComponent->GetRotationQuat () * FVector::Up () ) : FVector::Up ();
	}

void CActor::SetActorLocation ( const FVector & InLocation, bool bTeleport )
	{
	if (!bTeleport)
		{
		if (RootComponent)
			{
			RootComponent->SetLocation ( InLocation );
			bIsLerpingLocation = false;
			LocationLerpAlpha = 0.0f;
			RootComponent->MarkTransformDirty ();
			}
		}
	else
		{
		TeleportTo ( InLocation );
		}
	}

void CActor::SetActorLocation ( float inX, float inY, float inZ, bool bTeleport )
	{
	SetActorLocation ( FVector ( inX, inY, inZ ), bTeleport );
	}

void CActor::SetActorScale ( const FVector & InScale )
	{
	if (RootComponent)
		{
		RootComponent->SetScale ( InScale );
		}
	}

void CActor::SetActorScale ( float inX, float inY, float inZ )
	{
	SetActorScale ( FVector ( inX, inY, inZ ) );
	}

void CActor::SetActorScale ( float InScale )
	{
	SetActorScale ( InScale, InScale, InScale );
	}

void CActor::SetActorRotation ( const FVector & inRotation )
	{
	FQuat rotationQuat = FQuat::FromEulerAngles (
		CEMath::DegreesToRadians ( inRotation.x ),
		CEMath::DegreesToRadians ( inRotation.y ),
		CEMath::DegreesToRadians ( inRotation.z )
	);
	SetActorRotation ( rotationQuat );
	}

void CActor::SetActorRotation ( const FQuat & inRotation )
	{
	if (RootComponent)
		{
		RootComponent->SetRotation ( inRotation );
		bIsLerpingRotation = false;
		RotationLerpAlpha = 0.0f;
		}
	}

void CActor::SetActorRotation ( float inX, float inY, float inZ )
	{
	SetActorRotation ( FVector ( inX, inY, inZ ) );
	}

void CActor::TeleportTo ( const FVector & NewLocation )
	{
	LOG_DEBUG ( "[ACTOR] TeleportTo: ", GetName (), " to (",
				NewLocation.x, ", ", NewLocation.y, ", ", NewLocation.z, ")" );

	if (RootComponent)
		{
		RootComponent->SetLocation ( NewLocation );
		RootComponent->UpdateTransform ();
		}
	}

void CActor::TeleportTo ( float NewX, float NewY, float NewZ )
	{
	TeleportTo ( FVector ( NewX, NewY, NewZ ) );
	}

void CActor::SetActorRotationImmediately ( const FQuat & NewRotation )
	{
	LOG_DEBUG ( "[ACTOR] SetActorRotationImmediately: ", GetName () );
	SetActorRotation ( NewRotation );
	}

void CActor::SetActorRotationImmediately ( const FVector & NewRotation )
	{
	LOG_DEBUG ( "[ACTOR] SetActorRotationImmediately: ", GetName (),
				" to (", NewRotation.x, ", ", NewRotation.y, ", ", NewRotation.z, ")" );
	SetActorRotation ( NewRotation );
	}

void CActor::SetActorRotationImmediately ( float inX, float inY, float inZ )
	{
	SetActorRotationImmediately ( FVector ( inX, inY, inZ ) );
	}

void CActor::DestroyGravity ()
	{
	auto it = std::find ( ActorComponents.begin (), ActorComponents.end (), m_Gravity );
	if (it != ActorComponents.end ())
		{
		ActorComponents.erase ( it );
		}
	}

void CActor::MoveActor ( const FVector & Delta, bool Interpolate )
	{
	if (!RootComponent) return;

	FVector currentLocation = RootComponent->GetLocation ();
	FVector newTarget = currentLocation + Delta;

	if (!Interpolate)
		{
		RootComponent->SetLocation ( newTarget );
		bIsLerpingLocation = false;
		bIsMoving = false;
		}
	else
		{
		if (bIsLerpingLocation)
			{
			TargetLocation = newTarget;
			}
		else
			{
			TargetLocation = newTarget;
			LerpStartLocation = currentLocation;
			LocationLerpAlpha = 0.0f;
			bIsLerpingLocation = true;
			bIsMoving = true;
			}
		}

	if (m_Gravity)
		{
		m_Gravity->UpdateLastPosition ( currentLocation );
		}
	}

void CActor::RotateActor ( const FVector & DeltaRotation, bool Interpolate )
	{
	FQuat deltaQuat = FQuat::FromEulerAngles (
		CEMath::DegreesToRadians ( DeltaRotation.x ),
		CEMath::DegreesToRadians ( DeltaRotation.y ),
		CEMath::DegreesToRadians ( DeltaRotation.z )
	);

	RotateActor ( deltaQuat, Interpolate );
	}

void CActor::RotateActor ( const FQuat & DeltaRotation, bool Interpolate )
	{
	if (!RootComponent) return;

	FQuat currentQuat = RootComponent->GetRotationQuat ();
	FQuat targetQuat = currentQuat * DeltaRotation;
	targetQuat.Normalize ();

	if (!Interpolate)
		{
		RootComponent->SetRotation ( targetQuat );
		RootComponent->UpdateTransform ();
		return;
		}

	TargetRotation = targetQuat;
	LerpStartRotation = currentQuat;
	RotationLerpAlpha = 0.0f;
	bIsLerpingRotation = true;
	}


void CActor::AddActorWorldOffset ( const FVector & DeltaLocation, bool Interpolate )
	{
	MoveActor ( DeltaLocation, Interpolate );
	}

void CActor::AddActorLocalOffset ( const FVector & DeltaLocation, bool Interpolate )
	{
	if (!RootComponent) return;

	FQuat rotation = RootComponent->GetRotationQuat ();
	rotation.Normalize ();

	FVector worldDelta = rotation * DeltaLocation;
	AddActorWorldOffset ( worldDelta, Interpolate );
	}



void CActor::MoveActorInDirection ( const FVector & Direction, float Distance, bool Interpolate )
	{
	if (!RootComponent || Direction.IsZero ()) return;

	FVector normalizedDir = Direction.Normalized ();
	FVector delta = normalizedDir * Distance;

	MoveActor ( delta, Interpolate );
	}

void CActor::RotateAroundAxis ( const FVector & Axis, float AngleDegrees, bool Interpolate )
	{
	if (!RootComponent || Axis.IsZero ()) return;

	FQuat rotationQuat ( Axis.Normalized (), CEMath::DegreesToRadians ( AngleDegrees ) );
	RotateActor ( rotationQuat, Interpolate );
	}

void CActor::SetCollisionEnabled ( bool value )
	{
	bIsCollisionEnabled = value;
	if (RootComponent)
		{
		RootComponent->SetCollisionEnabled ( bIsCollisionEnabled );
		}
	}

void CActor::OnComponentBeginOverlap ( CBaseCollisionComponent * other )
	{
	if (!other) return;	
	}

void CActor::OnComponentEndOverlap ( CBaseCollisionComponent * other )
	{
	if (!other) return;	
	}

void CActor::SetActorName ( const std::string & newName )
	{
	this->Rename ( newName );
	}

void CActor::OnComponentHit ( CBaseCollisionComponent * other )
	{
	if (!other || !m_Gravity) return;

	if (CTerrainComponent * terrain = dynamic_cast< CTerrainComponent * >( other ))
		{
		m_Gravity->SetVerticalVelocity ( 0.0f );
		}
	CBaseCollisionComponent * myCollision = FindComponent<CBaseCollisionComponent> ();
	if (myCollision && ( myCollision->ShouldBlockWith ( other ) || other->ShouldBlockWith ( myCollision ) ))
		{
		
		
		}
	}

void CActor::SetMovableState ( const EMovableState & state )
	{
	MovableState = state;

	switch (MovableState)
		{
			case EMovableState::STATIC:
				{
				m_Gravity->SetEnableGravity ( false );
				Velocity = FVector::Zero ();
				SetCollisionEnabled ( true );
				break;
				}
			case EMovableState::MOVABLE:
				{
				m_Gravity->SetEnableGravity ( true );
				SetCollisionEnabled ( true );
				break;
				}
			case EMovableState::DYNAMIC:
				{
				m_Gravity->SetEnableGravity ( true );
				SetCollisionEnabled ( true );
				break;
				}
			default:
				break;
		}
	}

void CActor::AddImpulse ( const FVector & Impulse )
	{
	Velocity += Impulse;
	}

void CActor::SetVelocity ( const FVector & NewVelocity )
	{
	Velocity = NewVelocity;
	}

void CActor::UpdatePhysics ( float DeltaTime )
	{
	if (MovableState == EMovableState::STATIC) return;

	if (m_Gravity && m_Gravity->IsGravityEnabled ())
		{
		m_Gravity->ApplyGravity ( DeltaTime );
		Velocity.y = m_Gravity->GetVerticalVelocity ();
		}

	if (!Velocity.IsZero ())
		{
		MoveActor ( Velocity * DeltaTime );
		}

		// Затухание для импульсов
	Velocity *= 0.98f;
	if (Velocity.LengthSquared () < 0.01f)
		{
		Velocity = FVector::Zero ();
		}
	}