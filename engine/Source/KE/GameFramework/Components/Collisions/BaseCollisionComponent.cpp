#include "KE/GameFramework/Components/Collisions/BaseCollisionComponent.h"
#include "KE/GameFramework/Actors/Actor.h"
#include "KE/Systems/CollisionSystem.h"

CBaseCollisionComponent::CBaseCollisionComponent ( CObject * inOwner,
                                                   const std::string & inDisplayName )
    : Super ( inOwner, inDisplayName )
    {
        // Проверяем, можно ли регистрироваться в системе
    if (!CCollisionSystem::IsShuttingDown ())
        {
        CCollisionSystem::Get ().RegisterCollisionComponent ( this );
        }
    else
        {
        LOG_WARN ( "[COLLISION] Cannot register component '", GetName (), "' - CollisionSystem not available" );
        }
    }

CBaseCollisionComponent::~CBaseCollisionComponent ()
    {
    OnEndPlay ();
    }

FVector CBaseCollisionComponent::GetWorldLocation ()
    {
    CActor * ownerActor = GetOwnerActor ();
    if (ownerActor)
        {
        FVector pos = ownerActor->GetActorLocation ();
        return pos;
        }

    CObject * owner = GetOwner ();
    int depth = 0;
    while (owner && depth < 10) // Ограничиваем глубину чтобы не зациклиться
        {
        if (CActor * actor = dynamic_cast< CActor * > ( owner ))
            {
            FVector pos = actor->GetActorLocation ();
            return pos;
            }

        if (CTransformComponent * transform = dynamic_cast< CTransformComponent * > ( owner ))
            {
            FVector pos = transform->GetLocation ();
            return pos;
            }

        owner = owner->GetOwner ();
        depth++;
        }

    LOG_WARN ( "[COLLISION] ", GetName (), " GetWorldLocation() failed - no valid owner found" );
    return FVector::Zero ();
    }

FVector CBaseCollisionComponent::GetWorldLocation () const
    {
    const CActor * ownerActor = GetOwnerActor ();
    if (ownerActor)
        {
        return ownerActor->GetActorLocation ();
        }

    const CObject * owner = GetOwner ();
    int depth = 0;
    while (owner && depth < 10)
        {
        if (const CActor * actor = dynamic_cast< const CActor * > ( owner ))
            {
            return actor->GetActorLocation ();
            }

        if (const CTransformComponent * transform = dynamic_cast< const CTransformComponent * > ( owner ))
            {
            return transform->GetLocation ();
            }

        owner = owner->GetOwner ();
        depth++;
        }

    return FVector::Zero ();
    }

void CBaseCollisionComponent::InitComponent ()
    {
    Super::InitComponent ();
    }

void CBaseCollisionComponent::Tick ( float DeltaTime )
    {
    Super::Tick ( DeltaTime );
    }

void CBaseCollisionComponent::OnEndPlay ()
    {
    Super::OnEndPlay ();
         // Проверяем, можно ли отписываться от системы
    if (!CCollisionSystem::IsShuttingDown ())
        {
        CCollisionSystem::Get ().UnregisterCollisionComponent ( this );
        }

    OverlappingComponents.clear ();
    }
void CBaseCollisionComponent::OnBeginPlay ()
    {
    Super::OnBeginPlay ();

    if (bIsCollisionEnabled)
        {
        LOG_DEBUG ( "Collision component '", GetName (), "' enabled with channel: ", m_CollisionChannel.GetName () );
        }
    }

bool CBaseCollisionComponent::CheckCollision ( CBaseCollisionComponent * other, FCollisionInfo & outInfo ) const
    {
    LOG_DEBUG ( "Collision this CBaseCollisionComponent always ignore and return false" );
    return false;
    }

void CBaseCollisionComponent::SetShapeType ( const ECollisionShape & inShape )
    {
    m_CollisionShape = inShape;
    }

void CBaseCollisionComponent::SetCollisionChannel ( const FCollisionChannel & channel )
    {
    m_CollisionChannel = channel;
    }

void CBaseCollisionComponent::SetCollisionChannel ( const std::string & channelName )
    {
    m_CollisionChannel = FCollisionChannel::Create ( channelName );
    }

void CBaseCollisionComponent::SetChannelAsStatic ()
    {
    m_CollisionChannel.SetupAsStatic ();
    LOG_DEBUG ( "Channel set as Static" );
    }

void CBaseCollisionComponent::SetChannelAsDynamic ()
    {
    m_CollisionChannel.SetupAsDynamic ();
    LOG_DEBUG ( "Channel set as Dynamic" );
    }

void CBaseCollisionComponent::SetChannelAsCharacter ()
    {
    m_CollisionChannel.SetupAsCharacter ();
    LOG_DEBUG ( "Channel set as Character" );
    }

void CBaseCollisionComponent::SetChannelAsTrigger ()
    {
    m_CollisionChannel.SetupAsTrigger ();
    LOG_DEBUG ( "Channel set as Trigger" );
    }

void CBaseCollisionComponent::SetChannelAsPawn ()
    {
    m_CollisionChannel.SetupAsPawn ();
    LOG_DEBUG ( "Channel set as Pawn" );
    }

void CBaseCollisionComponent::SetChannelAsVehicle ()
    {
    m_CollisionChannel.SetupAsVehicle ();
    LOG_DEBUG ( "Channel set as Vehicle" );
    }

void CBaseCollisionComponent::SetChannelAsInteractable ()
    {
    m_CollisionChannel.SetupAsInteractable ();
    LOG_DEBUG ( "Channel set as Interactable" );
    }

void CBaseCollisionComponent::SetChannelAsCustom ( const std::string & channelName,
                                                   ECollisionResponse defaultResponse )
    {
    m_CollisionChannel = FCollisionChannel::Create ( channelName, defaultResponse );
    }

void CBaseCollisionComponent::SetResponseToChannel ( const std::string & otherChannelName,
                                                     ECollisionResponse response )
    {
    m_CollisionChannel.SetResponseTo ( otherChannelName, response );
    }

void CBaseCollisionComponent::SetResponseToChannel ( ECollisionChannel otherChannel,
                                                     ECollisionResponse response )
    {
    m_CollisionChannel.SetResponseTo ( otherChannel, response );
    }

bool CBaseCollisionComponent::CanCollideWith ( const CBaseCollisionComponent * other ) const
    {
    if (!other || !bIsCollisionEnabled || !other->bIsCollisionEnabled)
        return false;

    return m_CollisionChannel.CanCollideWith ( other->m_CollisionChannel.GetChannel () );
    }

bool CBaseCollisionComponent::CanCollideWith ( const std::string & otherChannelName ) const
    {
    if (!bIsCollisionEnabled)
        return false;

    return m_CollisionChannel.CanCollideWith ( otherChannelName );
    }

bool CBaseCollisionComponent::ShouldBlockWith ( const CBaseCollisionComponent * other ) const
    {
    if (!other || !bIsCollisionEnabled || !other->bIsCollisionEnabled)
        return false;

    return m_CollisionChannel.ShouldBlock ( other->m_CollisionChannel.GetChannel () );
    }

bool CBaseCollisionComponent::ShouldOverlapWith ( const CBaseCollisionComponent * other ) const
    {
    if (!other || !bIsCollisionEnabled || !other->bIsCollisionEnabled)
        return false;

    return m_CollisionChannel.ShouldOverlap ( other->m_CollisionChannel.GetChannel () );
    }

void CBaseCollisionComponent::OnBeginOverlap ( CBaseCollisionComponent * other )
    {
    if (!other || !GetOwnerActor ())
        return;


    if (OverlappingComponents.find ( other ) != OverlappingComponents.end ())
        return;

    OverlappingComponents.insert ( other );
  
    GetOwnerActor ()->OnComponentBeginOverlap ( other );
    }
void CBaseCollisionComponent::OnEndOverlap ( CBaseCollisionComponent * other )
    {
    if (!other || !GetOwnerActor ())
        return;
   
    if (OverlappingComponents.find ( other ) == OverlappingComponents.end ())
        return;
    
    OverlappingComponents.erase ( other );

    GetOwnerActor ()->OnComponentEndOverlap ( other );
    }

void CBaseCollisionComponent::OnHit ( CBaseCollisionComponent * other )
    {
    if (GetOwnerActor () != nullptr)
        {
        GetOwnerActor ()->OnComponentHit ( other );
        }
    }

FVector CBaseCollisionComponent::GetExtremePoint ( const FVector & Direction ) const
    {
        // Базовая реализация - возвращает центр (должна быть переопределена в наследниках)
    return GetWorldLocation ();
    }

FVector CBaseCollisionComponent::GetTopPoint () const
    {
        
    return GetExtremePoint ( FVector ( 0.0f, 1.0f, 0.0f ) );
    }

FVector CBaseCollisionComponent::GetBottomPoint () const
    {
    return GetExtremePoint ( FVector ( 0.0f, -1.0f, 0.0f ) );
    }

FVector CBaseCollisionComponent::GetFrontPoint () const
    {
        // Предположим, что Z - вперед
    return GetExtremePoint ( FVector ( 0.0f, 0.0f, 1.0f ) );
    }

FVector CBaseCollisionComponent::GetBackPoint () const
    {
    return GetExtremePoint ( FVector ( 0.0f, 0.0f, -1.0f ) );
    }

FVector CBaseCollisionComponent::GetLeftPoint () const
    {
        // Предположим, что X - вправо, значит Left = -X
    return GetExtremePoint ( FVector ( -1.0f, 0.0f, 0.0f ) );
    }

FVector CBaseCollisionComponent::GetRightPoint () const
    {
    return GetExtremePoint ( FVector ( 1.0f, 0.0f, 0.0f ) );
    }

FVector CBaseCollisionComponent::GetLocalExtremePoint ( const FVector & LocalDirection ) const
    {
        // Преобразуем локальное направление в мировое с учетом поворота
    CActor * owner = GetOwnerActor ();
    if (owner)
        {
        FQuat rotation = owner->GetActorRotationQuat ();
        FVector worldDirection = rotation * LocalDirection;
        return GetExtremePoint ( worldDirection );
        }
    return GetExtremePoint ( LocalDirection );
    }