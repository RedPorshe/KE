#include "Components/GravityComponent.h"
#include "Actors/Actor.h"
#include "Core/CollisionSystem.h"
#include "Components/Collisions/BaseCollisionComponent.h"
#include "Components/Collisions/TerrainComponent.h"
#include "Utils/Math/CE_MathHelpers.h"
#include <cmath>
#include <limits>
#include <algorithm>

CGravityComponent::CGravityComponent ( CObject * inOwner, const std::string & inDisplayName )
    : Super ( inOwner, inDisplayName )
    {
    LOG_DEBUG ( "GravityComponent created: ", GetName () );
    }

CGravityComponent::~CGravityComponent ()
    {}

void CGravityComponent::InitComponent ()
    {
    Super::InitComponent ();
    CActor * owner = GetOwnerActor ();
    if (owner)
        {
        m_LastPosition = owner->GetActorLocation ();
        }
    }

void CGravityComponent::OnBeginPlay ()
    {
    Super::OnBeginPlay ();
    }

void CGravityComponent::ApplyGravity ( float DeltaTime )
    {
    CActor * owner = GetOwnerActor ();
    if (!owner) return;

    CBaseCollisionComponent * myCollision = owner->FindComponent<CBaseCollisionComponent> ();
    if (!myCollision) return;

    static float s_CorrectionTimer = 0.0f;
    s_CorrectionTimer += DeltaTime;

    FVector currentPos = owner->GetActorLocation ();
    FVector bottomPoint = myCollision->GetBottomPoint ();

    // Получаем все коллизионные компоненты
    std::vector<CBaseCollisionComponent *> allCollisions = COLLISION_SYSTEM.GetAllCollisionComponents ();

    // Находим террейн
    CTerrainComponent * terrain = nullptr;
    for (auto * comp : allCollisions)
        {
        if (comp && comp->GetOwnerActor () != owner &&
             comp->GetShapeType () == ECollisionShape::TERRAIN)
            {
            terrain = dynamic_cast< CTerrainComponent * >( comp );
            if (terrain) break;
            }
        }

    // Запоминаем предыдущее состояние
    m_bWasGrounded = bIsOnGround;

    // ПРОВЕРКА ЗЕМЛИ
    float checkDistance = m_GroundCheckDistance + myCollision->GetCollisionRadius () * 0.5f;
    FVector rayStart = bottomPoint;
    FVector rayEnd = bottomPoint + FVector ( 0.0f, -checkDistance, 0.0f );

    bool bRayHit = CheckGroundWithRay ( rayStart, rayEnd, myCollision );
    bool bSphereHit = CheckGroundWithSphere ( currentPos, myCollision );

    // Проверка через террейн
    bool bTerrainHit = false;
    float terrainHeight = 0.0f;
    if (terrain)
        {
        terrainHeight = terrain->GetHeightAtWorld ( currentPos );
        float bottomY = bottomPoint.y;

        if (bottomY <= terrainHeight + 5.0f)
            {
            bTerrainHit = true;
            m_GroundPoint = FVector ( currentPos.x, terrainHeight, currentPos.z );
            m_GroundNormal = FVector ( 0.0f, 1.0f, 0.0f );
            m_CurrentSlopeAngle = 0.0f;
            }
        }

    // Проверка нижних точек
    bool bExtremePointsHit = false;
    if (owner)
        {
        std::vector<FVector> directions = {
            FVector ( 0.0f, -1.0f, 1.0f ),
            FVector ( 0.0f, -1.0f, -1.0f ),
            FVector ( -1.0f, -1.0f, 0.0f ),
            FVector ( 1.0f, -1.0f, 0.0f ),
            FVector ( -0.7f, -1.0f, 0.7f ),
            FVector ( 0.7f, -1.0f, 0.7f ),
            FVector ( -0.7f, -1.0f, -0.7f ),
            FVector ( 0.7f, -1.0f, -0.7f )
            };

        for (const auto & dir : directions)
            {
            FVector point = myCollision->GetLocalExtremePoint ( dir );
            FRaycastResult pointResult = COLLISION_SYSTEM.Raycast (
                point,
                point + FVector ( 0.0f, -checkDistance, 0.0f ),
                myCollision->GetCollisionChannel ().GetName ()
            );

            if (pointResult.bHit && pointResult.HitComponent &&
                 pointResult.HitComponent->GetOwnerActor () != owner)
                {
                bExtremePointsHit = true;
                if (!bRayHit || pointResult.Location.y > m_GroundPoint.y)
                    {
                    m_GroundPoint = pointResult.Location;
                    m_GroundNormal = pointResult.Normal;

                    float dot = m_GroundNormal.Dot ( FVector ( 0.0f, 1.0f, 0.0f ) );
                    dot = CEMath::Clamp ( dot, -1.0f, 1.0f );
                    m_CurrentSlopeAngle = std::acos ( dot ) * ( 180.0f / 3.14159f );
                    }
                break;
                }
            }
        }

    // Объединяем результаты
    bIsOnGround = bRayHit || bSphereHit || bExtremePointsHit || bTerrainHit;

    // Проверка угла наклона
    if (bIsOnGround && m_CurrentSlopeAngle > m_MaxWalkableSlope)
        {
        bIsOnGround = false;
        }

    // Приземление
    if (bIsOnGround && !m_bWasGrounded)
        {
        m_VerticalVelocity = 0.0f;

        if (bTerrainHit && terrain)
            {
            float targetY = terrainHeight + ( currentPos.y - bottomPoint.y ) + 0.5f;
            owner->SetActorLocation ( FVector ( currentPos.x, targetY, currentPos.z ) );
            
            }
        else if (bRayHit)
            {
            FRaycastResult rayResult = COLLISION_SYSTEM.Raycast (
                rayStart,
                rayEnd,
                myCollision->GetCollisionChannel ().GetName ()
            );
            SnapToGround ( myCollision, rayResult );
            }

        s_CorrectionTimer = 0.0f;
        return;
        }

    // Коррекция проваливания
    if (bIsOnGround && terrain && s_CorrectionTimer > m_CorrectionCooldown)
        {
        float currentTerrainHeight = terrain->GetHeightAtWorld ( currentPos );
        float bottomY = bottomPoint.y;
        bool bValidTerrainHeight = ( currentTerrainHeight > -std::numeric_limits<float>::max () * 0.5f );

        if (bValidTerrainHeight)
            {
            float penetrationDepth = bottomY - currentTerrainHeight;
            if (penetrationDepth < -0.4f)
                {
                float targetY = currentPos.y - penetrationDepth;
                owner->SetActorLocation ( FVector ( currentPos.x, targetY, currentPos.z ) );
                
                s_CorrectionTimer = 0.0f;
                return;
                }
            }
        else
            {
            bIsOnGround = false;           
            s_CorrectionTimer = 0.0f;
            }
        }

    // ОСНОВНАЯ ЛОГИКА ДВИЖЕНИЯ
    if (bIsOnGround && m_VerticalVelocity > 0.1f)
        {
        // На земле, но есть положительная скорость (прыжок только начался)
        FlyUp ( DeltaTime );
        }
    else if (!bIsOnGround)
        {
        // В воздухе - падаем или летим
        Fall ( DeltaTime );
        }

    // Проверка kill zone
    if (bottomPoint.y <= m_KillZone)
        {
        LOG_DEBUG ( "Объект ", owner->GetName (), " достиг kill zone на Y=", bottomPoint.y );
        owner->SetPendingToDestroy ();
        }

    // Обновляем последнюю позицию
    m_LastPosition = owner->GetActorLocation ();

    }

bool CGravityComponent::CheckGroundWithSphere ( const FVector & position, CBaseCollisionComponent * collision )
    {
    if (!collision) return false;

    FVector bottomPoint = collision->GetBottomPoint ();
    float radius = collision->GetCollisionRadius () * 0.3f;

    std::vector<CBaseCollisionComponent *> overlaps = COLLISION_SYSTEM.SphereOverlap (
        bottomPoint + FVector ( 0.0f, -radius, 0.0f ),
        radius,
        collision->GetCollisionChannel ().GetName ()
    );

    for (auto * comp : overlaps)
        {
        if (comp != collision && comp->GetOwnerActor () != GetOwnerActor ())
            {
            m_GroundPoint = comp->GetTopPoint ();
            m_GroundNormal = FVector ( 0.0f, 1.0f, 0.0f );
            return true;
            }
        }

    return false;
    }

bool CGravityComponent::CheckGroundWithRay ( const FVector & start, const FVector & end, CBaseCollisionComponent * collision )
    {
    if (!collision) return false;

    FRaycastResult rayResult = COLLISION_SYSTEM.Raycast (
        start,
        end,
        collision->GetCollisionChannel ().GetName ()
    );

    if (rayResult.bHit && rayResult.HitComponent &&
         rayResult.HitComponent->GetOwnerActor () != GetOwnerActor ())
        {
        m_GroundPoint = rayResult.Location;
        m_GroundNormal = rayResult.Normal;

        float dot = m_GroundNormal.Dot ( FVector ( 0.0f, 1.0f, 0.0f ) );
        dot = CEMath::Clamp ( dot, -1.0f, 1.0f );
        m_CurrentSlopeAngle = std::acos ( dot ) * ( 180.0f / 3.14159f );

        return true;
        }

    return false;
    }

void CGravityComponent::SnapToGround ( CBaseCollisionComponent * collision, const FRaycastResult & rayResult )
    {
    if (!collision || !rayResult.bHit) return;

    CActor * owner = GetOwnerActor ();
    if (!owner) return;

    FVector currentPos = owner->GetActorLocation ();
    FVector bottomPoint = collision->GetBottomPoint ();

    // ВАЖНО: Прилипаем нижней точкой, а не центром!
    float targetY = rayResult.Location.y + ( currentPos.y - bottomPoint.y ) + 0.1f;
    owner->SetActorLocation ( FVector ( currentPos.x, targetY, currentPos.z ) );

    
    }

bool CGravityComponent::SweepMovement ( CBaseCollisionComponent * collision,
                                        const FVector & startPos,
                                        const FVector & endPos,
                                        FVector & outAdjustedPos,
                                        FVector & outHitNormal,
                                        CBaseCollisionComponent *& outHitComponent )
    {
    if (!collision) return false;

    CActor * owner = GetOwnerActor ();
    if (!owner) return false;

    FVector delta = endPos - startPos;
    float deltaLength = delta.Length ();

    if (deltaLength < 0.001f)
        {
        outAdjustedPos = endPos;
        return false;
        }

    FVector direction = delta / deltaLength;
    std::vector<CBaseCollisionComponent *> allCollisions = COLLISION_SYSTEM.GetAllCollisionComponents ();

    float closestHitTime = 1.0f;
    FVector closestNormal = FVector::Zero ();
    CBaseCollisionComponent * closestComponent = nullptr;

    for (auto * other : allCollisions)
        {
        if (!other || other == collision || other->GetOwnerActor () == owner)
            continue;

        if (!collision->CanCollideWith ( other ) || !other->CanCollideWith ( collision ))
            continue;

        FCollisionInfo startInfo;
        bool bStartCollision = collision->CheckCollision ( other, startInfo );
        if (bStartCollision) continue;

        FVector originalPos = owner->GetActorLocation ();
        owner->SetActorLocation ( endPos );

        FCollisionInfo endInfo;
        bool bEndCollision = collision->CheckCollision ( other, endInfo );
        owner->SetActorLocation ( originalPos );

        if (bEndCollision)
            {
            float low = 0.0f;
            float high = 1.0f;
            float hitTime = 1.0f;
            FVector hitNormal = endInfo.Normal;

            for (int i = 0; i < 5; i++)
                {
                float mid = ( low + high ) * 0.5f;
                FVector testPos = startPos + direction * ( deltaLength * mid );

                owner->SetActorLocation ( testPos );
                FCollisionInfo testInfo;
                bool bTestCollision = collision->CheckCollision ( other, testInfo );
                owner->SetActorLocation ( originalPos );

                if (bTestCollision)
                    {
                    high = mid;
                    hitTime = mid;
                    hitNormal = testInfo.Normal;
                    }
                else
                    {
                    low = mid;
                    }
                }

            if (hitTime < closestHitTime)
                {
                closestHitTime = hitTime;
                closestNormal = hitNormal;
                closestComponent = other;
                }
            }
        }

    if (closestHitTime < 1.0f)
        {
        outAdjustedPos = startPos + direction * ( deltaLength * ( closestHitTime - 0.01f ) );
        outHitNormal = closestNormal;
        outHitComponent = closestComponent;

        
        return true;
        }

    outAdjustedPos = endPos;
    outHitNormal = FVector::Zero ();
    outHitComponent = nullptr;
    return false;
    }

void CGravityComponent::Fall ( float DeltaTime )
    {
    CActor * owner = GetOwnerActor ();
    if (!owner) return;

    CBaseCollisionComponent * myCollision = owner->FindComponent<CBaseCollisionComponent> ();
    if (!myCollision) return;

    FVector currentPos = owner->GetActorLocation ();
    FVector bottomPoint = myCollision->GetBottomPoint ();

    // Применяем гравитацию
    m_VerticalVelocity -= m_GravityStrength * m_GravityScale * DeltaTime;
    if (m_VerticalVelocity < m_MaxFallSpeed)
        {
        m_VerticalVelocity = m_MaxFallSpeed;
        }

    float deltaMove = m_VerticalVelocity * DeltaTime;
    FVector targetPos = currentPos + FVector ( 0.0f, deltaMove, 0.0f );

    // Только при падении вниз проверяем коллизии
    if (m_VerticalVelocity < 0)
        {
        FVector bottomTarget = bottomPoint + FVector ( 0.0f, deltaMove, 0.0f );
        FVector adjustedBottom;
        FVector hitNormal;
        CBaseCollisionComponent * hitComponent = nullptr;

        if (SweepMovement ( myCollision, bottomPoint, bottomTarget, adjustedBottom, hitNormal, hitComponent ))
            {
            float moveRatio = ( adjustedBottom.y - bottomPoint.y ) / deltaMove;
            float newY = currentPos.y + deltaMove * moveRatio;
            owner->SetActorLocation ( FVector ( currentPos.x, newY, currentPos.z ) );

            if (hitNormal.y > 0.5f)
                {
                m_VerticalVelocity = 0.0f;
                bIsOnGround = true;
              
                }
            else
                {
                m_VerticalVelocity = 0.0f;
                LOG_DEBUG ( "[FALL WALL] Удар о стену" );
                }
            }
        else
            {
            owner->SetActorLocation ( targetPos );
            }
        }
    else // Летим вверх или стоим
        {
        owner->SetActorLocation ( targetPos );
        }
    }

void CGravityComponent::FlyUp ( float DeltaTime )
    {
    CActor * owner = GetOwnerActor ();
    if (!owner) return;

    CBaseCollisionComponent * myCollision = owner->FindComponent<CBaseCollisionComponent> ();
    if (!myCollision) return;

    FVector currentPos = owner->GetActorLocation ();

    // При полете вверх гравитация всё равно действует
    m_VerticalVelocity -= m_GravityStrength * m_GravityScale * DeltaTime;

    // Но мы можем лететь вверх, если скорость еще положительная
    if (m_VerticalVelocity > 0)
        {
        float deltaMove = m_VerticalVelocity * DeltaTime;
        FVector targetPos = currentPos + FVector ( 0.0f, deltaMove, 0.0f );
        owner->SetActorLocation ( targetPos );
        }
    else
        {
        // Скорость стала отрицательной - начинаем падение
        // Ничего не делаем, Fall() обработает в следующем кадре
        }
    }

void CGravityComponent::Tick ( float DeltaTime )
    {
    Super::Tick ( DeltaTime );

   
    }

float CGravityComponent::GetJumpVelocity ( float JumpHeight ) const
    {
    float gravity = m_GravityStrength * m_GravityScale;
    return std::sqrt ( 2.0f * gravity * JumpHeight );
    }