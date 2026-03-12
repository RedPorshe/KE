#pragma once

#include "Components/TransformComponent.h"
#include "Core/Collision.h"  

struct FCollisionInfo;

class KE_API CBaseCollisionComponent : public CTransformComponent
    {
    CHUDDO_DECLARE_CLASS ( CBaseCollisionComponent, CTransformComponent );

    public:
        CBaseCollisionComponent ( CObject * inOwner = nullptr,
                                  const std::string & inDisplayName = "BaseCollisionComponent" );
        virtual ~CBaseCollisionComponent ();

        virtual void InitComponent () override;
        virtual void Tick ( float DeltaTime ) override;
        virtual void OnBeginPlay () override;

        // Основные методы коллизий
        virtual bool CheckCollision ( CBaseCollisionComponent * other, FCollisionInfo & outInfo ) const;

        // НЕ-const версия (существующая)
        FVector GetWorldLocation ();

        // ДОБАВЛЯЕМ const версию
        FVector GetWorldLocation () const;

        // Управление коллизиями
        void SetCollisionEnabled ( bool value = true ) { bIsCollisionEnabled = value; }
        bool IsCollisionEnabled () const { return bIsCollisionEnabled; }

        // Работа с каналами (новые методы)
        void SetCollisionChannel ( const FCollisionChannel & channel );
        void SetCollisionChannel ( const std::string & channelName );  // Перегрузка для имени
        const FCollisionChannel & GetCollisionChannel () const { return m_CollisionChannel; }

        // Быстрые настройки каналов (удобные методы)
        void SetChannelAsStatic ();
        void SetChannelAsDynamic ();
        void SetChannelAsCharacter ();
        void SetChannelAsTrigger ();
        void SetChannelAsPawn ();
        void SetChannelAsVehicle ();
        void SetChannelAsInteractable ();
        void SetChannelAsCustom ( const std::string & channelName,
                                  ECollisionResponse defaultResponse = ECollisionResponse::BLOCK );

        // Настройка ответов на другие каналы
        void SetResponseToChannel ( const std::string & otherChannelName,
                                    ECollisionResponse response );
        void SetResponseToChannel ( ECollisionChannel otherChannel,
                                    ECollisionResponse response );

        // Проверка взаимодействия с другим компонентом
        bool CanCollideWith ( const CBaseCollisionComponent * other ) const;
        bool CanCollideWith ( const std::string & otherChannelName ) const;
        bool ShouldBlockWith ( const CBaseCollisionComponent * other ) const;
        bool ShouldOverlapWith ( const CBaseCollisionComponent * other ) const;
        virtual float GetCollisionRadius () const { return 0.0f; }
        virtual FVector GetBoundingBox () const { return FVector::Zero (); };
        virtual FVector GetExtremePoint ( const FVector & Direction ) const;
        virtual FVector GetTopPoint () const;
        virtual FVector GetBottomPoint () const;
        virtual FVector GetFrontPoint () const;
        virtual FVector GetBackPoint () const;
        virtual FVector GetLeftPoint () const;
        virtual FVector GetRightPoint () const;
        FVector GetLocalExtremePoint ( const FVector & LocalDirection ) const;

        // Форма коллизии
        ECollisionShape GetShapeType () const { return m_CollisionShape; }

        void OnBeginOverlap ( CBaseCollisionComponent * other );
        void OnEndOverlap ( CBaseCollisionComponent * other );
        void OnHit ( CBaseCollisionComponent * other );

        void SetShapeType ( const ECollisionShape & inShape );

        // Статический метод для проверки состояния системы
        static bool IsCollisionSystemShuttingDown ();

    protected:
        // Установка формы (только для наследников)
        std::unordered_set<CBaseCollisionComponent *> OverlappingComponents;

        // Данные коллизии
        FCollisionChannel m_CollisionChannel = FCollisionChannel::Static ();
        ECollisionShape m_CollisionShape = ECollisionShape::NONE;
        bool bIsCollisionEnabled = true;
    };

REGISTER_CLASS_FACTORY ( CBaseCollisionComponent );