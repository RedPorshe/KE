#pragma once

#include "KE/GameFramework/Components/Collisions/BaseCollisionComponent.h"

class KE_API CCylinderComponent : public CBaseCollisionComponent
    {
    CHUDDO_DECLARE_CLASS ( CCylinderComponent, CBaseCollisionComponent );

    public:
        CCylinderComponent ( CObject * inOwner = nullptr,
                             const std::string & inDisplayName = "CylinderComponent" );
        virtual ~CCylinderComponent ();

        virtual void InitComponent () override;
        virtual void Tick ( float DeltaTime ) override;
        virtual void OnBeginPlay () override;

        // Основные методы коллизий
        virtual bool CheckCollision ( CBaseCollisionComponent * other, FCollisionInfo & outInfo ) const override;
        virtual float GetCollisionRadius () const override;
        virtual FVector GetBoundingBox () const override { return FVector ( m_Radius * 2.0f, m_Radius * 2.0f, m_Height ); }

        // Геттеры/сеттеры
        float GetRadius () const { return m_Radius; }
        float GetHeight () const { return m_Height; }
        void SetRadius ( float radius ) { m_Radius = radius; }
        void SetHeight ( float height ) { m_Height = height; }

        // Специфические методы для цилиндра
        FVector GetTopCenter () const;
        FVector GetBottomCenter () const;
        float GetHalfHeight () const { return m_Height * 0.5f; }
        virtual FVector GetExtremePoint ( const FVector & Direction ) const override;
    private:
        float m_Radius = 25.0f;   // Радиус цилиндра
        float m_Height = 50.0f;    // Полная высота цилиндра
    };

REGISTER_CLASS_FACTORY ( CCylinderComponent );