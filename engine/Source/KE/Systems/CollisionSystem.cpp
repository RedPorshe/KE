#include "KE/Systems/CollisionSystem.h"
#include "KE/GameFramework/Actors/Actor.h"
#include "KE/GameFramework/Components/TransformComponent.h"
#include "KE/GameFramework/Components/Collisions/SphereComponent.h"
#include "KE/GameFramework/Components/Collisions/CapsuleComponent.h"
#include "KE/GameFramework/Components/Collisions/CylinderComponent.h"
#include "KE/GameFramework/Components/Collisions/ConeComponent.h"
#include "KE/GameFramework/Components/GravityComponent.h"
#include "KE/GameFramework/Components/Collisions/BoxComponent.h"
#include "KE/GameFramework/Components/Collisions/TerrainComponent.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

// ============================================================================
// Static Member Initialization
// ============================================================================
CCollisionSystem * CCollisionSystem::s_Instance = nullptr;
bool CCollisionSystem::s_IsInitialized = false;
bool CCollisionSystem::bIsGlobalShuttingDown = false;  
static bool bIsAlreadyShuttingDown = false;  

// ============================================================================
// Constructors & Destructor
// ============================================================================

CCollisionSystem::CCollisionSystem ( )
    : IEngineSystem()
    {
    LogDebug ( "created " );
    }

CCollisionSystem::~CCollisionSystem ()
    {
    if (!m_CollisionComponents.empty ())
        {
        LOG_WARN ( "CollisionSystem destroyed with ",
                   m_CollisionComponents.size (),
                   " components still registered!" );
        
        Shutdown ();
        }
        Shutdown ();
    }

bool CCollisionSystem::PreInit ()
    {
    s_Instance = this;
    LogDebug ( "PreInit - Checking collision system" );
    return true;
    }

bool CCollisionSystem::Init ()
    {
    LogDebug ( "Initializing CollisionSystem" );

    if (!GetEngine ())
        {
        LogError ( "Engine not set" );
        return false;
        }

    bIsInitialized = true;
    LogInfo ( "Collision system initialized successfully" );
    return true;
    }

void CCollisionSystem::Shutdown ()
    {
    if (!bIsAlreadyShuttingDown)
        {
        bIsGlobalShuttingDown = true;  // Устанавливаем глобальный флаг ДО очистки
        bIsAlreadyShuttingDown = true;

        LOG_DEBUG ( "Collision System shutting down..." );

        // Отключаем коллизии у всех компонентов, но НЕ отписываем их
        // (они сами отпишутся в деструкторах, если смогут)
        for (CBaseCollisionComponent * comp : m_CollisionComponents)
            {
            if (comp)
                {
                comp->SetCollisionEnabled ( false );
                }
            }

            // Очищаем все контейнеры
        m_CollisionComponents.clear ();
        m_SpatialGrid.clear ();
        m_Callbacks.clear ();
        m_LastPositions.clear ();
        m_PreviousFrameCollisions.clear ();
        m_CurrentFrameCollisions.clear ();

        // Сбрасываем флаги
        bDebugDraw = false;
        m_LastFrameCollisions = 0;
        m_AccumulatedTime = 0.0f;
        s_Instance = nullptr;
        LOG_DEBUG ( "Collision System shutdown complete" );
        }
    }

    // ============================================================================
    // Singleton Access
    // ============================================================================

CCollisionSystem & CCollisionSystem::Get ()
    {  
    if (!s_Instance)
        {
            // Если синглтон не инициализирован, создаём временный
            // (но обычно PreInit должен вызываться до этого)
        static CCollisionSystem fallbackInstance;
        return fallbackInstance;
        }
    return *s_Instance;
    }

    // ============================================================================
    // Component Registration
    // ============================================================================

void CCollisionSystem::RegisterCollisionComponent ( CBaseCollisionComponent * component )
    {
        // Не регистрируем новые компоненты во время выключения
    if (bIsGlobalShuttingDown || bIsAlreadyShuttingDown)
        {
        LOG_WARN ( "Attempted to register component during shutdown: ", component ? component->GetName () : "null" );
        return;
        }

    if (!component)
        return;

    auto it = std::find ( m_CollisionComponents.begin (), m_CollisionComponents.end (), component );

    if (it == m_CollisionComponents.end ())
        {
        m_CollisionComponents.push_back ( component );

        if (component->GetOwnerActor ())
            {
            m_LastPositions[ component ] = component->GetOwnerActor ()->GetActorLocation ();
            }

        LOG_DEBUG ( "Registered collision component: ", component->GetName () );
        }
    }

void CCollisionSystem::UnregisterCollisionComponent ( CBaseCollisionComponent * component )
    {
        // Если система выключается, просто игнорируем запросы на отписку
    if (bIsGlobalShuttingDown || bIsAlreadyShuttingDown)
        {
        return;
        }

    if (!component)
        return;

    auto it = std::find ( m_CollisionComponents.begin (), m_CollisionComponents.end (), component );

    if (it != m_CollisionComponents.end ())
        {
        m_CollisionComponents.erase ( it );
        m_LastPositions.erase ( component );

        // Remove from collision tracking sets
        for (auto pairIt = m_PreviousFrameCollisions.begin (); pairIt != m_PreviousFrameCollisions.end ();)
            {
            if (pairIt->first == component || pairIt->second == component)
                pairIt = m_PreviousFrameCollisions.erase ( pairIt );
            else
                ++pairIt;
            }

        for (auto pairIt = m_CurrentFrameCollisions.begin (); pairIt != m_CurrentFrameCollisions.end ();)
            {
            if (pairIt->first == component || pairIt->second == component)
                pairIt = m_CurrentFrameCollisions.erase ( pairIt );
            else
                ++pairIt;
            }

        LOG_DEBUG ( "Unregistered collision component: ", component->GetName () );
        }
    }

    // ============================================================================
    // Main Update
    // ============================================================================

void CCollisionSystem::Update ( float deltaTime )
    {
    if (!IsInitialized ()) return;

    if (bIsGlobalShuttingDown || bIsAlreadyShuttingDown) return;

    m_AccumulatedTime += deltaTime;

    float updateInterval = 1.0f / m_UpdateRate;

    if (m_AccumulatedTime >= updateInterval)
        {
        ProcessCollisions ();
        m_AccumulatedTime -= updateInterval;
        }
    }

const std::string CCollisionSystem::GetSystemName () const
    {
    return "CollisionSystem";
    }

    // ============================================================================
    // Core Collision Processing
    // ============================================================================


void CCollisionSystem::ProcessCollisions ()
    {
        // Не обрабатываем коллизии во время выключения
    if (bIsGlobalShuttingDown || bIsAlreadyShuttingDown)
        return;

    m_LastFrameCollisions = 0;

    // Меняем местами множества вместо очистки и вставки (эффективнее)
    std::swap ( m_PreviousFrameCollisions, m_CurrentFrameCollisions );
    m_CurrentFrameCollisions.clear ();

    // Оптимизация: используем пространственное разделение если включено
    if (bUseSpatialPartition && !m_SpatialGrid.empty ())
        {
        ProcessCollisionsSpatial ();
        }
    else
        {
            // Стандартный O(n²) подход для маленьких наборов
        const size_t numComponents = m_CollisionComponents.size ();

        for (size_t i = 0; i < numComponents; ++i)
            {
            CBaseCollisionComponent * compA = m_CollisionComponents[ i ];

            if (!IsValidCollisionComponent ( compA ))
                continue;

            for (size_t j = i + 1; j < numComponents; ++j)
                {
                CBaseCollisionComponent * compB = m_CollisionComponents[ j ];

                if (!IsValidCollisionComponent ( compB ))
                    continue;

                // Быстрая проверка каналов перед детальной коллизией
                if (!compA->CanCollideWith ( compB ) || !compB->CanCollideWith ( compA ))
                    continue;

                FCollisionInfo info {};
                info.ComponentA = compA;
                info.ComponentB = compB;

                // Проверяем коллизию через компонент
                if (compA->CheckCollision ( compB, info ))
                    {
                    ProcessComponentPair ( info );
                    }
                }
            }
        }

        // ВАЖНО: Обрабатываем пары, которые были в прошлом кадре, но исчезли в этом
    for (const auto & pair : m_PreviousFrameCollisions)
        {
            // Если этой пары нет в текущем кадре, значит оверлап закончился
        if (m_CurrentFrameCollisions.find ( pair ) == m_CurrentFrameCollisions.end ())
            {
            CBaseCollisionComponent * compA = pair.first;
            CBaseCollisionComponent * compB = pair.second;

            if (compA && compB)
                {
                    // Уведомляем об окончании оверлапа
                compA->OnEndOverlap ( compB );
                compB->OnEndOverlap ( compA );

                FireCollisionEvent ( ECollisionEventType::END_OVERLAP,
                                     FCollisionInfo { compA, compB, FVector (), FVector (), 0.0f } );
                }
            }
        }
    }


    // ============================================================================
    // Capsule Collision Checks
    // ============================================================================

bool CCollisionSystem::CheckSphereCapsule ( CBaseCollisionComponent * sphere,
                                            CBaseCollisionComponent * capsule,
                                            FCollisionInfo & outInfo ) const
    {
    CCapsuleComponent * cap = dynamic_cast< CCapsuleComponent * >( capsule );
    if (!cap) return false;

    FVector spherePos = sphere->GetWorldLocation ();
    float sphereRadius = sphere->GetCollisionRadius ();

    // Получаем центры полусфер капсулы
    FVector topCenter = cap->GetTopSphereCenter ();
    FVector bottomCenter = cap->GetBottomSphereCenter ();
    float capRadius = cap->GetRadius ();

    // Находим ближайшую точку на оси капсулы к центру сферы
    FVector axis = topCenter - bottomCenter;
    float axisLength = axis.Length ();

    if (axisLength < 0.001f)
        {
            // Капсула вырождена в сферу
        return CheckSphereSphere ( sphere, capsule, outInfo );
        }

    FVector axisDir = axis / axisLength;
    FVector sphereToBottom = spherePos - bottomCenter;

    // Проекция центра сферы на ось капсулы
    float t = sphereToBottom.Dot ( axisDir );
    t = std::max ( 0.0f, std::min ( t, axisLength ) );

    FVector closestPointOnAxis = bottomCenter + axisDir * t;

    // Проверяем расстояние
    FVector delta = spherePos - closestPointOnAxis;
    float distance = delta.Length ();
    float radiusSum = sphereRadius + capRadius;

    if (distance <= radiusSum)
        {
        outInfo.ComponentA = sphere;
        outInfo.ComponentB = capsule;
        outInfo.Depth = radiusSum - distance;

        if (distance > 0.001f)
            {
            outInfo.Normal = delta / distance;
            outInfo.Location = closestPointOnAxis + outInfo.Normal * capRadius;
            }
        else
            {
            outInfo.Normal = FVector ( 0.0f, 0.0f, 1.0f );
            outInfo.Location = spherePos;
            }

        return true;
        }

    return false;
    }

bool CCollisionSystem::CheckBoxCapsule ( CBaseCollisionComponent * box,
                                         CBaseCollisionComponent * capsule,
                                         FCollisionInfo & outInfo ) const
    {
    CBoxComponent * boxComp = dynamic_cast< CBoxComponent * >( box );
    CCapsuleComponent * cap = dynamic_cast< CCapsuleComponent * >( capsule );

    if (!boxComp || !cap) return false;

    FVector boxPos = box->GetWorldLocation ();
    FVector boxHalf = boxComp->GetHalfExtents ();
    FQuat boxRot = box->GetOwnerActor ()->GetActorRotationQuat ();

    // Получаем центры полусфер капсулы
    FVector capTop = cap->GetTopSphereCenter ();
    FVector capBottom = cap->GetBottomSphereCenter ();
    float capRadius = cap->GetRadius ();

    // Преобразуем точки капсулы в локальное пространство бокса
    FVector localTop = boxRot.Inverse () * ( capTop - boxPos );
    FVector localBottom = boxRot.Inverse () * ( capBottom - boxPos );
    FVector localAxis = localTop - localBottom;
    float axisLength = localAxis.Length ();

    if (axisLength < 0.001f)
        {
            // Капсула вырождена в сферу - используем sphere-box
        CSphereComponent * tempSphere = dynamic_cast< CSphereComponent * > ( capsule );
        if (tempSphere)
            return CheckSphereBox ( capsule, box, outInfo );
        return false;
        }

    FVector localDir = localAxis / axisLength;

    // Находим ближайшую точку на отрезке капсулы к боксу
    float t = 0.0f;
    FVector localClosest;

    // Для каждого измерения находим параметр t
    for (int i = 0; i < 3; i++)
        {
        if (std::abs ( localDir[ i ] ) > 0.001f)
            {
            float t1 = ( -boxHalf[ i ] - localBottom[ i ] ) / localDir[ i ];
            float t2 = ( boxHalf[ i ] - localBottom[ i ] ) / localDir[ i ];

            float tMin = std::min ( t1, t2 );
            float tMax = std::max ( t1, t2 );

            t = std::max ( t, tMin );
            t = std::min ( t, tMax );
            }
        }

    t = std::max ( 0.0f, std::min ( t, axisLength ) );
    localClosest = localBottom + localDir * t;

    // Ограничиваем точку границами бокса
    localClosest.x = std::max ( -boxHalf.x, std::min ( localClosest.x, boxHalf.x ) );
    localClosest.y = std::max ( -boxHalf.y, std::min ( localClosest.y, boxHalf.y ) );
    localClosest.z = std::max ( -boxHalf.z, std::min ( localClosest.z, boxHalf.z ) );

    // Преобразуем обратно в мировое пространство
    FVector closestPointOnBox = boxPos + boxRot * localClosest;

    // Находим ближайшую точку на оси капсулы
    FVector capAxis = capTop - capBottom;
    float capLength = capAxis.Length ();
    FVector capDir = capAxis / capLength;
    FVector boxToCapBottom = closestPointOnBox - capBottom;
    float capT = boxToCapBottom.Dot ( capDir );
    capT = std::max ( 0.0f, std::min ( capT, capLength ) );
    FVector closestPointOnCapsule = capBottom + capDir * capT;

    // Проверяем расстояние
    FVector delta = closestPointOnBox - closestPointOnCapsule;
    float distance = delta.Length ();

    if (distance <= capRadius)
        {
        outInfo.ComponentA = box;
        outInfo.ComponentB = capsule;
        outInfo.Depth = capRadius - distance;

        if (distance > 0.001f)
            {
            outInfo.Normal = delta / distance;
            outInfo.Location = closestPointOnCapsule + outInfo.Normal * capRadius;
            }
        else
            {
            outInfo.Normal = FVector ( 1.0f, 0.0f, 0.0f );
            outInfo.Location = closestPointOnBox;
            }

        return true;
        }

    return false;
    }

bool CCollisionSystem::CheckCapsuleCapsule ( CBaseCollisionComponent * capA,
                                             CBaseCollisionComponent * capB,
                                             FCollisionInfo & outInfo ) const
    {
    CCapsuleComponent * capsuleA = dynamic_cast< CCapsuleComponent * >( capA );
    CCapsuleComponent * capsuleB = dynamic_cast< CCapsuleComponent * >( capB );

    if (!capsuleA || !capsuleB) return false;

    // Получаем центры полусфер для обеих капсул
    FVector aTop = capsuleA->GetTopSphereCenter ();
    FVector aBottom = capsuleA->GetBottomSphereCenter ();
    FVector bTop = capsuleB->GetTopSphereCenter ();
    FVector bBottom = capsuleB->GetBottomSphereCenter ();

    float aRadius = capsuleA->GetRadius ();
    float bRadius = capsuleB->GetRadius ();

    // Находим ближайшие точки между отрезками (осями капсул)
    FVector aAxis = aTop - aBottom;
    FVector bAxis = bTop - bBottom;

    float aLen = aAxis.Length ();
    float bLen = bAxis.Length ();

    if (aLen < 0.001f || bLen < 0.001f)
        {
            // Одна из капсул вырождена в сферу
        if (aLen < 0.001f)
            return CheckSphereCapsule ( capA, capB, outInfo );
        else
            return CheckSphereCapsule ( capB, capA, outInfo );
        }

    FVector aDir = aAxis / aLen;
    FVector bDir = bAxis / bLen;

    FVector aStart = aBottom;
    FVector bStart = bBottom;

    // Находим ближайшие точки между двумя отрезками
    FVector delta = bStart - aStart;
    float aDotB = aDir.Dot ( bDir );
    float aDotDelta = aDir.Dot ( delta );
    float bDotDelta = bDir.Dot ( delta );

    float tA, tB;
    float denom = 1.0f - aDotB * aDotB;

    if (std::abs ( denom ) < 0.001f)
        {
            // Отрезки параллельны
        tA = 0.0f;
        tB = bDotDelta;
        }
    else
        {
        tA = ( aDotDelta - aDotB * bDotDelta ) / denom;
        tB = ( aDotB * aDotDelta - bDotDelta ) / denom;
        }

        // Ограничиваем параметры длинами отрезков
    tA = std::max ( 0.0f, std::min ( tA, aLen ) );
    tB = std::max ( 0.0f, std::min ( tB, bLen ) );

    // Ближайшие точки
    FVector closestA = aStart + aDir * tA;
    FVector closestB = bStart + bDir * tB;

    // Проверяем расстояние
    FVector deltaAB = closestB - closestA;
    float distance = deltaAB.Length ();
    float radiusSum = aRadius + bRadius;

    if (distance <= radiusSum)
        {
        outInfo.ComponentA = capA;
        outInfo.ComponentB = capB;
        outInfo.Depth = radiusSum - distance;

        if (distance > 0.001f)
            {
            outInfo.Normal = deltaAB / distance;
            outInfo.Location = closestA + outInfo.Normal * aRadius;
            }
        else
            {
            outInfo.Normal = FVector ( 1.0f, 0.0f, 0.0f );
            outInfo.Location = closestA;
            }

        return true;
        }

    return false;
    }

    // ============================================================================
    // Sphere Collision Checks
    // ============================================================================

void CCollisionSystem::ProcessComponentPair ( const FCollisionInfo & collisionInfo )
    {
    CBaseCollisionComponent * compA = collisionInfo.ComponentA;
    CBaseCollisionComponent * compB = collisionInfo.ComponentB;

    if (!compA || !compB)
        return;

    auto pair = std::make_pair ( compA, compB );
    bool bWasColliding = ( m_PreviousFrameCollisions.find ( pair ) != m_PreviousFrameCollisions.end () );

    m_LastFrameCollisions++;
    m_CurrentFrameCollisions.insert ( pair );

    bool bShouldBlock = compA->ShouldBlockWith ( compB ) || compB->ShouldBlockWith ( compA );
    bool bShouldOverlap = compA->ShouldOverlapWith ( compB ) && compB->ShouldOverlapWith ( compA );

    if (bShouldBlock)
        {
        ResolveCollision ( collisionInfo );
        }

    if (bShouldOverlap && !bWasColliding)
        {
        compA->OnBeginOverlap ( compB );
        compB->OnBeginOverlap ( compA );

        FireCollisionEvent ( ECollisionEventType::BEGIN_OVERLAP, collisionInfo );
        }
    else if (bShouldBlock && !bWasColliding)
        {
        compA->OnHit ( compB );
        compB->OnHit ( compA );

        FireCollisionEvent ( ECollisionEventType::COLLISION_HIT, collisionInfo );
        }
    }

bool CCollisionSystem::CheckSphereSphere ( CBaseCollisionComponent * a,
                                           CBaseCollisionComponent * b,
                                           FCollisionInfo & outInfo ) const
    {
    if (!a || !b) return false;

    FVector posA = a->GetWorldLocation ();
    FVector posB = b->GetWorldLocation ();

    float radiusA = a->GetCollisionRadius ();
    float radiusB = b->GetCollisionRadius ();

    // Try to get exact radius from sphere component
    if (auto * sphereA = dynamic_cast< CSphereComponent * >( a ))
        radiusA = sphereA->GetRadius ();
    if (auto * sphereB = dynamic_cast< CSphereComponent * >( b ))
        radiusB = sphereB->GetRadius ();

    FVector delta = posB - posA;
    float distSq = delta.Dot ( delta );
    float radiusSum = radiusA + radiusB;

    if (distSq <= radiusSum * radiusSum)
        {
        float distance = std::sqrt ( distSq );
        outInfo.Depth = radiusSum - distance;

        if (distance > 0.001f)
            {
            outInfo.Normal = delta / distance;
            outInfo.Location = posA + outInfo.Normal * ( radiusA - outInfo.Depth * 0.5f );
            }
        else
            {
            outInfo.Normal = FVector ( 0.0f, 0.0f, 1.0f );
            outInfo.Location = posA;
            }

        return true;
        }

    return false;
    }

    // ============================================================================
    // Sphere-Box Collision Check
    // ============================================================================

bool CCollisionSystem::CheckSphereBox ( CBaseCollisionComponent * sphere,
                                        CBaseCollisionComponent * box,
                                        FCollisionInfo & outInfo ) const
    {
    if (!sphere || !box) return false;

    FVector spherePos = sphere->GetWorldLocation ();
    FVector boxPos = box->GetWorldLocation ();
    float sphereRadius = sphere->GetCollisionRadius ();

    // Get box half extents
    FVector boxHalfExtents;
    if (auto * boxComp = dynamic_cast< CBoxComponent * >( box ))
        {
        boxHalfExtents = boxComp->GetHalfExtents ();
        }
    else
        {
        FVector boundingBox = box->GetBoundingBox ();
        boxHalfExtents = boundingBox * 0.5f;
        }

        // Transform sphere position to box local space
    FQuat boxRot = box->GetOwnerActor ()->GetActorRotationQuat ();
    FVector localSpherePos = boxRot.Inverse () * ( spherePos - boxPos );

    // Find closest point in local space
    FVector localClosest;
    localClosest.x = std::max ( -boxHalfExtents.x, std::min ( localSpherePos.x, boxHalfExtents.x ) );
    localClosest.y = std::max ( -boxHalfExtents.y, std::min ( localSpherePos.y, boxHalfExtents.y ) );
    localClosest.z = std::max ( -boxHalfExtents.z, std::min ( localSpherePos.z, boxHalfExtents.z ) );

    // Transform back to world space
    FVector closestPoint = boxPos + boxRot * localClosest;

    // Check distance
    FVector delta = spherePos - closestPoint;
    float distSq = delta.Dot ( delta );

    if (distSq <= sphereRadius * sphereRadius)
        {
        float distance = std::sqrt ( distSq );
        outInfo.ComponentA = sphere;
        outInfo.ComponentB = box;
        outInfo.Depth = sphereRadius - distance;

        if (distance > 0.001f)
            {
            outInfo.Normal = delta / distance;
            outInfo.Location = closestPoint;
            }
        else
            {
                // Find closest face normal
            float minDist = sphereRadius;
            FVector normal ( 1.0f, 0.0f, 0.0f );

            float distToXMin = std::abs ( localSpherePos.x + boxHalfExtents.x );
            if (distToXMin < minDist) { minDist = distToXMin; normal = FVector ( -1.0f, 0.0f, 0.0f ); }

            float distToXMax = std::abs ( boxHalfExtents.x - localSpherePos.x );
            if (distToXMax < minDist) { minDist = distToXMax; normal = FVector ( 1.0f, 0.0f, 0.0f ); }

            float distToYMin = std::abs ( localSpherePos.y + boxHalfExtents.y );
            if (distToYMin < minDist) { minDist = distToYMin; normal = FVector ( 0.0f, -1.0f, 0.0f ); }

            float distToYMax = std::abs ( boxHalfExtents.y - localSpherePos.y );
            if (distToYMax < minDist) { minDist = distToYMax; normal = FVector ( 0.0f, 1.0f, 0.0f ); }

            float distToZMin = std::abs ( localSpherePos.z + boxHalfExtents.z );
            if (distToZMin < minDist) { minDist = distToZMin; normal = FVector ( 0.0f, 0.0f, -1.0f ); }

            float distToZMax = std::abs ( boxHalfExtents.z - localSpherePos.z );
            if (distToZMax < minDist) { normal = FVector ( 0.0f, 0.0f, 1.0f ); }

            outInfo.Normal = boxRot * normal;
            outInfo.Location = spherePos;
            }

        return true;
        }

    return false;
    }

    // ============================================================================
    // Box-Box Collision Checks
    // ============================================================================

bool CCollisionSystem::CheckAABBAABB ( const FVector & posA, const FVector & halfA,
                                       const FVector & posB, const FVector & halfB,
                                       FCollisionInfo & outInfo,
                                       CBaseCollisionComponent * compA,
                                       CBaseCollisionComponent * compB ) const
    {
    FVector delta = posB - posA;

    float overlapX = halfA.x + halfB.x - std::abs ( delta.x );
    float overlapY = halfA.y + halfB.y - std::abs ( delta.y );
    float overlapZ = halfA.z + halfB.z - std::abs ( delta.z );

    if (overlapX > 0 && overlapY > 0 && overlapZ > 0)
        {
            // Find smallest overlap
        float minOverlap = overlapX;
        FVector normal ( 1.0f, 0.0f, 0.0f );

        if (overlapY < minOverlap)
            {
            minOverlap = overlapY;
            normal = FVector ( 0.0f, 1.0f, 0.0f );
            }
        if (overlapZ < minOverlap)
            {
            minOverlap = overlapZ;
            normal = FVector ( 0.0f, 0.0f, 1.0f );
            }

            // Set normal direction
        if (delta.x > 0 && normal.x != 0) normal.x *= -1;
        if (delta.y > 0 && normal.y != 0) normal.y *= -1;
        if (delta.z > 0 && normal.z != 0) normal.z *= -1;

        outInfo.ComponentA = compA;
        outInfo.ComponentB = compB;
        outInfo.Depth = minOverlap;
        outInfo.Normal = normal;
        outInfo.Location = posA + normal * ( halfA.x - minOverlap * 0.5f );

        return true;
        }

    return false;
    }

bool CCollisionSystem::CheckBoxBox ( CBaseCollisionComponent * a,
                                     CBaseCollisionComponent * b,
                                     FCollisionInfo & outInfo ) const
    {
    if (!a || !b) return false;

    CBoxComponent * boxA = dynamic_cast< CBoxComponent * >( a );
    CBoxComponent * boxB = dynamic_cast< CBoxComponent * >( b );

    if (!boxA || !boxB) return false;

    FVector posA = a->GetWorldLocation ();
    FVector posB = b->GetWorldLocation ();
    FVector halfA = boxA->GetHalfExtents ();
    FVector halfB = boxB->GetHalfExtents ();

    FQuat rotA = a->GetOwnerActor ()->GetActorRotationQuat ();
    FQuat rotB = b->GetOwnerActor ()->GetActorRotationQuat ();

    // If both boxes are axis-aligned, use fast AABB check
    if (rotA.IsIdentity () && rotB.IsIdentity ())
        {
        return CheckAABBAABB ( posA, halfA, posB, halfB, outInfo, a, b );
        }

        // Otherwise use full OBB check
    return CheckOBBOBB ( boxA, boxB, outInfo );
    }

bool CCollisionSystem::CheckOBBOBB ( CBoxComponent * boxA, CBoxComponent * boxB, FCollisionInfo & outInfo ) const
    {
    FVector posA = boxA->GetWorldLocation ();
    FVector posB = boxB->GetWorldLocation ();
    FVector halfA = boxA->GetHalfExtents ();
    FVector halfB = boxB->GetHalfExtents ();

    FQuat rotA = boxA->GetOwnerActor ()->GetActorRotationQuat ();
    FQuat rotB = boxB->GetOwnerActor ()->GetActorRotationQuat ();

    // Get axes in world space
    FVector axesA[ 3 ] = {
        rotA * FVector ( 1.0f, 0.0f, 0.0f ),
        rotA * FVector ( 0.0f, 1.0f, 0.0f ),
        rotA * FVector ( 0.0f, 0.0f, 1.0f )
        };

    FVector axesB[ 3 ] = {
        rotB * FVector ( 1.0f, 0.0f, 0.0f ),
        rotB * FVector ( 0.0f, 1.0f, 0.0f ),
        rotB * FVector ( 0.0f, 0.0f, 1.0f )
        };

        // All 15 axes to test (3 from A, 3 from B, 9 cross products)
    std::vector<FVector> testAxes;

    // Add axes from A and B
    for (int i = 0; i < 3; i++)
        {
        testAxes.push_back ( axesA[ i ] );
        testAxes.push_back ( axesB[ i ] );
        }

        // Add cross products
    for (int i = 0; i < 3; i++)
        {
        for (int j = 0; j < 3; j++)
            {
            FVector cross = axesA[ i ].Cross ( axesB[ j ] );
            if (!cross.IsZero ())
                {
                testAxes.push_back ( cross.Normalized () );
                }
            }
        }

    FVector delta = posB - posA;
    float minOverlap = std::numeric_limits<float>::max ();
    FVector minAxis;

    // Test each axis
    for (const FVector & axis : testAxes)
        {
        if (axis.IsZero ()) continue;

        // Project box A onto axis
        float projA = std::abs ( halfA.x * std::abs ( axis.Dot ( axesA[ 0 ] ) ) ) +
            std::abs ( halfA.y * std::abs ( axis.Dot ( axesA[ 1 ] ) ) ) +
            std::abs ( halfA.z * std::abs ( axis.Dot ( axesA[ 2 ] ) ) );

        // Project box B onto axis
        float projB = std::abs ( halfB.x * std::abs ( axis.Dot ( axesB[ 0 ] ) ) ) +
            std::abs ( halfB.y * std::abs ( axis.Dot ( axesB[ 1 ] ) ) ) +
            std::abs ( halfB.z * std::abs ( axis.Dot ( axesB[ 2 ] ) ) );

        float centerDist = std::abs ( delta.Dot ( axis ) );

        if (centerDist > projA + projB)
            {
                // Separating axis found - no collision
            return false;
            }

        float overlap = ( projA + projB ) - centerDist;
        if (overlap < minOverlap)
            {
            minOverlap = overlap;
            minAxis = axis;
            }
        }

        // Collision detected
    outInfo.ComponentA = boxA;
    outInfo.ComponentB = boxB;
    outInfo.Depth = minOverlap;
    outInfo.Normal = minAxis * ( delta.Dot ( minAxis ) > 0 ? -1.0f : 1.0f );

    // Calculate contact point (simplified - midpoint along normal)
    outInfo.Location = posA + outInfo.Normal * ( halfA.Length () - minOverlap * 0.5f );

    return true;
    }

    // ============================================================================
    // Collision Resolution
    // ============================================================================
void CCollisionSystem::ResolveCollision ( const FCollisionInfo & collision )
    {
    if (!collision.ComponentA || !collision.ComponentB)
        return;

    CActor * actorA = collision.ComponentA->GetOwnerActor ();
    CActor * actorB = collision.ComponentB->GetOwnerActor ();

    if (!actorA || !actorB)
        return;

    bool bIsAStatic = ( collision.ComponentA->GetCollisionChannel ().GetName () == "WorldStatic" );
    bool bIsBStatic = ( collision.ComponentB->GetCollisionChannel ().GetName () == "WorldStatic" );

    FVector push = collision.Normal * collision.Depth;

    // Только корректируем позицию, НЕ обнуляем скорость!
    if (bIsAStatic && !bIsBStatic)
        {
        actorB->SetActorLocation ( actorB->GetActorLocation () + push );
        // НЕ вызываем обнуление скорости здесь!
        }
    else if (!bIsAStatic && bIsBStatic)
        {
        actorA->SetActorLocation ( actorA->GetActorLocation () - push );
        // НЕ вызываем обнуление скорости здесь!
        }
    else
        {
        FVector halfPush = push * 0.5f;
        actorA->SetActorLocation ( actorA->GetActorLocation () - halfPush );
        actorB->SetActorLocation ( actorB->GetActorLocation () + halfPush );
        }
    }

    // ============================================================================
    // Event System
    // ============================================================================

void CCollisionSystem::FireCollisionEvent ( ECollisionEventType eventType, const FCollisionInfo & info )
    {
    auto it = m_Callbacks.find ( eventType );
    if (it != m_Callbacks.end ())
        {
        it->second ( info );
        }
    }

void CCollisionSystem::ProcessCollisionsSpatial ()
    {
        // Не обрабатываем во время выключения
    if (bIsGlobalShuttingDown || bIsAlreadyShuttingDown)
        return;

    // Обновляем пространственное разделение если нужно
    static float gridUpdateTimer = 0.0f;
    gridUpdateTimer += m_AccumulatedTime;

    if (gridUpdateTimer >= 0.1f) // Обновляем сетку 10 раз в секунду
        {
        UpdateSpatialPartition ();
        gridUpdateTimer = 0.0f;
        }

        // Множество для отслеживания обработанных пар
    std::set<std::pair<CBaseCollisionComponent *, CBaseCollisionComponent *>> processedPairs;

    for (CBaseCollisionComponent * compA : m_CollisionComponents)
        {
        if (!IsValidCollisionComponent ( compA ))
            continue;

        // Получаем потенциально сталкивающиеся компоненты
        auto potentialCollisions = GetPotentiallyCollidingComponents ( compA );

        for (CBaseCollisionComponent * compB : potentialCollisions)
            {
            if (!IsValidCollisionComponent ( compB ))
                continue;

            // Убеждаемся что обрабатываем каждую пару только один раз
            auto pair = std::make_pair ( compA, compB );
            auto reversePair = std::make_pair ( compB, compA );

            if (processedPairs.find ( pair ) != processedPairs.end () ||
                 processedPairs.find ( reversePair ) != processedPairs.end ())
                {
                continue;
                }

                // Быстрая проверка каналов
            if (!compA->CanCollideWith ( compB ) || !compB->CanCollideWith ( compA ))
                continue;
            FCollisionInfo Info {};
            Info.ComponentA = compA;
            Info.ComponentB = compB;
            // Проверяем коллизию
            if (compA->CheckCollision ( compB, Info ))
                {
                processedPairs.insert ( pair );
                ProcessComponentPair ( Info );
                }
            }
        }
    }

void CCollisionSystem::RegisterCollisionCallback ( ECollisionEventType eventType,
                                                   const FCollisionCallback & callback )
    {
    m_Callbacks[ eventType ] = callback;
    }

void CCollisionSystem::UnregisterCollisionCallback ( ECollisionEventType eventType )
    {
    m_Callbacks.erase ( eventType );
    }

    // ============================================================================
    // Raycasting
    // ============================================================================

FRaycastResult CCollisionSystem::Raycast ( const FVector & start,
                                           const FVector & end,
                                           const std::string & channelName ) const
    {
    FVector direction = end - start;
    float distance = direction.Length ();

    if (distance < 0.001f)
        return FRaycastResult ();

    direction = direction / distance;
    return Raycast ( start, direction, distance, channelName );
    }

FRaycastResult CCollisionSystem::Raycast ( const FVector & start,
                                           const FVector & direction,
                                           float distance,
                                           const std::string & channelName ) const
    {
    FRaycastResult result;
    float closestDistance = std::numeric_limits<float>::max ();

    for (CBaseCollisionComponent * component : m_CollisionComponents)
        {
        if (!component || !component->IsCollisionEnabled ())
            continue;

        if (channelName != "All" && !component->CanCollideWith ( channelName ))
            continue;

        if (!component->GetOwnerActor ())
            continue;

        // Для террейна используем специальную проверку
        if (component->GetShapeType () == ECollisionShape::TERRAIN)
            {
            FVector hitPoint, normal;
            float hitDist;

            if (CheckRayTerrain ( start, direction, distance, component, hitPoint, normal, hitDist ))
                {
                if (hitDist < closestDistance)
                    {
                    closestDistance = hitDist;
                    result.bHit = true;
                    result.HitComponent = component;
                    result.Location = hitPoint;
                    result.Normal = normal;
                    result.Distance = hitDist;
                    }
                }
            continue;
            }

            // Для остальных компонентов - существующая проверка
        FVector compPos = component->GetOwnerActor ()->GetActorLocation ();
        float radius = component->GetCollisionRadius ();

        FVector toSphere = compPos - start;
        float projection = toSphere.Dot ( direction );

        if (projection < 0.0f || projection > distance)
            continue;

        FVector closestPoint = start + direction * projection;
        FVector toClosest = closestPoint - compPos;
        float distSq = toClosest.Dot ( toClosest );

        if (distSq <= radius * radius)
            {
            float hitDistance = projection - std::sqrt ( radius * radius - distSq );

            if (hitDistance >= 0.0f && hitDistance < closestDistance)
                {
                closestDistance = hitDistance;
                result.bHit = true;
                result.HitComponent = component;
                result.Location = start + direction * hitDistance;
                result.Normal = ( result.Location - compPos ).Normalized ();
                result.Distance = hitDistance;
                }
            }
        }

    return result;
    }

    // ============================================================================
    // Overlap Tests
    // ============================================================================

std::vector<CBaseCollisionComponent *> CCollisionSystem::SphereOverlap (
    const FVector & center, float radius, const std::string & channelName ) const
    {
    std::vector<CBaseCollisionComponent *> results;

    for (CBaseCollisionComponent * component : m_CollisionComponents)
        {
        if (!component || !component->IsCollisionEnabled ())
            continue;

        if (channelName != "All" && !component->CanCollideWith ( channelName ))
            continue;

        if (!component->GetOwnerActor ())
            continue;

        FVector compPos = component->GetOwnerActor ()->GetActorLocation ();
        float compRadius = component->GetCollisionRadius ();

        FVector delta = compPos - center;
        float distSq = delta.Dot ( delta );
        float radiusSum = radius + compRadius;

        if (distSq <= radiusSum * radiusSum)
            {
            results.push_back ( component );
            }
        }

    return results;
    }

std::vector<CBaseCollisionComponent *> CCollisionSystem::BoxOverlap (
    const FVector & center, const FVector & halfExtents, const FVector & rotation,
    const std::string & channelName ) const
    {
        // Simple implementation - convert to sphere check
        // TODO: Implement proper Box-Box overlap test
    float sphereRadius = std::max ( halfExtents.x, std::max ( halfExtents.y, halfExtents.z ) );
    return SphereOverlap ( center, sphereRadius, channelName );
    }

    // ============================================================================
    // Manual Collision Checks
    // ============================================================================

std::vector<FCollisionInfo> CCollisionSystem::CheckCollisions (
    CBaseCollisionComponent * component ) const
    {
    std::vector<FCollisionInfo> collisions;

    if (!component || !component->IsCollisionEnabled ())
        return collisions;

    for (CBaseCollisionComponent * other : m_CollisionComponents)
        {
        if (!other || other == component || !other->IsCollisionEnabled ())
            continue;

        if (!component->CanCollideWith ( other ))
            continue;

        FCollisionInfo info;
        info.ComponentA = component;
        info.ComponentB = other;

        if (component->GetOwnerActor () && other->GetOwnerActor ())
            {
            FVector posA = component->GetOwnerActor ()->GetActorLocation ();
            FVector posB = other->GetOwnerActor ()->GetActorLocation ();

            float radiusA = component->GetCollisionRadius ();
            float radiusB = other->GetCollisionRadius ();
            float totalRadius = radiusA + radiusB;

            FVector delta = posB - posA;
            float distSq = delta.Dot ( delta );
            float totalRadiusSq = totalRadius * totalRadius;

            if (distSq <= totalRadiusSq)
                {
                float distance = std::sqrt ( distSq );

                if (distance > 0.001f)
                    {
                    info.Normal = delta / distance;
                    info.Depth = totalRadius - distance;
                    info.Location = posA + info.Normal * ( radiusA - info.Depth * 0.5f );
                    }
                else
                    {
                    info.Normal = FVector ( 0.0f, 0.0f, 1.0f );
                    info.Depth = totalRadius;
                    info.Location = posA;
                    }

                collisions.push_back ( info );
                }
            }
        }

    return collisions;
    }

std::vector<FCollisionInfo> CCollisionSystem::CheckCollisionsAtLocation (
    const FVector & location, float radius ) const
    {
    std::vector<FCollisionInfo> collisions;
    auto components = SphereOverlap ( location, radius, "All" );

    for (CBaseCollisionComponent * comp : components)
        {
        if (!comp || !comp->GetOwnerActor ())
            continue;

        FCollisionInfo info;
        info.ComponentA = nullptr; // Mark as area check
        info.ComponentB = comp;
        info.Location = comp->GetOwnerActor ()->GetActorLocation ();
        info.Normal = ( info.Location - location ).Normalized ();
        info.Depth = radius - ( info.Location - location ).Length ();

        collisions.push_back ( info );
        }

    return collisions;
    }

    // ============================================================================
    // Spatial Partitioning
    // ============================================================================

void CCollisionSystem::UpdateSpatialPartition ()
    {
    if (bIsGlobalShuttingDown || bIsAlreadyShuttingDown)
        return;

    m_SpatialGrid.clear ();

    for (CBaseCollisionComponent * component : m_CollisionComponents)
        {
        if (!component || !component->GetOwnerActor ())
            continue;

        FVector position = component->GetOwnerActor ()->GetActorLocation ();

        int cellX = static_cast< int >( position.x / m_CellSize );
        int cellY = static_cast< int >( position.y / m_CellSize );
        int cellZ = static_cast< int >( position.z / m_CellSize );

        int64_t cellKey = ( static_cast< int64_t >( cellX ) << 42 ) |
            ( static_cast< int64_t >( cellY ) << 21 ) |
            static_cast< int64_t >( cellZ );

        m_SpatialGrid[ cellKey ].Components.push_back ( component );
        }
    }

float CCollisionSystem::GetComponentBoundingRadius ( CBaseCollisionComponent * component ) const
    {
    if (!component) return 0.0f;

    switch (component->GetShapeType ())
        {
            case ECollisionShape::SPHERE:
                return component->GetCollisionRadius ();

            case ECollisionShape::BOX:
                if (CBoxComponent * box = dynamic_cast< CBoxComponent * >( component ))
                    {
                    FVector half = box->GetHalfExtents ();
                    return half.Length ();
                    }
                break;

            case ECollisionShape::CAPSULE:
                if (CCapsuleComponent * cap = dynamic_cast< CCapsuleComponent * >( component ))
                    {
                    return cap->GetRadius () + cap->GetHalfHeight ();
                    }
                break;

            case ECollisionShape::TERRAIN:
                if (CTerrainComponent * terr = dynamic_cast< CTerrainComponent * >( component ))
                    {
                    FVector box = terr->GetBoundingBox ();
                    return box.Length () * 0.5f; // Половина диагонали
                    }
                break;

            default:
                break;
        }

    return 100.0f; // Значение по умолчанию
    }

std::vector<CBaseCollisionComponent *> CCollisionSystem::GetPotentiallyCollidingComponents (
    CBaseCollisionComponent * component ) const
    {
    std::vector<CBaseCollisionComponent *> result;

    if (!component || !component->GetOwnerActor ())
        return result;

    FVector position = component->GetOwnerActor ()->GetActorLocation ();
    float radius = GetComponentBoundingRadius ( component );

    int minCellX = static_cast< int >( ( position.x - radius ) / m_CellSize );
    int maxCellX = static_cast< int >( ( position.x + radius ) / m_CellSize );
    int minCellY = static_cast< int >( ( position.y - radius ) / m_CellSize );
    int maxCellY = static_cast< int >( ( position.y + radius ) / m_CellSize );
    int minCellZ = static_cast< int >( ( position.z - radius ) / m_CellSize );
    int maxCellZ = static_cast< int >( ( position.z + radius ) / m_CellSize );

    for (int x = minCellX; x <= maxCellX; ++x)
        {
        for (int y = minCellY; y <= maxCellY; ++y)
            {
            for (int z = minCellZ; z <= maxCellZ; ++z)
                {
                int64_t cellKey = ( static_cast< int64_t >( x ) << 42 ) |
                    ( static_cast< int64_t >( y ) << 21 ) |
                    static_cast< int64_t >( z );

                auto it = m_SpatialGrid.find ( cellKey );
                if (it != m_SpatialGrid.end ())
                    {
                    for (CBaseCollisionComponent * other : it->second.Components)
                        {
                        if (other != component)
                            {
                                // Дополнительная проверка расстояния
                            FVector otherPos = other->GetOwnerActor ()->GetActorLocation ();
                            float distSq = ( otherPos - position ).LengthSquared ();
                            float maxDist = radius + GetComponentBoundingRadius ( other );

                            if (distSq <= maxDist * maxDist)
                                {
                                result.push_back ( other );
                                }
                            }
                        }
                    }
                }
            }
        }

    return result;
    }

    // ============================================================================
    // Terrain Collision Checks
    // ============================================================================

bool CCollisionSystem::CheckSphereTerrain ( CBaseCollisionComponent * sphere,
                                            CBaseCollisionComponent * terrain,
                                            FCollisionInfo & outInfo ) const
    {
    CTerrainComponent * terr = dynamic_cast< CTerrainComponent * >( terrain );
    if (!terr) return false;

    FVector spherePos = sphere->GetWorldLocation ();
    float sphereRadius = sphere->GetCollisionRadius ();

    // Получаем высоту террейна под центром сферы
    float terrainHeight = terr->GetHeightAtWorld ( spherePos );

    // Проверяем, касается ли сфера террейна
    if (spherePos.y - sphereRadius <= terrainHeight)
        {
        outInfo.ComponentA = sphere;
        outInfo.ComponentB = terrain;
        outInfo.Depth = ( terrainHeight - ( spherePos.y - sphereRadius ) );
        outInfo.Normal = FVector ( 0.0f, 1.0f, 0.0f ); // Нормаль вверх
        outInfo.Location = FVector ( spherePos.x, terrainHeight, spherePos.z );

        return true;
        }

    return false;
    }

bool CCollisionSystem::CheckBoxTerrain ( CBaseCollisionComponent * box,
                                         CBaseCollisionComponent * terrain,
                                         FCollisionInfo & outInfo ) const
    {
    CBoxComponent * boxComp = dynamic_cast< CBoxComponent * >( box );
    CTerrainComponent * terr = dynamic_cast< CTerrainComponent * >( terrain );

    if (!boxComp || !terr) return false;

    FVector boxPos = box->GetWorldLocation ();
    FVector half = boxComp->GetHalfExtents ();

    // Находим самую нижнюю точку бокса (с учётом вращения)
    FQuat boxRot = box->GetOwnerActor ()->GetActorRotationQuat ();

    // 8 углов бокса
    FVector corners[ 8 ] = {
        boxRot * FVector ( -half.x, -half.y, -half.z ) + boxPos,
        boxRot * FVector ( half.x, -half.y, -half.z ) + boxPos,
        boxRot * FVector ( -half.x,  half.y, -half.z ) + boxPos,
        boxRot * FVector ( half.x,  half.y, -half.z ) + boxPos,
        boxRot * FVector ( -half.x, -half.y,  half.z ) + boxPos,
        boxRot * FVector ( half.x, -half.y,  half.z ) + boxPos,
        boxRot * FVector ( -half.x,  half.y,  half.z ) + boxPos,
        boxRot * FVector ( half.x,  half.y,  half.z ) + boxPos
        };

        // Находим самую нижнюю точку
    float minY = corners[ 0 ].y;
    for (int i = 1; i < 8; i++)
        {
        if (corners[ i ].y < minY)
            minY = corners[ i ].y;
        }

        // Получаем высоту террейна под центром бокса
    float terrainHeight = terr->GetHeightAtWorld ( boxPos );

    // Проверяем, касается ли бокс террейна
    if (minY <= terrainHeight)
        {
        outInfo.ComponentA = box;
        outInfo.ComponentB = terrain;
        outInfo.Depth = terrainHeight - minY;
        outInfo.Normal = FVector ( 0.0f, 1.0f, 0.0f ); // Та же нормаль, что и у сферы
        outInfo.Location = FVector ( boxPos.x, terrainHeight, boxPos.z );

        return true;
        }

    return false;
    }

bool CCollisionSystem::CheckCapsuleTerrain ( CBaseCollisionComponent * capsule,
                                             CBaseCollisionComponent * terrain,
                                             FCollisionInfo & outInfo ) const
    {
    CCapsuleComponent * cap = dynamic_cast< CCapsuleComponent * >( capsule );
    CTerrainComponent * terr = dynamic_cast< CTerrainComponent * >( terrain );

    if (!cap || !terr) return false;

    // Получаем центры полусфер капсулы
    FVector topCenter = cap->GetTopSphereCenter ();
    FVector bottomCenter = cap->GetBottomSphereCenter ();
    float radius = cap->GetRadius ();

    // Проверяем нижнюю полусферу (самая нижняя точка)
    float lowestPoint = bottomCenter.y - radius;

    // Получаем высоту террейна под центром капсулы
    FVector capsulePos = capsule->GetWorldLocation ();
    float terrainHeight = terr->GetHeightAtWorld ( capsulePos );

    // Проверяем, касается ли капсула террейна
    if (lowestPoint <= terrainHeight)
        {
        outInfo.ComponentA = capsule;
        outInfo.ComponentB = terrain;
        outInfo.Depth = terrainHeight - lowestPoint;
        outInfo.Normal = FVector ( 0.0f, 1.0f, 0.0f ); // Та же нормаль, что и у сферы
        outInfo.Location = FVector ( capsulePos.x, terrainHeight, capsulePos.z );

        return true;
        }

    return false;
    }

bool CCollisionSystem::CheckRayTerrain ( const FVector & start, const FVector & direction, float maxDistance,
                                         CBaseCollisionComponent * terrain,
                                         FVector & outHit, FVector & outNormal, float & outDist ) const
    {
    CTerrainComponent * terr = dynamic_cast< CTerrainComponent * >( terrain );
    if (!terr) return false;

    // Простой DDA (Digital Differential Analyzer) алгоритм для рейкаста по террейну
    float step = terr->GetTerrainData ().CellSize * 0.5f;
    FVector current = start;
    float traveled = 0.0f;

    while (traveled < maxDistance)
        {
        float terrainY = terr->GetHeightAtWorld ( current );

        if (current.y <= terrainY)
            {
            outHit = current;
            outNormal = FVector ( 0.0f, 1.0f, 0.0f );
            outDist = traveled;
            return true;
            }

        current += direction * step;
        traveled += step;
        }

    return false;
    }

    // ============================================================================
    // Cylinder Collision Checks
    // ============================================================================

bool CCollisionSystem::CheckSphereCylinder ( CBaseCollisionComponent * sphere,
                                             CBaseCollisionComponent * cylinder,
                                             FCollisionInfo & outInfo ) const
    {
    CSphereComponent * sphereComp = dynamic_cast< CSphereComponent * >( sphere );
    CCylinderComponent * cylComp = dynamic_cast< CCylinderComponent * >( cylinder );

    if (!sphereComp || !cylComp) return false;

    FVector spherePos = sphere->GetWorldLocation ();
    float sphereRadius = sphereComp->GetRadius ();

    FVector cylPos = cylinder->GetWorldLocation ();
    float cylRadius = cylComp->GetRadius ();
    float cylHalfHeight = cylComp->GetHalfHeight ();
    FQuat cylRot = cylinder->GetOwnerActor ()->GetActorRotationQuat ();

    // Преобразуем позицию сферы в локальное пространство цилиндра
    FVector localSpherePos = cylRot.Inverse () * ( spherePos - cylPos );

    // Проецируем на плоскость XZ (основания цилиндра)
    float distXZ = std::sqrt ( localSpherePos.x * localSpherePos.x + localSpherePos.z * localSpherePos.z );

    // Проверка по высоте (Y)
    if (std::abs ( localSpherePos.y ) > cylHalfHeight + sphereRadius)
        return false;

    // Проверка по радиусу
    if (distXZ > cylRadius + sphereRadius)
        return false;

    // Если внутри цилиндра по всем осям - есть коллизия
    // Находим ближайшую точку на поверхности цилиндра
    FVector localClosestPoint = localSpherePos;

    // Ограничиваем по высоте
    if (std::abs ( localClosestPoint.y ) > cylHalfHeight)
        localClosestPoint.y = ( localClosestPoint.y > 0 ) ? cylHalfHeight : -cylHalfHeight;

    // Ограничиваем по радиусу
    float currentDistXZ = std::sqrt ( localClosestPoint.x * localClosestPoint.x + localClosestPoint.z * localClosestPoint.z );
    if (currentDistXZ > cylRadius)
        {
        float scale = cylRadius / currentDistXZ;
        localClosestPoint.x *= scale;
        localClosestPoint.z *= scale;
        }

        // Вычисляем глубину проникновения
    FVector delta = localSpherePos - localClosestPoint;
    float distance = delta.Length ();

    outInfo.ComponentA = sphere;
    outInfo.ComponentB = cylinder;
    outInfo.Depth = sphereRadius - distance;

    if (distance > 0.001f)
        {
        outInfo.Normal = ( cylRot * delta ).Normalized ();
        outInfo.Location = spherePos - outInfo.Normal * sphereRadius;
        }
    else
        {
        outInfo.Normal = FVector ( 0.0f, 1.0f, 0.0f );
        outInfo.Location = spherePos;
        }

    return true;
    }

bool CCollisionSystem::CheckBoxCylinder ( CBaseCollisionComponent * box,
                                          CBaseCollisionComponent * cylinder,
                                          FCollisionInfo & outInfo ) const
    {
    CBoxComponent * boxComp = dynamic_cast< CBoxComponent * >( box );
    CCylinderComponent * cylComp = dynamic_cast< CCylinderComponent * >( cylinder );

    if (!boxComp || !cylComp) return false;

    FVector boxPos = box->GetWorldLocation ();
    FVector boxHalf = boxComp->GetHalfExtents ();
    FQuat boxRot = box->GetOwnerActor ()->GetActorRotationQuat ();

    FVector cylPos = cylinder->GetWorldLocation ();
    float cylRadius = cylComp->GetRadius ();
    float cylHalfHeight = cylComp->GetHalfHeight ();
    FQuat cylRot = cylinder->GetOwnerActor ()->GetActorRotationQuat ();

    // Преобразуем позицию бокса в локальное пространство цилиндра
    FVector localBoxPos = cylRot.Inverse () * ( boxPos - cylPos );

    // Трансформируем половины размеров бокса в локальное пространство цилиндра
    FVector localHalf;

    // Получаем оси бокса в мировом пространстве
    FVector boxAxisX = boxRot * FVector ( 1, 0, 0 );
    FVector boxAxisY = boxRot * FVector ( 0, 1, 0 );
    FVector boxAxisZ = boxRot * FVector ( 0, 0, 1 );

    // Получаем оси цилиндра в мировом пространстве
    FVector cylAxisX = cylRot * FVector ( 1, 0, 0 );
    FVector cylAxisY = cylRot * FVector ( 0, 1, 0 );
    FVector cylAxisZ = cylRot * FVector ( 0, 0, 1 );

    // Проецируем половины размеров бокса на оси цилиндра
    localHalf.x = std::abs ( boxAxisX.Dot ( cylAxisX ) ) * boxHalf.x +
        std::abs ( boxAxisY.Dot ( cylAxisX ) ) * boxHalf.y +
        std::abs ( boxAxisZ.Dot ( cylAxisX ) ) * boxHalf.z;

    localHalf.y = std::abs ( boxAxisX.Dot ( cylAxisY ) ) * boxHalf.x +
        std::abs ( boxAxisY.Dot ( cylAxisY ) ) * boxHalf.y +
        std::abs ( boxAxisZ.Dot ( cylAxisY ) ) * boxHalf.z;

    localHalf.z = std::abs ( boxAxisX.Dot ( cylAxisZ ) ) * boxHalf.x +
        std::abs ( boxAxisY.Dot ( cylAxisZ ) ) * boxHalf.y +
        std::abs ( boxAxisZ.Dot ( cylAxisZ ) ) * boxHalf.z;

    // Проверка по высоте (Y)
    if (std::abs ( localBoxPos.y ) > cylHalfHeight + localHalf.y)
        return false;

    // Проверка по радиусу - находим ближайшую точку бокса к оси цилиндра
    float minX = localBoxPos.x - localHalf.x;
    float maxX = localBoxPos.x + localHalf.x;
    float minZ = localBoxPos.z - localHalf.z;
    float maxZ = localBoxPos.z + localHalf.z;

    // Находим ближайшую точку к оси цилиндра (0,0)
    float closestX = 0.0f;
    if (localBoxPos.x > 0)
        closestX = std::max ( 0.0f, minX );
    else
        closestX = std::min ( 0.0f, maxX );

    float closestZ = 0.0f;
    if (localBoxPos.z > 0)
        closestZ = std::max ( 0.0f, minZ );
    else
        closestZ = std::min ( 0.0f, maxZ );

    float distToAxis = std::sqrt ( closestX * closestX + closestZ * closestZ );

    if (distToAxis > cylRadius)
        return false;

    // Есть коллизия
    outInfo.ComponentA = box;
    outInfo.ComponentB = cylinder;

    // Упрощённая нормаль (в направлении от оси цилиндра)
    if (distToAxis > 0.001f)
        {
        outInfo.Normal = ( cylRot * FVector ( closestX, 0.0f, closestZ ) ).Normalized ();
        }
    else
        {
        outInfo.Normal = FVector ( 1.0f, 0.0f, 0.0f );
        }

    outInfo.Depth = cylRadius - distToAxis;
    outInfo.Location = boxPos + outInfo.Normal * ( boxHalf.Length () - outInfo.Depth * 0.5f );

    return true;
    }

bool CCollisionSystem::CheckCapsuleCylinder ( CBaseCollisionComponent * capsule,
                                              CBaseCollisionComponent * cylinder,
                                              FCollisionInfo & outInfo ) const
    {
    CCapsuleComponent * capComp = dynamic_cast< CCapsuleComponent * >( capsule );
    CCylinderComponent * cylComp = dynamic_cast< CCylinderComponent * >( cylinder );

    if (!capComp || !cylComp) return false;

    FVector capPos = capsule->GetWorldLocation ();
    float capRadius = capComp->GetRadius ();
    float capHalfHeight = capComp->GetHalfHeight ();
    FQuat capRot = capsule->GetOwnerActor ()->GetActorRotationQuat ();

    FVector cylPos = cylinder->GetWorldLocation ();
    float cylRadius = cylComp->GetRadius ();
    float cylHalfHeight = cylComp->GetHalfHeight ();
    FQuat cylRot = cylinder->GetOwnerActor ()->GetActorRotationQuat ();

    // Получаем центры полусфер капсулы
    FVector capTop = capRot * FVector ( 0, 0, capHalfHeight ) + capPos;
    FVector capBottom = capRot * FVector ( 0, 0, -capHalfHeight ) + capPos;

    // Преобразуем в локальное пространство цилиндра
    FVector localCapTop = cylRot.Inverse () * ( capTop - cylPos );
    FVector localCapBottom = cylRot.Inverse () * ( capBottom - cylPos );
    FVector localCapAxis = localCapTop - localCapBottom;
    float capLength = localCapAxis.Length ();
    FVector localCapDir = localCapAxis / capLength;

    // Ось цилиндра в локальном пространстве - Y
    FVector cylAxis ( 0, 1, 0 );

    // Находим ближайшие точки между отрезками
    FVector w = localCapBottom - FVector ( 0, -cylHalfHeight, 0 );
    float a = 1.0f; // Длина оси цилиндра нормирована
    float b = localCapDir.Dot ( localCapDir );
    float c = localCapDir.Dot ( cylAxis );
    float d = cylAxis.Dot ( w );
    float e = localCapDir.Dot ( w );

    float t, s;
    float denom = a * b - c * c;

    if (std::abs ( denom ) < 0.001f)
        {
        t = 0.0f;
        s = e / b;
        }
    else
        {
        t = ( c * e - b * d ) / denom;
        s = ( a * e - c * d ) / denom;
        }

        // Ограничиваем параметры
    t = std::max ( -cylHalfHeight, std::min ( cylHalfHeight, t ) );
    s = std::max ( 0.0f, std::min ( capLength, s ) );

    // Ближайшие точки
    FVector closestCyl = FVector ( 0, t, 0 );
    FVector closestCap = localCapBottom + localCapDir * s;

    // Проверяем расстояние в плоскости XZ
    FVector delta = closestCap - closestCyl;
    float distXZ = std::sqrt ( delta.x * delta.x + delta.z * delta.z );
    float distY = std::abs ( delta.y );

    if (distXZ > cylRadius + capRadius)
        return false;

    // Есть коллизия
    outInfo.ComponentA = capsule;
    outInfo.ComponentB = cylinder;

    if (distXZ > 0.001f)
        {
        FVector normalXZ = FVector ( delta.x, 0, delta.z ).Normalized ();
        outInfo.Normal = cylRot * normalXZ;
        }
    else
        {
        outInfo.Normal = FVector ( 1.0f, 0.0f, 0.0f );
        }

    outInfo.Depth = cylRadius + capRadius - distXZ;
    outInfo.Location = ( capTop + capBottom ) * 0.5f + outInfo.Normal * ( capRadius - outInfo.Depth * 0.5f );

    return true;
    }

bool CCollisionSystem::CheckCylinderCylinder ( CBaseCollisionComponent * cylA,
                                               CBaseCollisionComponent * cylB,
                                               FCollisionInfo & outInfo ) const
    {
    CCylinderComponent * cylAComp = dynamic_cast< CCylinderComponent * >( cylA );
    CCylinderComponent * cylBComp = dynamic_cast< CCylinderComponent * >( cylB );

    if (!cylAComp || !cylBComp) return false;

    FVector posA = cylA->GetWorldLocation ();
    float radiusA = cylAComp->GetRadius ();
    float halfHeightA = cylAComp->GetHalfHeight ();
    FQuat rotA = cylA->GetOwnerActor ()->GetActorRotationQuat ();

    FVector posB = cylB->GetWorldLocation ();
    float radiusB = cylBComp->GetRadius ();
    float halfHeightB = cylBComp->GetHalfHeight ();
    FQuat rotB = cylB->GetOwnerActor ()->GetActorRotationQuat ();

    // Оси цилиндров в локальном пространстве
    FVector axisA = rotA * FVector ( 0, 1, 0 );
    FVector axisB = rotB * FVector ( 0, 1, 0 );

    // Вектор между центрами
    FVector delta = posB - posA;

    // Находим ближайшие точки между осями
    float a = 1.0f;
    float b = 1.0f;
    float c = axisA.Dot ( axisB );
    float d = axisA.Dot ( delta );
    float e = axisB.Dot ( delta );

    float s, t;
    float denom = a * b - c * c;

    if (std::abs ( denom ) < 0.001f)
        {
        s = 0.0f;
        t = e;
        }
    else
        {
        s = ( c * e - b * d ) / denom;
        t = ( a * e - c * d ) / denom;
        }

        // Ограничиваем параметры
    s = std::max ( -halfHeightA, std::min ( halfHeightA, s ) );
    t = std::max ( -halfHeightB, std::min ( halfHeightB, t ) );

    // Ближайшие точки на осях
    FVector closestA = posA + axisA * s;
    FVector closestB = posB + axisB * t;

    // Проверяем расстояние
    FVector deltaClosest = closestB - closestA;
    float distXZ = std::sqrt ( deltaClosest.x * deltaClosest.x + deltaClosest.z * deltaClosest.z );

    if (distXZ > radiusA + radiusB)
        return false;

    // Проверка по высоте
    if (std::abs ( deltaClosest.y ) > halfHeightA + halfHeightB)
        return false;

    // Есть коллизия
    outInfo.ComponentA = cylA;
    outInfo.ComponentB = cylB;

    if (distXZ > 0.001f)
        {
        outInfo.Normal = ( closestB - closestA ).Normalized ();
        }
    else
        {
        outInfo.Normal = FVector ( 1.0f, 0.0f, 0.0f );
        }

    outInfo.Depth = radiusA + radiusB - distXZ;
    outInfo.Location = ( closestA + closestB ) * 0.5f;

    return true;
    }

bool CCollisionSystem::CheckCylinderTerrain ( CBaseCollisionComponent * cylinder,
                                              CBaseCollisionComponent * terrain,
                                              FCollisionInfo & outInfo ) const
    {
    CCylinderComponent * cylComp = dynamic_cast< CCylinderComponent * >( cylinder );
    CTerrainComponent * terrComp = dynamic_cast< CTerrainComponent * >( terrain );

    if (!cylComp || !terrComp) return false;

    FVector cylPos = cylinder->GetWorldLocation ();
    float cylRadius = cylComp->GetRadius ();
    float cylHalfHeight = cylComp->GetHalfHeight ();
    FQuat cylRot = cylinder->GetOwnerActor ()->GetActorRotationQuat ();

    // Получаем нижнюю точку цилиндра
    FVector bottomCenter = cylPos + cylRot * FVector ( 0, -cylHalfHeight, 0 );
    float lowestPoint = bottomCenter.y - cylRadius;

    // Получаем высоту террейна под центром цилиндра
    float terrainHeight = terrComp->GetHeightAtWorld ( cylPos );

    if (lowestPoint <= terrainHeight)
        {
        outInfo.ComponentA = cylinder;
        outInfo.ComponentB = terrain;
        outInfo.Depth = terrainHeight - lowestPoint;
        outInfo.Normal = FVector ( 0.0f, 1.0f, 0.0f );
        outInfo.Location = FVector ( cylPos.x, terrainHeight, cylPos.z );

        return true;
        }

    return false;
    }

    // ============================================================================
    // Cone Collision Checks
    // ============================================================================

bool CCollisionSystem::CheckSphereCone ( CBaseCollisionComponent * sphere,
                                         CBaseCollisionComponent * cone,
                                         FCollisionInfo & outInfo ) const
    {
    CSphereComponent * sphereComp = dynamic_cast< CSphereComponent * >( sphere );
    CConeComponent * coneComp = dynamic_cast< CConeComponent * >( cone );

    if (!sphereComp || !coneComp) return false;

    FVector spherePos = sphere->GetWorldLocation ();
    float sphereRadius = sphereComp->GetRadius ();

    FVector conePos = cone->GetWorldLocation ();
    float coneRadius = coneComp->GetRadius ();
    float coneHeight = coneComp->GetHeight ();
    float coneHalfHeight = coneHeight * 0.5f;
    float coneSlope = coneComp->GetSlope (); // radius / height

    FQuat coneRot = cone->GetOwnerActor ()->GetActorRotationQuat ();

    // Преобразуем позицию сферы в локальное пространство конуса
    // В локальном пространстве: основание внизу (y = -halfHeight), острие вверху (y = +halfHeight)
    FVector localSpherePos = coneRot.Inverse () * ( spherePos - conePos );

    // Проверка по высоте (сфера может быть выше или ниже конуса)
    if (localSpherePos.y < -coneHalfHeight - sphereRadius ||
         localSpherePos.y > coneHalfHeight + sphereRadius)
        return false;

    // Радиус конуса на данной высоте (линейно уменьшается от основания к острию)
    float t = ( localSpherePos.y + coneHalfHeight ) / coneHeight; // 0 у основания, 1 у острия
    t = std::max ( 0.0f, std::min ( 1.0f, t ) );
    float radiusAtY = coneRadius * ( 1.0f - t );

    // Расстояние от оси конуса в плоскости XZ
    float distXZ = std::sqrt ( localSpherePos.x * localSpherePos.x + localSpherePos.z * localSpherePos.z );

    // Проверка на коллизию
    if (distXZ > radiusAtY + sphereRadius)
        return false;

    // Если сфера ниже основания конуса, проверяем специально
    if (localSpherePos.y < -coneHalfHeight)
        {
            // Проверяем коллизию с основанием (круг)
        float distToBase = std::abs ( localSpherePos.y + coneHalfHeight );
        if (distToBase > sphereRadius)
            return false;

        // Проверяем расстояние до края основания
        if (distXZ > coneRadius + sphereRadius)
            return false;
        }

        // Если сфера выше острия, проверяем коллизию с остриём
    if (localSpherePos.y > coneHalfHeight)
        {
        float distToTip = std::abs ( localSpherePos.y - coneHalfHeight );
        if (distToTip > sphereRadius)
            return false;

        // У острия радиус = 0
        if (distXZ > sphereRadius)
            return false;
        }

        // Есть коллизия - находим ближайшую точку на конусе
    FVector localClosestPoint = localSpherePos;

    // Ограничиваем по высоте
    if (localClosestPoint.y < -coneHalfHeight)
        localClosestPoint.y = -coneHalfHeight;
    else if (localClosestPoint.y > coneHalfHeight)
        localClosestPoint.y = coneHalfHeight;

    // Находим радиус на этой высоте
    t = ( localClosestPoint.y + coneHalfHeight ) / coneHeight;
    t = std::max ( 0.0f, std::min ( 1.0f, t ) );
    float closestRadius = coneRadius * ( 1.0f - t );

    // Ограничиваем по радиусу
    float closestDistXZ = std::sqrt ( localClosestPoint.x * localClosestPoint.x +
                                      localClosestPoint.z * localClosestPoint.z );

    if (closestDistXZ > closestRadius)
        {
        float scale = closestRadius / closestDistXZ;
        localClosestPoint.x *= scale;
        localClosestPoint.z *= scale;
        }

        // Вычисляем глубину проникновения
    FVector delta = localSpherePos - localClosestPoint;
    float distance = delta.Length ();

    outInfo.ComponentA = sphere;
    outInfo.ComponentB = cone;
    outInfo.Depth = sphereRadius - distance;

    if (distance > 0.001f)
        {
        outInfo.Normal = ( coneRot * delta ).Normalized ();
        outInfo.Location = spherePos - outInfo.Normal * sphereRadius;
        }
    else
        {
        outInfo.Normal = FVector ( 0.0f, 1.0f, 0.0f );
        outInfo.Location = spherePos;
        }

    return true;
    }

bool CCollisionSystem::CheckBoxCone ( CBaseCollisionComponent * box,
                                      CBaseCollisionComponent * cone,
                                      FCollisionInfo & outInfo ) const
    {
    CBoxComponent * boxComp = dynamic_cast< CBoxComponent * >( box );
    CConeComponent * coneComp = dynamic_cast< CConeComponent * >( cone );

    if (!boxComp || !coneComp) return false;

    FVector boxPos = box->GetWorldLocation ();
    FVector boxHalf = boxComp->GetHalfExtents ();
    FQuat boxRot = box->GetOwnerActor ()->GetActorRotationQuat ();

    FVector conePos = cone->GetWorldLocation ();
    float coneRadius = coneComp->GetRadius ();
    float coneHeight = coneComp->GetHeight ();
    float coneHalfHeight = coneHeight * 0.5f;
    float coneSlope = coneComp->GetSlope ();

    FQuat coneRot = cone->GetOwnerActor ()->GetActorRotationQuat ();

    // Преобразуем позицию бокса в локальное пространство конуса
    FVector localBoxPos = coneRot.Inverse () * ( boxPos - conePos );

    // Трансформируем половины размеров бокса в локальное пространство конуса
    // Это сложно для точной проверки, используем приближение с 8 углами
    FVector corners[ 8 ];
    for (int i = 0; i < 8; i++)
        {
        FVector localCorner (
            ( i & 1 ) ? boxHalf.x : -boxHalf.x,
            ( i & 2 ) ? boxHalf.y : -boxHalf.y,
            ( i & 4 ) ? boxHalf.z : -boxHalf.z
        );

        // Поворачиваем угол в мировое пространство, затем в локальное конуса
        FVector worldCorner = boxPos + boxRot * localCorner;
        corners[ i ] = coneRot.Inverse () * ( worldCorner - conePos );
        }

        // Проверяем каждый угол на принадлежность конусу
    bool anyInside = false;
    for (int i = 0; i < 8; i++)
        {
        float y = corners[ i ].y;
        float distXZ = std::sqrt ( corners[ i ].x * corners[ i ].x + corners[ i ].z * corners[ i ].z );

        // Радиус конуса на этой высоте
        float t = ( y + coneHalfHeight ) / coneHeight;
        if (t < 0 || t > 1) continue; // Вне высоты конуса

        float radiusAtY = coneRadius * ( 1.0f - t );

        if (distXZ <= radiusAtY)
            {
            anyInside = true;
            break;
            }
        }

    if (!anyInside)
        {
            // Дополнительно проверяем пересечение рёбер конуса с боксом
            // Упрощённо - проверяем расстояние до оси конуса
        float minY = corners[ 0 ].y, maxY = corners[ 0 ].y;
        float minXZ = std::sqrt ( corners[ 0 ].x * corners[ 0 ].x + corners[ 0 ].z * corners[ 0 ].z );

        for (int i = 1; i < 8; i++)
            {
            minY = std::min ( minY, corners[ i ].y );
            maxY = std::max ( maxY, corners[ i ].y );
            float distXZ = std::sqrt ( corners[ i ].x * corners[ i ].x + corners[ i ].z * corners[ i ].z );
            minXZ = std::min ( minXZ, distXZ );
            }

            // Если бокс полностью выше или ниже конуса
        if (maxY < -coneHalfHeight || minY > coneHalfHeight)
            return false;

        // Находим максимальный радиус конуса в диапазоне высот бокса
        float yCenter = ( minY + maxY ) * 0.5f;
        float t = ( yCenter + coneHalfHeight ) / coneHeight;
        t = std::max ( 0.0f, std::min ( 1.0f, t ) );
        float radiusAtY = coneRadius * ( 1.0f - t );

        // Если бокс достаточно далеко от оси
        if (minXZ > radiusAtY)
            return false;
        }

        // Есть коллизия
    outInfo.ComponentA = box;
    outInfo.ComponentB = cone;

    // Упрощённая нормаль (в сторону от оси конуса)
    FVector dirToBox = localBoxPos;
    dirToBox.y = 0;
    if (dirToBox.Length () > 0.001f)
        {
        outInfo.Normal = ( coneRot * dirToBox ).Normalized ();
        }
    else
        {
        outInfo.Normal = FVector ( 1.0f, 0.0f, 0.0f );
        }

    outInfo.Depth = 1.0f; // Приблизительная глубина
    outInfo.Location = boxPos;

    return true;
    }

bool CCollisionSystem::CheckCapsuleCone ( CBaseCollisionComponent * capsule,
                                          CBaseCollisionComponent * cone,
                                          FCollisionInfo & outInfo ) const
    {
    CCapsuleComponent * capComp = dynamic_cast< CCapsuleComponent * >( capsule );
    CConeComponent * coneComp = dynamic_cast< CConeComponent * >( cone );

    if (!capComp || !coneComp) return false;

    FVector capPos = capsule->GetWorldLocation ();
    float capRadius = capComp->GetRadius ();
    float capHalfHeight = capComp->GetHalfHeight ();
    FQuat capRot = capsule->GetOwnerActor ()->GetActorRotationQuat ();

    FVector conePos = cone->GetWorldLocation ();
    float coneRadius = coneComp->GetRadius ();
    float coneHeight = coneComp->GetHeight ();
    float coneHalfHeight = coneHeight * 0.5f;
    float coneSlope = coneComp->GetSlope ();

    FQuat coneRot = cone->GetOwnerActor ()->GetActorRotationQuat ();

    // Получаем центры полусфер капсулы
    FVector capTop = capRot * FVector ( 0, 0, capHalfHeight ) + capPos;
    FVector capBottom = capRot * FVector ( 0, 0, -capHalfHeight ) + capPos;

    // Преобразуем в локальное пространство конуса
    FVector localCapTop = coneRot.Inverse () * ( capTop - conePos );
    FVector localCapBottom = coneRot.Inverse () * ( capBottom - conePos );

    // Упрощённо: проверяем расстояние от оси капсулы до оси конуса
    FVector capAxis = localCapTop - localCapBottom;
    float capLength = capAxis.Length ();
    FVector capDir = capAxis / capLength;

    // Ось конуса в локальном пространстве - Y
    FVector coneAxis ( 0, 1, 0 );

    // Находим ближайшие точки между осями
    FVector w = localCapBottom - FVector ( 0, -coneHalfHeight, 0 );
    float a = 1.0f;
    float b = capDir.Dot ( capDir );
    float c = capDir.Dot ( coneAxis );
    float d = coneAxis.Dot ( w );
    float e = capDir.Dot ( w );

    float t, s;
    float denom = a * b - c * c;

    if (std::abs ( denom ) < 0.001f)
        {
        t = 0.0f;
        s = e / b;
        }
    else
        {
        t = ( c * e - b * d ) / denom;
        s = ( a * e - c * d ) / denom;
        }

    t = std::max ( -coneHalfHeight, std::min ( coneHalfHeight, t ) );
    s = std::max ( 0.0f, std::min ( capLength, s ) );

    FVector closestCone = FVector ( 0, t, 0 );
    FVector closestCap = localCapBottom + capDir * s;

    // Радиус конуса на этой высоте
    float tNorm = ( t + coneHalfHeight ) / coneHeight;
    tNorm = std::max ( 0.0f, std::min ( 1.0f, tNorm ) );
    float radiusAtY = coneRadius * ( 1.0f - tNorm );

    // Расстояние между точками в плоскости XZ
    FVector delta = closestCap - closestCone;
    float distXZ = std::sqrt ( delta.x * delta.x + delta.z * delta.z );

    if (distXZ > radiusAtY + capRadius)
        return false;

    // Есть коллизия
    outInfo.ComponentA = capsule;
    outInfo.ComponentB = cone;

    if (distXZ > 0.001f)
        {
        FVector normalXZ = FVector ( delta.x, 0, delta.z ).Normalized ();
        outInfo.Normal = coneRot * normalXZ;
        }
    else
        {
        outInfo.Normal = FVector ( 1.0f, 0.0f, 0.0f );
        }

    outInfo.Depth = radiusAtY + capRadius - distXZ;
    outInfo.Location = ( capTop + capBottom ) * 0.5f + outInfo.Normal * capRadius;

    return true;
    }

bool CCollisionSystem::CheckCylinderCone ( CBaseCollisionComponent * cylinder,
                                           CBaseCollisionComponent * cone,
                                           FCollisionInfo & outInfo ) const
    {
    CCylinderComponent * cylComp = dynamic_cast< CCylinderComponent * >( cylinder );
    CConeComponent * coneComp = dynamic_cast< CConeComponent * >( cone );

    if (!cylComp || !coneComp) return false;

    FVector cylPos = cylinder->GetWorldLocation ();
    float cylRadius = cylComp->GetRadius ();
    float cylHalfHeight = cylComp->GetHalfHeight ();
    FQuat cylRot = cylinder->GetOwnerActor ()->GetActorRotationQuat ();

    FVector conePos = cone->GetWorldLocation ();
    float coneRadius = coneComp->GetRadius ();
    float coneHeight = coneComp->GetHeight ();
    float coneHalfHeight = coneHeight * 0.5f;
    float coneSlope = coneComp->GetSlope ();

    FQuat coneRot = cone->GetOwnerActor ()->GetActorRotationQuat ();

    // Преобразуем позицию цилиндра в локальное пространство конуса
    FVector localCylPos = coneRot.Inverse () * ( cylPos - conePos );

    // Ось цилиндра в локальном пространстве
    FVector cylAxisLocal = coneRot.Inverse () * ( cylRot * FVector ( 0, 1, 0 ) );

    // Упрощённо: проверяем коллизию как пересечение сфер
    float cylBoundRadius = std::sqrt ( cylRadius * cylRadius + cylHalfHeight * cylHalfHeight );
    float coneBoundRadius = std::sqrt ( coneRadius * coneRadius + coneHalfHeight * coneHalfHeight );

    if (( localCylPos ).Length () > cylBoundRadius + coneBoundRadius)
        return false;

    // Более точная проверка - дискретизация цилиндра по высоте
    int numSamples = 5;
    for (int i = 0; i < numSamples; i++)
        {
        float t = ( float ) i / ( numSamples - 1 );
        float yOffset = ( t - 0.5f ) * 2.0f * cylHalfHeight;

        FVector localPoint = localCylPos + cylAxisLocal * yOffset;

        // Радиус конуса на этой высоте
        float tNorm = ( localPoint.y + coneHalfHeight ) / coneHeight;
        if (tNorm < 0 || tNorm > 1) continue;

        float radiusAtY = coneRadius * ( 1.0f - tNorm );
        float distXZ = std::sqrt ( localPoint.x * localPoint.x + localPoint.z * localPoint.z );

        if (distXZ <= radiusAtY + cylRadius)
            {
            outInfo.ComponentA = cylinder;
            outInfo.ComponentB = cone;
            outInfo.Normal = FVector ( 1.0f, 0.0f, 0.0f );
            outInfo.Depth = 1.0f;
            outInfo.Location = cylPos;
            return true;
            }
        }

    return false;
    }

bool CCollisionSystem::CheckConeCone ( CBaseCollisionComponent * coneA,
                                       CBaseCollisionComponent * coneB,
                                       FCollisionInfo & outInfo ) const
    {
    CConeComponent * coneAComp = dynamic_cast< CConeComponent * >( coneA );
    CConeComponent * coneBComp = dynamic_cast< CConeComponent * >( coneB );

    if (!coneAComp || !coneBComp) return false;

    FVector posA = coneA->GetWorldLocation ();
    float radiusA = coneAComp->GetRadius ();
    float heightA = coneAComp->GetHeight ();
    float halfHeightA = heightA * 0.5f;
    FQuat rotA = coneA->GetOwnerActor ()->GetActorRotationQuat ();

    FVector posB = coneB->GetWorldLocation ();
    float radiusB = coneBComp->GetRadius ();
    float heightB = coneBComp->GetHeight ();
    float halfHeightB = heightB * 0.5f;
    FQuat rotB = coneB->GetOwnerActor ()->GetActorRotationQuat ();

    // Упрощённо: проверяем как сферы
    float distSq = ( posB - posA ).LengthSquared ();
    float boundRadiusA = std::sqrt ( radiusA * radiusA + halfHeightA * halfHeightA );
    float boundRadiusB = std::sqrt ( radiusB * radiusB + halfHeightB * halfHeightB );

    if (distSq > ( boundRadiusA + boundRadiusB ) * ( boundRadiusA + boundRadiusB ))
        return false;

    // Точная проверка сложна, пока считаем что коллизия есть
    outInfo.ComponentA = coneA;
    outInfo.ComponentB = coneB;
    outInfo.Normal = ( posB - posA ).Normalized ();
    outInfo.Depth = boundRadiusA + boundRadiusB - std::sqrt ( distSq );
    outInfo.Location = ( posA + posB ) * 0.5f;

    return true;
    }

bool CCollisionSystem::CheckConeTerrain ( CBaseCollisionComponent * cone,
                                          CBaseCollisionComponent * terrain,
                                          FCollisionInfo & outInfo ) const
    {
    CConeComponent * coneComp = dynamic_cast< CConeComponent * >( cone );
    CTerrainComponent * terrComp = dynamic_cast< CTerrainComponent * >( terrain );

    if (!coneComp || !terrComp) return false;

    FVector conePos = cone->GetWorldLocation ();
    float coneRadius = coneComp->GetRadius ();
    float coneHeight = coneComp->GetHeight ();
    float coneHalfHeight = coneHeight * 0.5f;
    FQuat coneRot = cone->GetOwnerActor ()->GetActorRotationQuat (); 

    // Получаем нижнюю точку конуса (основание)
    FVector bottomCenter = conePos + coneRot * FVector ( 0, -coneHalfHeight, 0 );
    float lowestPoint = bottomCenter.y - coneRadius;

    // Получаем высоту террейна под центром конуса
    float terrainHeight = terrComp->GetHeightAtWorld ( conePos );

    if (lowestPoint <= terrainHeight)
        {
        outInfo.ComponentA = cone;
        outInfo.ComponentB = terrain;
        outInfo.Depth = terrainHeight - lowestPoint;
        outInfo.Normal = FVector ( 0.0f, 1.0f, 0.0f );
        outInfo.Location = FVector ( conePos.x, terrainHeight, conePos.z );

        return true;
        }

    return false;
    }