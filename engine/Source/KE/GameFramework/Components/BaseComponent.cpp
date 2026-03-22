#include "KE/GameFramework/Components/BaseComponent.h"
#include "KE/GameFramework/Actors/Actor.h"




CBaseComponent::CBaseComponent ( CObject * owner, const std::string & inName ) : CObject ( owner, inName )
	{	
	 // Ищем актора в цепочке владельцев
	CObject * current = owner;
	while (current && !ActorOwner)
		{
		ActorOwner = dynamic_cast< CActor * >( current );
		current = current->GetOwner ();
		}
	}

CBaseComponent::~CBaseComponent ()
	{
	
	}

void CBaseComponent::InitComponent ()
	{
	if (bIsInitialized)
		{		
		return;
		}
	bIsInitialized = true;
	}

void CBaseComponent::Tick ( float DeltaTime )
	{
	if (!bIsComponentTick) return;

	if (!bIsInitialized)
		{
		static int warnCount = 0;
		if (warnCount++ < 3) // Лимитируем предупреждения
			{
			LOG_WARN ( "Component '" , GetName ()
				, "' not initialized, skipping tick");
			}
		return;
		}

		// Тикаем собственные компоненты
	for (auto comp : OwnedComponents)
		{
		if (comp && comp->CanTick ())
			{
			comp->Tick ( DeltaTime );
			}
		}
		
	for (auto actor : AttachedActors)
		{
		if (actor && actor->IsCanTickOnAttached () && actor->IsAttached ())
			{
			actor->Tick ( DeltaTime );
			}
		}	

	
	}

bool CBaseComponent::CanTick () const
	{
	return bIsComponentTick && bIsInitialized;
	}

void CBaseComponent::OnBeginPlay ()
	{
	if (bIsAutoInit)
		{
		InitComponent ();
		}
	//over funcs on begin play can place here...
	}

void CBaseComponent::SetPrimaryTick ( bool value )
	{
	bIsComponentTick = value;
	}

void CBaseComponent::SetAutoInitialize ( bool value )
	{
	bIsAutoInit = value;
	}

CActor * CBaseComponent::GetOwnerActor ()
	{
	 // Если ещё не нашли актора, ищем
	if (!ActorOwner)
		{
		CObject * current = GetOwner ();
		while (current && !ActorOwner)
			{
			ActorOwner = dynamic_cast< CActor * >( current );
			current = current->GetOwner ();
			}
		}
	return ActorOwner;
	}

CActor * CBaseComponent::GetOwnerActor () const
	{
	if (ActorOwner)
		return ActorOwner;

	CObject * current = GetOwner ();
	while (current)
		{
		if (CActor * actor = dynamic_cast< CActor * >( current ))
			{
			return actor;
			}
		current = current->GetOwner ();
		}
	return nullptr;
	}

bool CBaseComponent::IsHaveOwnerActor ()
	{
	return GetOwnerActor () != nullptr;
	}

void CBaseComponent::AttachComponentToComponent ( CBaseComponent * CompToAttach )
	{
	if (!CompToAttach)
		{
		LOG_ERROR ( "Component to attach is nullptr");
		return;
		}

	if (CompToAttach == this)
		{
		LOG_ERROR ( "Cannot attach component to itself");
		return;
		}

		// Проверяем, не прикреплён ли уже
	auto it = std::find ( OwnedComponents.begin (), OwnedComponents.end (), CompToAttach );
	if (it != OwnedComponents.end ())
		{
		LOG_WARN ( "Component '" , CompToAttach->GetName (), "' already attached to this component");
		return;
		}

		// Проверяем, не создаст ли это циклическую ссылку
	if (WouldCreateCircularReference ( CompToAttach ))
		{
		LOG_ERROR( "Would create circular reference");
		return;
		}

		// Передаём владение
	if (CompToAttach->GetOwner ()->TransferOwnership ( CompToAttach, this ))
		{
		OwnedComponents.push_back ( CompToAttach );
		LOG_DEBUG(  "Success attach component '" , CompToAttach->GetName ()
			, "' to component '", GetName () , "'");
		}
	else
		{
		LOG_ERROR ("Failed to transfer ownership");
		}
	}

void CBaseComponent::AttachActorToComponent ( CActor * ActorToAttach )
	{
	if (!ActorToAttach)
		{
		LOG_ERROR( "Actor to attach is nullptr");
		return;
		}

		// Проверяем, не прикреплён ли уже
	auto it = std::find ( AttachedActors.begin (), AttachedActors.end (), ActorToAttach );
	if (it != AttachedActors.end ())
		{
		LOG_WARN( "Actor '" , ActorToAttach->GetName ()
			, "' already attached to this component");
		return;
		}

		// ЛОГИЧЕСКОЕ прикрепление (не владение!)
	AttachedActors.push_back ( ActorToAttach );
	

	// Если это SceneComponent, можно обновить трансформы
	if (CTransformComponent * SceneComp = dynamic_cast< CTransformComponent * >( this ))
		{
		if (CTransformComponent * ActorRoot = ActorToAttach->GetRootComponent ())
			{
				// Прикрепляем корневой компонент актора к этому компоненту
			ActorRoot->AttachComponentToComponent ( SceneComp );
			}
		}
	ActorToAttach->SetIsAttached ( true );
	LOG_DEBUG( "Success attach actor '" , ActorToAttach->GetName ()
		, "' to component '", this->GetName () , "'");
	}

bool CBaseComponent::WouldCreateCircularReference ( CBaseComponent * CompToAttach ) 
	{
		// Проверяем, не является ли CompToAttach нашим предком
	CObject * current = this;
	while (current)
		{
		if (current == CompToAttach)
			return true;

		CBaseComponent * comp = dynamic_cast< CBaseComponent * >( current );
		current = comp ? comp->GetOwner () : nullptr;
		}
	return false;
	}

void CBaseComponent::DetachFromParent ()
	{
	}


