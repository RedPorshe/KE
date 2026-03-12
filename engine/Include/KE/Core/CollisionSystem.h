#pragma once
#include "CoreMinimal.h"
#include "Core/Object.h"

#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <set>

class CBaseCollisionComponent;
class CBoxComponent;
class CCapsuleComponent;
class CSphereComponent;

// ============================================================================
// Enums
// ============================================================================

enum class ECollisionEventType : uint8_t
    {
    BEGIN_OVERLAP,
    END_OVERLAP,
    COLLISION_HIT
    };

    // ============================================================================
    // Structures
    // ============================================================================

struct FCollisionInfo
    {
    CBaseCollisionComponent * ComponentA = nullptr;
    CBaseCollisionComponent * ComponentB = nullptr;
    FVector Location;       // Точка столкновения
    FVector Normal;         // Нормаль столкновения
    float Depth = 0.0f;     // Глубина проникновения
    };

struct FRaycastResult
    {
    bool bHit = false;
    CBaseCollisionComponent * HitComponent = nullptr;
    FVector Location;
    FVector Normal;
    float Distance = 0.0f;
    };

    // ============================================================================
    // Callback Types
    // ============================================================================

using FCollisionCallback = std::function<void ( const FCollisionInfo & )>;

// ============================================================================
// Main Collision System Class
// ============================================================================

class KE_API CCollisionSystem : public CObject
    {
    CHUDDO_DECLARE_CLASS ( CCollisionSystem, CObject );

    public:
        // ------------------------------------------------------------------------
        // Constructors & Destructor
        // ------------------------------------------------------------------------

        CCollisionSystem ( CObject * inOwner = nullptr, const std::string & inDisplayName = "CollisionSystem" );
        virtual ~CCollisionSystem ();

        // Delete copy/move constructors and assignment operators
        CCollisionSystem ( const CCollisionSystem & ) = delete;
        CCollisionSystem & operator=( const CCollisionSystem & ) = delete;
        void Shutdown ();

        // ------------------------------------------------------------------------
        // Singleton Access
        // ------------------------------------------------------------------------

        static CCollisionSystem & Get ();

        // ------------------------------------------------------------------------
        // Component Registration
        // ------------------------------------------------------------------------

        void RegisterCollisionComponent ( CBaseCollisionComponent * component );
        void UnregisterCollisionComponent ( CBaseCollisionComponent * component );

        // ------------------------------------------------------------------------
        // Main Update
        // ------------------------------------------------------------------------

        void Update ( float deltaTime );

        // ------------------------------------------------------------------------
        // Collision Queries
        // ------------------------------------------------------------------------
        std::vector<CBaseCollisionComponent *> GetAllCollisionComponents () const { return m_CollisionComponents; }
        // Manual collision checks
        std::vector<FCollisionInfo> CheckCollisions ( CBaseCollisionComponent * component ) const;
        std::vector<FCollisionInfo> CheckCollisionsAtLocation ( const FVector & location, float radius ) const;

        // Raycasting
        FRaycastResult Raycast ( const FVector & start, const FVector & end,
                                 const std::string & channelName = "All" ) const;
        FRaycastResult Raycast ( const FVector & start, const FVector & direction, float distance,
                                 const std::string & channelName = "All" ) const;

        // Overlap tests
        std::vector<CBaseCollisionComponent *> SphereOverlap ( const FVector & center, float radius,
                                                               const std::string & channelName = "All" ) const;
        std::vector<CBaseCollisionComponent *> BoxOverlap ( const FVector & center, const FVector & halfExtents,
                                                            const FVector & rotation,
                                                            const std::string & channelName = "All" ) const;

        // ------------------------------------------------------------------------
        // Callback Management
        // ------------------------------------------------------------------------

        void RegisterCollisionCallback ( ECollisionEventType eventType, const FCollisionCallback & callback );
        void UnregisterCollisionCallback ( ECollisionEventType eventType );

        // ------------------------------------------------------------------------
        // Debug & Statistics
        // ------------------------------------------------------------------------

        void EnableDebugDraw ( bool enable ) { bDebugDraw = enable; }
        bool IsDebugDrawEnabled () const { return bDebugDraw; }

        int32 GetRegisteredComponentsCount () const { return static_cast< int32 >( m_CollisionComponents.size () ); }
        int32 GetActiveCollisionsCount () const { return m_LastFrameCollisions; }

        // ------------------------------------------------------------------------
        // Optimization Settings
        // ------------------------------------------------------------------------

        void SetUpdateRate ( float rateHz ) { m_UpdateRate = rateHz; }
        void EnableSpatialPartition ( bool enable ) { bUseSpatialPartition = enable; }
        void SetSpatialCellSize ( float cellSize ) { m_CellSize = cellSize; }

        // ------------------------------------------------------------------------
        // State Checking
        // ------------------------------------------------------------------------

        static bool IsShuttingDown () { return bIsGlobalShuttingDown; }

    private:
        // ------------------------------------------------------------------------
        // Internal Collision Processing
        // ------------------------------------------------------------------------

        void ProcessCollisions ();
        void ResolveCollision ( const FCollisionInfo & collision );
        void FireCollisionEvent ( ECollisionEventType eventType, const FCollisionInfo & info );
        void ProcessCollisionsSpatial ();

        // ------------------------------------------------------------------------
        // Geometry Checks
        // ------------------------------------------------------------------------
        inline bool IsValidCollisionComponent ( CBaseCollisionComponent * comp ) const;
        void ProcessComponentPair ( const FCollisionInfo & collisionInfo );

        // Basic shape collisions
        bool CheckSphereSphere ( CBaseCollisionComponent * a, CBaseCollisionComponent * b, FCollisionInfo & outInfo ) const;
        bool CheckSphereBox ( CBaseCollisionComponent * sphere, CBaseCollisionComponent * box, FCollisionInfo & outInfo ) const;
        bool CheckBoxBox ( CBaseCollisionComponent * a, CBaseCollisionComponent * b, FCollisionInfo & outInfo ) const;

        // Capsule collisions
        bool CheckSphereCapsule ( CBaseCollisionComponent * sphere, CBaseCollisionComponent * capsule, FCollisionInfo & outInfo ) const;
        bool CheckBoxCapsule ( CBaseCollisionComponent * box, CBaseCollisionComponent * capsule, FCollisionInfo & outInfo ) const;
        bool CheckCapsuleCapsule ( CBaseCollisionComponent * capA, CBaseCollisionComponent * capB, FCollisionInfo & outInfo ) const;

        // Advanced box checks
        bool CheckAABBAABB ( const FVector & posA, const FVector & halfA,
                             const FVector & posB, const FVector & halfB,
                             FCollisionInfo & outInfo,
                             CBaseCollisionComponent * compA,
                             CBaseCollisionComponent * compB ) const;
        bool CheckOBBOBB ( CBoxComponent * boxA, CBoxComponent * boxB, FCollisionInfo & outInfo ) const;

        bool CheckSphereTerrain ( CBaseCollisionComponent * sphere, CBaseCollisionComponent * terrain, FCollisionInfo & outInfo ) const;
        bool CheckBoxTerrain ( CBaseCollisionComponent * box, CBaseCollisionComponent * terrain, FCollisionInfo & outInfo ) const;
        bool CheckCapsuleTerrain ( CBaseCollisionComponent * capsule, CBaseCollisionComponent * terrain, FCollisionInfo & outInfo ) const;
        bool CheckRayTerrain ( const FVector & start, const FVector & direction, float maxDistance,
                               CBaseCollisionComponent * terrain, FVector & outHit, FVector & outNormal, float & outDist ) const;

        // Cylinder collisions
        bool CheckSphereCylinder ( CBaseCollisionComponent * sphere, CBaseCollisionComponent * cylinder, FCollisionInfo & outInfo ) const;
        bool CheckBoxCylinder ( CBaseCollisionComponent * box, CBaseCollisionComponent * cylinder, FCollisionInfo & outInfo ) const;
        bool CheckCapsuleCylinder ( CBaseCollisionComponent * capsule, CBaseCollisionComponent * cylinder, FCollisionInfo & outInfo ) const;
        bool CheckCylinderCylinder ( CBaseCollisionComponent * cylA, CBaseCollisionComponent * cylB, FCollisionInfo & outInfo ) const;
        bool CheckCylinderTerrain ( CBaseCollisionComponent * cylinder, CBaseCollisionComponent * terrain, FCollisionInfo & outInfo ) const;

        // Cone collisions
        bool CheckSphereCone ( CBaseCollisionComponent * sphere, CBaseCollisionComponent * cone, FCollisionInfo & outInfo ) const;
        bool CheckBoxCone ( CBaseCollisionComponent * box, CBaseCollisionComponent * cone, FCollisionInfo & outInfo ) const;
        bool CheckCapsuleCone ( CBaseCollisionComponent * capsule, CBaseCollisionComponent * cone, FCollisionInfo & outInfo ) const;
        bool CheckCylinderCone ( CBaseCollisionComponent * cylinder, CBaseCollisionComponent * cone, FCollisionInfo & outInfo ) const;
        bool CheckConeCone ( CBaseCollisionComponent * coneA, CBaseCollisionComponent * coneB, FCollisionInfo & outInfo ) const;
        bool CheckConeTerrain ( CBaseCollisionComponent * cone, CBaseCollisionComponent * terrain, FCollisionInfo & outInfo ) const;

        // ------------------------------------------------------------------------
        // Spatial Partitioning
        // ------------------------------------------------------------------------

        struct FSpatialCell
            {
            std::vector<CBaseCollisionComponent *> Components;
            };

        void UpdateSpatialPartition ();
        std::vector<CBaseCollisionComponent *> GetPotentiallyCollidingComponents ( CBaseCollisionComponent * component ) const;
        float GetComponentBoundingRadius ( CBaseCollisionComponent * component ) const;

        // ------------------------------------------------------------------------
        // Member Variables
        // ------------------------------------------------------------------------

        // Core collision data
        std::vector<CBaseCollisionComponent *> m_CollisionComponents;
        std::unordered_map<CBaseCollisionComponent *, FVector> m_LastPositions;

        // Collision state tracking
        std::set<std::pair<CBaseCollisionComponent *, CBaseCollisionComponent *>> m_PreviousFrameCollisions;
        std::set<std::pair<CBaseCollisionComponent *, CBaseCollisionComponent *>> m_CurrentFrameCollisions;

        // Spatial partitioning
        bool bUseSpatialPartition = false;
        float m_CellSize = 500.0f;
        std::unordered_map<int64_t, FSpatialCell> m_SpatialGrid;

        // Callbacks
        std::unordered_map<ECollisionEventType, FCollisionCallback> m_Callbacks;

        // Statistics and timing
        bool bDebugDraw = false;
        int32 m_LastFrameCollisions = 0;
        float m_UpdateRate = 60.0f;        // Hz
        float m_AccumulatedTime = 0.0f;

        // Static instance for singleton
        static std::unique_ptr<CCollisionSystem> s_Instance;
        static bool s_IsInitialized;
        static bool bIsGlobalShuttingDown;  // Добавлено для проверки состояния выключения

        // Friend classes
        friend class CBoxComponent;
        friend class CSphereComponent;
        friend class CBaseCollisionComponent;
        friend class CCapsuleComponent;
        friend class CTerrainComponent;
        friend class CCylinderComponent;
        friend class CConeComponent;
    };

    // ============================================================================
    // Macros
    // ============================================================================

#define COLLISION_SYSTEM CCollisionSystem::Get()

#include "Components/Collisions/BaseCollisionComponent.h"
inline bool CCollisionSystem::IsValidCollisionComponent ( CBaseCollisionComponent * comp ) const
    { return comp && comp->IsCollisionEnabled (); }

    // ============================================================================
    // Factory Registration
    // ============================================================================

REGISTER_CLASS_FACTORY ( CCollisionSystem );