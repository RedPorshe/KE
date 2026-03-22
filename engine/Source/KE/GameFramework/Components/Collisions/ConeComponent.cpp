#include "KE/GameFramework/Components/Collisions/ConeComponent.h"
#include "KE/GameFramework/Actors/Actor.h"
#include "KE/Systems/CollisionSystem.h"
#include <cmath>

CConeComponent::CConeComponent ( CObject * inOwner, const std::string & inDisplayName )
    : Super ( inOwner, inDisplayName )
    {
    SetShapeType ( ECollisionShape::CONE );
    LOG_DEBUG ( "ConeComponent created: ", GetName (),
                ", Radius: ", m_Radius,
                ", Height: ", m_Height );
    }

CConeComponent::~CConeComponent ()
    {}

void CConeComponent::InitComponent ()
    {
    Super::InitComponent ();
    }

void CConeComponent::Tick ( float DeltaTime )
    {
    Super::Tick ( DeltaTime );
    }

void CConeComponent::OnBeginPlay ()
    {
    Super::OnBeginPlay ();
    }

    // ============================================================================
    // Collision Checks
    // ============================================================================

bool CConeComponent::CheckCollision ( CBaseCollisionComponent * other, FCollisionInfo & outInfo ) const
    {
    if (!other || !IsCollisionEnabled () || !other->IsCollisionEnabled ())
        return false;

    if (!CanCollideWith ( other ))
        return false;

    ECollisionShape otherShape = other->GetShapeType ();
    CConeComponent * nonConstThis = const_cast< CConeComponent * >( this );

    switch (otherShape)
        {
            case ECollisionShape::NONE:
                return false;

            case ECollisionShape::SPHERE:
                return COLLISION_SYSTEM.CheckSphereCone ( other, nonConstThis, outInfo );

            case ECollisionShape::BOX:
                return COLLISION_SYSTEM.CheckBoxCone ( other, nonConstThis, outInfo );

            case ECollisionShape::CAPSULE:
                return COLLISION_SYSTEM.CheckCapsuleCone ( other, nonConstThis, outInfo );

            case ECollisionShape::CYLINDER:
                return COLLISION_SYSTEM.CheckCylinderCone ( other, nonConstThis, outInfo );

            case ECollisionShape::CONE:
                return COLLISION_SYSTEM.CheckConeCone ( nonConstThis, other, outInfo );

            case ECollisionShape::TERRAIN:
                return COLLISION_SYSTEM.CheckConeTerrain ( nonConstThis, other, outInfo );

            case ECollisionShape::COMPOUND:
            case ECollisionShape::MESH:
            case ECollisionShape::RAY:
            case ECollisionShape::PLANE:
                LOG_DEBUG ( "stub for Cone-", ( int ) otherShape, " collision" );
                return false;

            case ECollisionShape::MAX:
            default:
                break;
        }

    return false;
    }

float CConeComponent::GetCollisionRadius () const
    {
        // Для грубых проверок возвращаем радиус описанной сферы
    return std::sqrt ( m_Radius * m_Radius + m_Height * m_Height );
    }

    // ============================================================================
    // Cone Specific Methods
    // ============================================================================

FVector CConeComponent::GetTip () const
    {
    FVector worldPos = GetWorldLocation ();
    FQuat rotation = GetOwnerActor ()->GetActorRotationQuat ();

    // Острие конуса - вверху по локальной оси Y
    FVector localOffset ( 0.0f, m_Height * 0.5f, 0.0f );
    FVector worldOffset = rotation * localOffset;

    return worldPos + worldOffset;
    }

FVector CConeComponent::GetBaseCenter () const
    {
    FVector worldPos = GetWorldLocation ();
    FQuat rotation = GetOwnerActor ()->GetActorRotationQuat ();

    // Центр основания - внизу по локальной оси Y
    FVector localOffset ( 0.0f, -m_Height * 0.5f, 0.0f );
    FVector worldOffset = rotation * localOffset;

    return worldPos + worldOffset;
    }

float CConeComponent::GetSlope () const
    {
        // Наклон стенки = радиус / высота
    return m_Radius / m_Height;
    }

FVector CConeComponent::GetExtremePoint ( const FVector & Direction ) const
    {
    FVector center = GetWorldLocation ();
    FQuat rotation = GetOwnerActor ()->GetActorRotationQuat ();

    // Направление в локальном пространстве
    FVector localDir = rotation.Inverse () * Direction;
    float len = localDir.Length ();
    if (len < 0.001f)
        return center;

    localDir /= len;

    // Для конуса: основание внизу (y = -halfHeight), острие вверху (y = +halfHeight)
    float halfHeight = m_Height * 0.5f;

    // Проверяем разные части конуса
    FVector bestPoint;
    float bestProjection = -std::numeric_limits<float>::max ();

    // Проверяем острие
    FVector tip ( 0.0f, halfHeight, 0.0f );
    float tipProj = tip.Dot ( localDir );
    if (tipProj > bestProjection)
        {
        bestProjection = tipProj;
        bestPoint = tip;
        }

        // Проверяем окружность основания
    const int numSamples = 16;
    for (int i = 0; i < numSamples; i++)
        {
        float angle = ( 2.0f * 3.14159f * i ) / numSamples;
        FVector basePoint (
            m_Radius * cos ( angle ),
            -halfHeight,
            m_Radius * sin ( angle )
        );

        float proj = basePoint.Dot ( localDir );
        if (proj > bestProjection)
            {
            bestProjection = proj;
            bestPoint = basePoint;
            }
        }

    return center + rotation * bestPoint;
    }