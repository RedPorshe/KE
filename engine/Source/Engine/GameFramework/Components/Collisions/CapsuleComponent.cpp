#include "Components/Collisions/CapsuleComponent.h"
#include "Actors/Actor.h"
#include "Core/CollisionSystem.h"
#include <cmath>

// ============================================================================
// Constructors & Destructor
// ============================================================================

CCapsuleComponent::CCapsuleComponent ( CObject * inOwner, const std::string & InName )
	: Super ( inOwner, InName )
	{
	SetShapeType ( ECollisionShape::CAPSULE );
	SetChannelAsPawn ();
	SetResponseToChannel ( ECollisionChannel::WorldDynamic, ECollisionResponse::BLOCK );
	SetResponseToChannel ( ECollisionChannel::WorldStatic, ECollisionResponse::BLOCK );
	SetResponseToChannel ( ECollisionChannel::Trigger, ECollisionResponse::OVERLAP );
	SetResponseToChannel ( ECollisionChannel::Character, ECollisionResponse::BLOCK );
	SetResponseToChannel ( ECollisionChannel::Pawn, ECollisionResponse::BLOCK );
	LOG_DEBUG ( "CapsuleComponent created: ", GetName (),
				", Radius: ", m_Radius,
				", HalfHeight: ", m_HalfHeight );
	}

CCapsuleComponent::~CCapsuleComponent ()
	{}

	// ============================================================================
	// Component Lifecycle
	// ============================================================================

void CCapsuleComponent::InitComponent ()
	{
	Super::InitComponent ();
	}

void CCapsuleComponent::Tick ( float DeltaTime )
	{
	Super::Tick ( DeltaTime );
	}

void CCapsuleComponent::OnBeginPlay ()
	{
	Super::OnBeginPlay ();
	}

	// ============================================================================
	// Collision Interface
	// ============================================================================

bool CCapsuleComponent::CheckCollision ( CBaseCollisionComponent * other, FCollisionInfo & outInfo ) const
	{
	if (!other || !IsCollisionEnabled () || !other->IsCollisionEnabled ())
		return false;

	if (!CanCollideWith ( other ))
		return false;

	ECollisionShape otherShape = other->GetShapeType ();

	CCapsuleComponent * nonConstThis = const_cast< CCapsuleComponent * >( this );


	switch (otherShape)
		{
			case ECollisionShape::NONE:
				{
				return false;
				}

			case ECollisionShape::SPHERE:
				{
				return COLLISION_SYSTEM.CheckSphereCapsule ( other, nonConstThis, outInfo );
				}

			case ECollisionShape::BOX:
				{
				return COLLISION_SYSTEM.CheckBoxCapsule ( other, nonConstThis, outInfo );
				}

			case ECollisionShape::CAPSULE:
				{
				return COLLISION_SYSTEM.CheckCapsuleCapsule ( nonConstThis, other, outInfo );
				}

			case ECollisionShape::CYLINDER:
				{

				return COLLISION_SYSTEM.CheckCapsuleCylinder ( nonConstThis, other, outInfo );
				}

			case ECollisionShape::CONE:
				{
				return COLLISION_SYSTEM.CheckCapsuleCone ( nonConstThis, other, outInfo );
				}

			case ECollisionShape::COMPOUND:
				{
					// TODO: Implement Sphere-Compound collision
				LOG_DEBUG ( "stub for Sphere-Compound collision" );
				return false;
				}

			case ECollisionShape::MESH:
				{
					// TODO: Implement Sphere-Mesh collision
				LOG_DEBUG ( "stub for Sphere-Mesh collision" );
				return false;
				}

			case ECollisionShape::TERRAIN:
				{
				return COLLISION_SYSTEM.CheckCapsuleTerrain ( nonConstThis, other, outInfo );
				}

			case ECollisionShape::RAY:
				{
					// TODO: Implement Sphere-Ray collision
				LOG_DEBUG ( "stub for Sphere-Ray collision" );
				return false;
				}

			case ECollisionShape::PLANE:
				{
					// TODO: Implement Sphere-Plane collision
				LOG_DEBUG ( "stub for Sphere-Plane collision" );
				return false;
				}

			case ECollisionShape::MAX:
			default:
				break;
		}

	return false;
	}

float CCapsuleComponent::GetCollisionRadius () const
	{
	return    m_Radius + m_HalfHeight ;
	}

FVector CCapsuleComponent::GetBoundingBox () const
	{
		// Возвращаем размеры ограничивающего бокса
	float totalHeight = GetTotalHeight ();
	return FVector (
		m_Radius * 2.0f,           // Ширина (X)
		m_Radius * 2.0f,           // Глубина (Y)
		totalHeight                 // Высота (Z)
	);
	}

	// ============================================================================
	// Capsule Specific Methods
	// ============================================================================

FVector CCapsuleComponent::GetTopSphereCenter () const
	{
	FVector worldPos = GetWorldLocation ();
	FQuat rotation = GetOwnerActor ()->GetActorRotationQuat ();

	// Смещение вверх по локальной оси Z
	FVector localOffset ( 0, 0, m_HalfHeight );
	FVector worldOffset = rotation * localOffset;

	return worldPos + worldOffset;
	}

FVector CCapsuleComponent::GetBottomSphereCenter () const
	{
	FVector worldPos = GetWorldLocation ();
	FQuat rotation = GetOwnerActor ()->GetActorRotationQuat ();

	// Смещение вниз по локальной оси Z
	FVector localOffset ( 0, 0, -m_HalfHeight );
	FVector worldOffset = rotation * localOffset;

	return worldPos + worldOffset;
	}

FVector CCapsuleComponent::GetExtremePoint ( const FVector & Direction ) const
	{
	FVector center = GetWorldLocation ();
	FQuat rotation = GetOwnerActor ()->GetActorRotationQuat ();

	// Направление в локальном пространстве
	FVector localDir = rotation.Inverse () * Direction;
	float len = localDir.Length ();
	if (len < 0.001f)
		return center;

	localDir /= len;

	// Для капсулы: комбинация сферы на концах и цилиндра посередине
	// Проецируем направление на ось капсулы (локальная Z)
	float axisProjection = localDir.z;
	float radialScale = std::sqrt ( 1.0f - axisProjection * axisProjection );

	// Точка на капсуле
	FVector localPoint (
		localDir.x * m_Radius,
		localDir.y * m_Radius,
		axisProjection * ( m_HalfHeight + m_Radius * ( 1.0f - std::abs ( axisProjection ) ) )
	);

	return center + rotation * localPoint;
	}