#pragma once

#include "Components/Collisions/BaseCollisionComponent.h"

class KE_API CBoxComponent : public CBaseCollisionComponent
    {
    CHUDDO_DECLARE_CLASS ( CBoxComponent, CBaseCollisionComponent );

    public:
        // Конструктор - исправлено название компонента и добавлен параметр для размеров
        CBoxComponent ( CObject * inOwner = nullptr,
                        const std::string & inDisplayName = "BoxComponent" );

        virtual ~CBoxComponent ();

        virtual void InitComponent () override;
        virtual void Tick ( float DeltaTime ) override;
        virtual void OnBeginPlay () override;
        FVector GetHalfExtents () const { return m_HalfExtents; }
        // Проверка коллизии с другими компонентами
        virtual bool CheckCollision ( CBaseCollisionComponent * other, FCollisionInfo & outInfo ) const override;

        // Получение радиуса для совместимости со сферической проверкой
        virtual float GetCollisionRadius () const override;

        // Установка/получение половины размеров бокса
        void SetHalfExtents ( const FVector & halfExtents ) { m_HalfExtents = halfExtents; }
       

        // Получение ограничивающего объема (для системы коллизий)
        virtual FVector GetBoundingBox () const override { return m_HalfExtents * 2.0f; }
        virtual FVector GetExtremePoint ( const FVector & Direction ) const override;
        FVector GetWorldLocation () const;
        FVector GetWorldRotation () const;
    private:
        FVector m_HalfExtents = FVector ( 25.0f, 25.0f, 25.0f );  // Половина размеров бокса по каждой оси

        // Вспомогательные методы - добавили const
    };