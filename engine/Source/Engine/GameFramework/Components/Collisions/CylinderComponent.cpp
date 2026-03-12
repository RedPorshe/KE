#include "Components/Collisions/CylinderComponent.h"
#include "Actors/Actor.h"
#include "Core/CollisionSystem.h"
#include <cmath>

CCylinderComponent::CCylinderComponent ( CObject * inOwner, const std::string & inDisplayName )
	: CBaseCollisionComponent ( inOwner, inDisplayName )
	{
	SetShapeType ( ECollisionShape::CYLINDER );
	LOG_DEBUG ( "CylinderComponent created: ", GetName (),
				", Radius: ", m_Radius,
				", Height: ", m_Height );
	}

CCylinderComponent::~CCylinderComponent ()
	{}

void CCylinderComponent::InitComponent ()
	{
	Super::InitComponent ();
	}

void CCylinderComponent::Tick ( float DeltaTime )
	{
	Super::Tick ( DeltaTime );
	}

void CCylinderComponent::OnBeginPlay ()
	{
	Super::OnBeginPlay ();
	}

	// ============================================================================
	// Collision Checks
	// ============================================================================

bool CCylinderComponent::CheckCollision ( CBaseCollisionComponent * other, FCollisionInfo & outInfo ) const
	{
	if (!other || !IsCollisionEnabled () || !other->IsCollisionEnabled ())
		return false;

	if (!CanCollideWith ( other ))
		return false;

	ECollisionShape otherShape = other->GetShapeType ();
	CCylinderComponent * nonConstThis = const_cast< CCylinderComponent * >( this );

	switch (otherShape)
		{
			case ECollisionShape::NONE:
				return false;

			case ECollisionShape::SPHERE:
				return COLLISION_SYSTEM.CheckSphereCylinder ( other, nonConstThis, outInfo );

			case ECollisionShape::BOX:
				return COLLISION_SYSTEM.CheckBoxCylinder ( other, nonConstThis, outInfo );

			case ECollisionShape::CAPSULE:
				return COLLISION_SYSTEM.CheckCapsuleCylinder ( other, nonConstThis, outInfo );

			case ECollisionShape::CYLINDER:
				return COLLISION_SYSTEM.CheckCylinderCylinder ( nonConstThis, other, outInfo );

			case ECollisionShape::CONE:
				return COLLISION_SYSTEM.CheckCylinderCone ( nonConstThis, other, outInfo );

			case ECollisionShape::COMPOUND:
				LOG_DEBUG ( "stub for Cylinder-Compound collision" );
				return false;

			case ECollisionShape::MESH:
				LOG_DEBUG ( "stub for Cylinder-Mesh collision" );
				return false;

			case ECollisionShape::TERRAIN:
				return COLLISION_SYSTEM.CheckCylinderTerrain ( nonConstThis, other, outInfo );

			case ECollisionShape::RAY:
				LOG_DEBUG ( "stub for Cylinder-Ray collision" );
				return false;

			case ECollisionShape::PLANE:
				LOG_DEBUG ( "stub for Cylinder-Plane collision" );
				return false;

			case ECollisionShape::MAX:
			default:
				break;
		}

	return false;
	}

float CCylinderComponent::GetCollisionRadius () const
	{
		// Для грубых проверок возвращаем максимальный радиус описанной сферы
	float halfHeight = m_Height * 0.5f;
	return std::sqrt ( m_Radius * m_Radius + halfHeight * halfHeight );
	}

	// ============================================================================
	// Cylinder Specific Methods
	// ============================================================================

FVector CCylinderComponent::GetTopCenter () const
	{
	FVector worldPos = GetWorldLocation ();
	FQuat rotation = GetOwnerActor ()->GetActorRotationQuat ();

	// Смещение вверх по локальной оси Z
	FVector localOffset ( 0.0f, 0.0f, GetHalfHeight () );
	FVector worldOffset = rotation * localOffset;

	return worldPos + worldOffset;
	}

FVector CCylinderComponent::GetBottomCenter () const
	{
	FVector worldPos = GetWorldLocation ();
	FQuat rotation = GetOwnerActor ()->GetActorRotationQuat ();

	// Смещение вниз по локальной оси Z
	FVector localOffset ( 0.0f, 0.0f, -GetHalfHeight () );
	FVector worldOffset = rotation * localOffset;

	return worldPos + worldOffset;
	}

FVector CCylinderComponent::GetExtremePoint ( const FVector & Direction ) const
	{
	FVector center = GetWorldLocation ();
	FQuat rotation = GetOwnerActor ()->GetActorRotationQuat ();

	// Направление в локальном пространстве
	FVector localDir = rotation.Inverse () * Direction;
	float len = localDir.Length ();
	if (len < 0.001f)
		return center;

	localDir /= len;

	// Для цилиндра: ограничиваем по радиусу в плоскости XZ и по высоте по Y
	FVector localPoint (
		localDir.x * m_Radius,
		( localDir.y > 0 ) ? GetHalfHeight () : -GetHalfHeight (),
		localDir.z * m_Radius
	);

	return center + rotation * localPoint;
	}