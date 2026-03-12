#pragma once
#include "Components/BaseComponent.h"

struct FRaycastResult;
class CBaseCollisionComponent;

class KE_API CGravityComponent : public CBaseComponent
    {
    CHUDDO_DECLARE_CLASS ( CGravityComponent, CBaseComponent );

    public:
        CGravityComponent ( CObject * inOwner = nullptr, const std::string & inDisplayName = "GravityComponent" );
        virtual ~CGravityComponent ();

        virtual void InitComponent () override;
        virtual void Tick ( float DeltaTime ) override;
        virtual void OnBeginPlay () override;

        // Настройки гравитации
        void SetGravityScale ( float scale ) { m_GravityScale = scale; }
        float GetGravityScale () const { return m_GravityScale; }

        void SetGravityStrength ( float strength ) { m_GravityStrength = strength; }
        float GetGravityStrength () const { return m_GravityStrength; }

        void SetMaxFallSpeed ( float maxSpeed ) { m_MaxFallSpeed = maxSpeed; }
        float GetMaxFallSpeed () const { return m_MaxFallSpeed; }

        void SetGroundCheckDistance ( float distance ) { m_GroundCheckDistance = distance; }
        float GetGroundCheckDistance () const { return m_GroundCheckDistance; }

        void ApplyGravity ( float DeltaTime );

        // Состояние
        bool IsGrounded () const { return bIsOnGround; }
        float GetVerticalVelocity () const { return m_VerticalVelocity; }
        void SetVerticalVelocity ( float vel ) { m_VerticalVelocity = vel; }
        bool IsGravityEnabled () const { return bIsGravityEnabled; }
        void SetEnableGravity ( bool bEnable ) { bIsGravityEnabled = bEnable; }

        // Дополнительные методы для проверки поверхности
        bool IsOnSlope () const { return m_CurrentSlopeAngle > m_MaxWalkableSlope; }
        float GetSlopeAngle () const { return m_CurrentSlopeAngle; }
        FVector GetGroundNormal () const { return m_GroundNormal; }
        FVector GetGroundPoint () const { return m_GroundPoint; }

        // Настройки ходьбы по склонам
        void SetMaxWalkableSlope ( float degrees ) { m_MaxWalkableSlope = degrees; }
        float GetMaxWalkableSlope () const { return m_MaxWalkableSlope; }

        void UpdateLastPosition ( const FVector & pos ) { m_LastPosition = pos; }

        // Вспомогательный метод для прыжка
        float GetJumpVelocity ( float JumpHeight ) const;

    protected:
        // Параметры гравитации
        float m_GravityScale = 1.0f;
        float m_GravityStrength = 9.8f;
        float m_VerticalVelocity = 0.0f;
        float m_KillZone = -1000.0f;
        float m_MaxFallSpeed = -1000.0f;
        float m_GroundCheckDistance = 5.0f;
        float m_MaxWalkableSlope = 35.0f;
        float m_LastCorrectionTime = 0.0f;
        float m_CorrectionCooldown = 0.02f;

        // Состояние
        bool bIsOnGround = false;
        bool m_bWasGrounded = false;
        float m_CurrentSlopeAngle = 0.0f;
        bool bIsGravityEnabled = false;
        FVector m_GroundNormal = FVector ( 0.0f, 1.0f, 0.0f );
        FVector m_GroundPoint = FVector::Zero ();
        FVector m_LastPosition = FVector::Zero ();

        // Вспомогательные методы
        bool CheckGroundWithSphere ( const FVector & position, CBaseCollisionComponent * collision );
        bool CheckGroundWithRay ( const FVector & start, const FVector & end, CBaseCollisionComponent * collision );
        void SnapToGround ( CBaseCollisionComponent * collision, const FRaycastResult & rayResult );
        bool SweepMovement ( CBaseCollisionComponent * collision,
                             const FVector & startPos,
                             const FVector & endPos,
                             FVector & outAdjustedPos,
                             FVector & outHitNormal,
                             CBaseCollisionComponent *& outHitComponent );

        void Fall ( float DeltaTime );
        void FlyUp ( float DeltaTime );
    };

REGISTER_CLASS_FACTORY ( CGravityComponent );