#pragma once

#include "BaseCollisionComponent.h"

class KE_API CConeComponent : public CBaseCollisionComponent
    {
    CHUDDO_DECLARE_CLASS ( CConeComponent, CBaseCollisionComponent );

    public:
        CConeComponent ( CObject * inOwner = nullptr,
                         const std::string & inDisplayName = "ConeComponent" );
        virtual ~CConeComponent ();

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
        virtual FVector GetExtremePoint ( const FVector & Direction ) const override;
        // Специфические методы для конуса
        FVector GetTip () const;           // Острие конуса (вверху)
        FVector GetBaseCenter () const;     // Центр основания (внизу)
        float GetSlope () const;            // Наклон стенки (радиус/высота)

    private:
        float m_Radius = 25.0f;   // Радиус основания
        float m_Height = 50.0f;    // Высота конуса
    };

REGISTER_CLASS_FACTORY ( CConeComponent );