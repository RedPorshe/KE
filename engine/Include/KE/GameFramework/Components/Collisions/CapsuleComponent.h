#pragma once

#include "Components/Collisions/BaseCollisionComponent.h"

class KE_API CCapsuleComponent : public CBaseCollisionComponent
    {
    CHUDDO_DECLARE_CLASS ( CCapsuleComponent, CBaseCollisionComponent );

    public:
        // ------------------------------------------------------------------------
        // Constructors & Destructor
        // ------------------------------------------------------------------------

        CCapsuleComponent ( CObject * inOwner = nullptr, const std::string & InName = "CapsuleComponent" );
        virtual ~CCapsuleComponent ();

        // ------------------------------------------------------------------------
        // Component Lifecycle
        // ------------------------------------------------------------------------

        virtual void InitComponent () override;
        virtual void Tick ( float DeltaTime ) override;
        virtual void OnBeginPlay () override;

        // ------------------------------------------------------------------------
        // Collision Interface
        // ------------------------------------------------------------------------

        virtual bool CheckCollision ( CBaseCollisionComponent * other, FCollisionInfo & outInfo ) const override;
        virtual float GetCollisionRadius () const override;
        virtual FVector GetBoundingBox () const override;
        virtual FVector GetExtremePoint ( const FVector & Direction ) const override;
        // ------------------------------------------------------------------------
        // Capsule Specific Methods
        // ------------------------------------------------------------------------

        void SetRadius ( float InRadius ) { m_Radius = InRadius; }
        float GetRadius () const { return m_Radius; }

        void SetHalfHeight ( float InHalfHeight ) { m_HalfHeight = InHalfHeight; }
        float GetHalfHeight () const { return m_HalfHeight; }

        void SetHeight ( float InHeight ) { m_HalfHeight = InHeight * 0.5f; }
        float GetHeight () const { return m_HalfHeight * 2.0f; }

        // Получить полную высоту включая полусферы
        float GetTotalHeight () const { return ( m_HalfHeight * 2.0f ) + ( m_Radius * 2.0f ); }

        // Получить позиции центров полусфер
        FVector GetTopSphereCenter () const;
        FVector GetBottomSphereCenter () const;

    private:
        float m_Radius = 18.0f;      // Радиус капсулы
        float m_HalfHeight = 8.6f;  // Половина высоты цилиндрической части
    };

REGISTER_CLASS_FACTORY ( CCapsuleComponent );