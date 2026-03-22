#pragma once

#include "KE/GameFramework/Components/BaseComponent.h"
#include "Core/Collision.h"
#include "Utils/Math/Matrix4.h"

class CBaseCollisionComponent;

class KE_API CTransformComponent : public CBaseComponent
    {
    CHUDDO_DECLARE_CLASS ( CTransformComponent, CBaseComponent );

    public:
        CTransformComponent ( CObject * inOwner = nullptr,
                              const std::string & inDisplayName = "TransformComponent" );
        virtual ~CTransformComponent ();

        virtual void InitComponent () override;
        virtual void Tick ( float DeltaTime ) override;
        virtual void OnBeginPlay () override;

        void UpdateTransform ();
        void MarkTransformDirty ();
        bool IdDirty () const { return bIsTransformDirty; }
        void SetAutoGenerateCollisionComponent ( bool value = true, const FCollisionChannel & inChannel = FCollisionChannel::Static () );

        // Локальное вращение (относительно собственных осей)
        void AddLocalRotation ( const FQuat & DeltaRotation );
        void AddLocalRotation ( const FVector & DeltaRotationDegrees );  // В градусах
        void AddLocalRotation ( float PitchDegrees, float YawDegrees, float RollDegrees );

        // Мировое вращение (относительно мировых осей)
        void AddWorldRotation ( const FQuat & DeltaRotation );
        void AddWorldRotation ( const FVector & DeltaRotationDegrees );  // В градусах
        void AddWorldRotation ( float PitchDegrees, float YawDegrees, float RollDegrees );

        // ========== Getters ==========
        // transform
        FTransform GetTransform () const;
        FTransform GetRelativeTransform () const;

        // Матрица трансформации для рендеринга
        const FMat4 & GetTransformMatrix () const { return m_TransformMatrix; }

        // location
        FVector GetLocation () const;
        FVector GetRelativeLocation () const;

        // scale
        FVector GetScale () const;
        FVector GetRelativeScale () const;

        // rotation
        FQuat GetRotationQuat () const;
        FQuat GetRelativeRotationQuat () const;
        FVector GetRotation () const;
        FVector GetRelativeRotation () const;

        // collision component
        CBaseCollisionComponent * GetCollisionComponent ();
        void SetCollisionComponent ( CBaseCollisionComponent * inComp );
        void SetCollisionComponent ( const std::string & inCompClassName );
        void SetCollisionEnabled ( bool value = true );

        // ========== Setters ==========
        // transform
        void SetTransform ( const FTransform & InTransform );
        void SetRelativeTransform ( const FTransform & InTransform );

        // location world
        void SetLocation ( const FVector & inLocation );
        void SetLocation ( float inX, float inY, float inZ );

        // location relative
        void SetRelativeLocation ( const FVector & inLocation );
        void SetRelativeLocation ( float inX, float inY, float inZ );

        // scale world
        void SetScale ( const FVector & inScale );
        void SetScale ( float inX, float inY, float inZ );
        void SetScale ( float scale );

        // scale relative
        void SetRelativeScale ( const FVector & inScale );
        void SetRelativeScale ( float inX, float inY, float inZ );
        void SetRelativeScale ( float scale );

        // rotation world
        void SetRotation ( const FQuat & inRotation );
        void SetRotation ( const FVector & inRotation );
        void SetRotation ( float inX, float inY, float inZ );

        // rotation relative
        void SetRelativeRotation ( const FQuat & inRotation );
        void SetRelativeRotation ( const FVector & inRotation );
        void SetRelativeRotation ( float inX, float inY, float inZ );

        // ========== Hierarchy ==========
        void AddChild ( CTransformComponent * Child );
        void RemoveChild ( CTransformComponent * Child );
        void AttachTo ( CTransformComponent * Parent );
        void DetachFromParent ();

        CTransformComponent * GetParent () const { return ParentTransform; }
        std::vector<CTransformComponent *> GetChildTransformComponents () const {
            return ChildTransformComponents;
            }

        bool IsChildTransformComponent () const;
        bool IsChildOf ( CTransformComponent * PotentialParent ) const;
        CTransformComponent * GetRootTransformComponent () const;

        FTransform GetParentTransform ();
    protected:
        FCollisionChannel CollisionChannel = FCollisionChannel::Static ();
        std::string m_collisionClass = "CBaseCollisionComponent";
        // Обновление матрицы трансформации для рендеринга
        virtual void UpdateTransformMatrix ();

        FTransform m_RelativeTransform = FTransform::Identity ();
        FTransform m_WorldTransform = FTransform::Identity ();

        // Кэшированные значения
        FTransform CachedWorldTransform = FTransform::Identity ();
        FTransform CachedRelativeTransform = FTransform::Identity ();

        // Матрица трансформации для рендеринга
        FMat4 m_TransformMatrix = FMat4::IdentityMatrix ();

        CBaseCollisionComponent * CollisionComp = nullptr;

        CTransformComponent * ParentTransform = nullptr;
        std::vector<CTransformComponent *> ChildTransformComponents;

        bool bIsTransformDirty = false;
        bool bIsCollisionEnabled = false;
        bool bIsAutoGenerateCollision = false;

    };

REGISTER_CLASS_FACTORY ( CTransformComponent );