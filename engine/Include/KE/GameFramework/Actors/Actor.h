#pragma once

#include "Core/Object.h"
#include "Render/RenderInfo.h"

// Forward declarations
class CWorld;
class CLevel;
class CBaseComponent;
class CTransformComponent;
class CBaseCollisionComponent;
class CGravityComponent;

// ============================================================================
// Enums
// ============================================================================
enum class EMovableState : uint8_t
    {
    STATIC,     // Объект не двигается (гравитация выключена)
    MOVABLE,    // Объект можно толкать (гравитация включена)
    DYNAMIC     // Объект управляется (полная физика)
    };

    // ============================================================================
    // Render Collection
    // ============================================================================
struct FRenderCollection
    {
    std::vector<FMeshInfo> Meshes;
    std::vector<FTerrainRenderInfo> Terrains;
    std::vector<FCollisionDebugInfo> DebugCollisions;
    std::vector<FTerrainDebugInfo> TerrainWireframes;

    void Clear ()
        {
        Meshes.clear ();
        Terrains.clear ();
        DebugCollisions.clear ();
        TerrainWireframes.clear ();
        }
    };

    // ============================================================================
    // Actor Class
    // ============================================================================
class  KE_API CActor : public CObject
    {
    CHUDDO_DECLARE_CLASS ( CActor, CObject )

    public:
        // ------------------------------------------------------------------------
        // Constructor & Destructor
        // ------------------------------------------------------------------------
        CActor ( CObject * owner = nullptr, const std::string & inName = "Actor" );
        virtual ~CActor ();

        // ------------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------------
        virtual void BeginPlay ();
        virtual void Tick ( float deltaTime );
        virtual void EndPlay ();
        virtual void DebugInfo ( float deltaTime );

        void Destroy ();
        void SetPendingToDestroy ();
        bool IsPendingToDestroy () const { return bIsPendingToDestroy; }

        // ------------------------------------------------------------------------
        // World & Level Access
        // ------------------------------------------------------------------------
        CLevel * GetLevel () const;
        CWorld * GetWorld () const;

        void InitializeAllComponents ();
            
        // ------------------------------------------------------------------------
        // Component Management
        // ------------------------------------------------------------------------
        template<typename T>
        T * FindComponent () const
            {
            for (CBaseComponent * comp : ActorComponents)
                {
                if (T * typedComp = dynamic_cast< T * >( comp ))
                    return typedComp;
                }
            return nullptr;
            }

        template<typename Comp, typename... Args>
        Comp * AddDefaultSubObject ( const std::string & desiredDisplayName = "SubObject" );

        CBaseComponent * AddDefaultSubObject ( const std::string & className,
                                               const std::string & desiredDisplayName );

        std::vector<CBaseComponent *> GetActorComponents () const { return ActorComponents; }

        // ------------------------------------------------------------------------
        // Root Component
        // ------------------------------------------------------------------------
        CTransformComponent * GetRootComponent () const { return RootComponent; }
        void SetRootComponent ( CTransformComponent * NewRoot );

        // ------------------------------------------------------------------------
        // Transform Getters (const & non-const)
        // ------------------------------------------------------------------------
        FVector GetActorLocation ();
        FVector GetActorRotation ();
        FVector GetActorScale ();
        FQuat GetActorRotationQuat ();

        FVector GetActorLocation () const;
        FVector GetActorRotation () const;
        FVector GetActorScale () const;
        FQuat GetActorRotationQuat () const;

        // ------------------------------------------------------------------------
        // Direction Vectors
        // ------------------------------------------------------------------------
        FVector GetActorForwardVector ();
        FVector GetActorRightVector ();
        FVector GetActorUpVector ();

        FVector GetActorForwardVector () const;
        FVector GetActorRightVector () const;
        FVector GetActorUpVector () const;

        // ------------------------------------------------------------------------
        // Transform Setters
        // ------------------------------------------------------------------------
        void SetActorLocation ( const FVector & InLocation, bool bTeleport = false );
        void SetActorLocation ( float inX, float inY, float inZ, bool bTeleport = false );

        void SetActorScale ( const FVector & InScale );
        void SetActorScale ( float inX, float inY, float inZ );
        void SetActorScale ( float InScale );

        void SetActorRotation ( const FVector & inRotation );
        void SetActorRotation ( const FQuat & inRotation );
        void SetActorRotation ( float inX, float inY, float inZ );

        // ------------------------------------------------------------------------
        // Movement (Immediate)
        // ------------------------------------------------------------------------
        void MoveActor ( const FVector & Delta, bool Interpolate = true );
        void MoveActorInDirection ( const FVector & Direction, float Distance, bool Interpolate = true );

        void RotateActor ( const FVector & DeltaRotation, bool Interpolate = true );
        void RotateActor ( const FQuat & DeltaRotation, bool Interpolate = true );
        void RotateAroundAxis ( const FVector & Axis, float AngleDegrees, bool Interpolate = true );

        // ------------------------------------------------------------------------
        // Movement (Offset-based)
        // ------------------------------------------------------------------------
        void AddActorWorldOffset ( const FVector & DeltaLocation, bool Interpolate = false );
        void AddActorLocalOffset ( const FVector & DeltaLocation, bool Interpolate = false );

       
       

        // ------------------------------------------------------------------------
        // Teleportation (Immediate)
        // ------------------------------------------------------------------------
        void TeleportTo ( const FVector & NewLocation );
        void TeleportTo ( float NewX, float NewY, float NewZ );

        void SetActorRotationImmediately ( const FQuat & NewRotation );
        void SetActorRotationImmediately ( const FVector & NewRotation );
        void SetActorRotationImmediately ( float inX, float inY, float inZ );

        // ------------------------------------------------------------------------
        // Interpolation
        // ------------------------------------------------------------------------
        void SetInterpolationSpeed ( float inSpeed ) { LerpSpeed = inSpeed; }
        bool IsLerpingLocation () const { return bIsLerpingLocation; }
        bool IsLerpingRotation () const { return bIsLerpingRotation; }
        bool IsMoving () const { return bIsMoving; }

        // ------------------------------------------------------------------------
        // Physics & Movement State
        // ------------------------------------------------------------------------
        EMovableState GetMovableState () const { return MovableState; }
        void SetMovableState ( const EMovableState & state );

        bool IsStatic () const { return MovableState == EMovableState::STATIC; }
        bool IsMovable () const { return MovableState == EMovableState::MOVABLE || MovableState == EMovableState::DYNAMIC; }
        bool IsDynamic () const { return MovableState == EMovableState::DYNAMIC; }

        void AddImpulse ( const FVector & Impulse );
        void SetVelocity ( const FVector & NewVelocity );
        FVector GetVelocity () const { return Velocity; }

        // ------------------------------------------------------------------------
        // Gravity
        // ------------------------------------------------------------------------
        CGravityComponent * GetGravityComponent () { return m_Gravity; }
        void DestroyGravity ();

        // ------------------------------------------------------------------------
        // Collision
        // ------------------------------------------------------------------------
        void SetCollisionEnabled ( bool value = true );
        bool IsCollisionEnabled () const { return bIsCollisionEnabled; }

        virtual void OnComponentBeginOverlap ( CBaseCollisionComponent * other );
        virtual void OnComponentEndOverlap ( CBaseCollisionComponent * other );
        virtual void OnComponentHit ( CBaseCollisionComponent * other );

        // ------------------------------------------------------------------------
        // Rendering & Visibility
        // ------------------------------------------------------------------------
        virtual FRenderCollection GetRenderInfo () const;
        virtual std::vector<FMeshInfo> GetRenderMeshes () const;

        void SetDrawCollisions ( bool bDraw ) { m_bDrawCollisions = bDraw; }
        bool IsDrawCollisionsEnabled () const { return m_bDrawCollisions; }

        void SetHiddenInGame ( bool value ) { bIsHiddenInGame = value; }
        bool IsHiddenInGame () const { return bIsHiddenInGame; }

        // ------------------------------------------------------------------------
        // Spawning
        // ------------------------------------------------------------------------
        template<typename ClassName, typename... Args>
        ClassName * SpawnActor ( const std::string & name = "ActorFromActor", Args&&... args );

        // ------------------------------------------------------------------------
        // Misc
        // ------------------------------------------------------------------------
        void SetActorName ( const std::string & newName );
        void SetCanTickOnAttached ( bool value ) { bIsCanTickAsAttached = value; }
        bool IsCanTickOnAttached () const { return bIsCanTickAsAttached; }

        void SetIsAttached ( bool value ) { bIsAttached = value; }
        bool IsAttached () const { return bIsAttached; }

    protected:
        // ------------------------------------------------------------------------
        // Physics
        // ------------------------------------------------------------------------
        virtual void UpdatePhysics ( float DeltaTime );

        // ------------------------------------------------------------------------
        // Member Variables - Grouped by functionality
        // ------------------------------------------------------------------------

        // Components
        std::vector<CBaseComponent *> ActorComponents;
        CTransformComponent * RootComponent = nullptr;
        CGravityComponent * m_Gravity = nullptr;

        // Movement State
        EMovableState MovableState = EMovableState::STATIC;
        FVector Velocity = FVector::Zero ();
        bool bIsMoving = false;
        bool bIsTerrain = false;

        // Interpolation
        FVector TargetLocation;
        FQuat TargetRotation;
        FVector LerpStartLocation;
        FQuat LerpStartRotation;
        float LocationLerpAlpha = 0.0f;
        float RotationLerpAlpha = 0.0f;
        bool bIsLerpingLocation = false;
        bool bIsLerpingRotation = false;
        float LerpSpeed = 10.0f;

        // Collision
        bool bIsCollisionEnabled = true;

        // Flags
        bool bIsCanTickAsAttached = false;
        bool bIsAttached = false;
        bool bIsPendingToDestroy = false;
        bool bIsHiddenInGame = false;
       

        // Debug
        bool m_bDrawCollisions = false;
        float DebugTimer = 0.f;
        bool bIsStarted = false;
        int saveBeginPlayCount = 0;
        public:
            bool ActorStartedBeginPlay () const { return bIsStarted; }
    };

    // ============================================================================
    // Template Implementations
    // ============================================================================
#include "Components/SceneComponent.h"

template<typename Comp, typename... Args>
inline Comp * CActor::AddDefaultSubObject ( const std::string & desiredDisplayName )
    {
    static_assert( std::is_base_of<CBaseComponent, Comp>::value,
                   "Class must be derived from CBaseComponent" );

    auto newComp = this->AddSubObject<Comp> ( desiredDisplayName );

    if (RootComponent == nullptr)
        {
        if (CTransformComponent * sceneComp = dynamic_cast< CTransformComponent * >( newComp ))
            {
            RootComponent = sceneComp;
            }
        }

    ActorComponents.push_back ( newComp );
    return newComp;
    }

template<typename ClassName, typename... Args>
inline ClassName * CActor::SpawnActor ( const std::string & name, Args&&... args )
    {
    return this->GetWorld ()->GetCurrentLevel ()->SpawnActor<ClassName> (
        name, std::forward<Args> ( args )... );
    }

REGISTER_CLASS_FACTORY ( CActor );