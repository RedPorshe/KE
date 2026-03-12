#include "GameInstance.h"
#include "World/World.h"
#include "GameFramework/GameMode.h"
#include "Core/Engine.h"



#include <filesystem>
#include <iostream>
#include <rapidjson/rapidjson.h>


// Статический член
CGameInstance * CGameInstance::Instance = nullptr;

CGameInstance::CGameInstance ( CObject * owner, const std::string & displayName )
	: Super ( owner, displayName )
	{
	LOG_DEBUG ( "Gameinstance created : ", GetName () );
	}

CGameInstance::~CGameInstance ()
	{
	if (CurrentWorld)
		{
		CurrentWorld = nullptr;
		}

	if (Instance == this)
		{
		Instance = nullptr;
		}
	LOG_INFO ( "[GAME] GameInstance destroyed: ", GetName () );
	}

	// ========== SINGLETON METHODS ==========

CGameInstance & CGameInstance::Get ()
	{
	if (!Instance)
		{
		LOG_ERROR ( "[ERROR] GameInstance not created! Call Create() first." );
		Create ();
		}
	return *Instance;
	}

bool CGameInstance::Create ()
	{
	if (Instance)
		{
		LOG_WARN ( "[WARNING] GameInstance already exists!" );
		return false;
		}

	Instance = new CGameInstance ( nullptr, "MainGameInstance" );
	return true;
	}

void CGameInstance::Destroy ()
	{
	if (Instance)
		{
		delete Instance;
		Instance = nullptr;
		LOG_DEBUG ( "[GAME] GameInstance destroyed" );
		}
	}

	// ========== WORLD MANAGEMENT ==========

CWorld * CGameInstance::CreateWorld ( const std::string & worldName )
	{
	if (CurrentWorld)
		{
		LOG_ERROR ( "[GAME] World already exists! Destroy current world first." );
		return nullptr;
		}

	CurrentWorld = new CWorld ( this, worldName );
	AddOwnedObject ( CurrentWorld );
	
	LOG_DEBUG ( "[GAME] World created: ", worldName );
	return CurrentWorld;
	}

bool CGameInstance::DestroyWorld ()
	{
	if (CurrentWorld)
		{
		if (RemoveOwnedObject ( CurrentWorld->GetName () ))
			{
			CurrentWorld = nullptr;
			LOG_DEBUG ( "[GAME] World destroyed" );
			return true;
			}
		else
			{
			LOG_ERROR ( "Something wrong with Remove owned world in GameInstance. Setting CurrentWorld = nullptr; Uniq_ptr delete automatic" );
			CurrentWorld = nullptr;
			return true;
			}
		}
	return false;
	}

CEngine & CGameInstance::GetEngine ()
	{
	return CEngine::Get ();
	}

	// ========== GAME LIFECYCLE ==========

void CGameInstance::Init ()
	{
	LOG_DEBUG ( "[GAME] Initializing GameInstance..." );
	GameTime = 0.0f;
	DeltaTime = 0.0f;

	// СОЗДАЕМ WORLD (НО НЕ GAME MODE!)
	if (!CurrentWorld)
		{
		CreateWorld ( "MainWorld" );
		}

		// World сам создаст GameMode в BeginPlay!
	if (CurrentWorld)
		{
		CurrentWorld->BeginPlay ();
		}
	}

void CGameInstance::Tick ( float deltaTime )
	{
	DeltaTime = deltaTime;
	GameTime += deltaTime;

	// Tick world если существует
	if (CurrentWorld)
		{
		CurrentWorld->Tick ( deltaTime );
		/*static int SafeBeginPlay = 0;
		if (SafeBeginPlay <= 3)
			SafeBeginPlay++;
		if (SafeBeginPlay == 4)
			{
			LOG_DEBUG ( "Safe begin play after 3 ticks" );
			CurrentWorld->BeginPlay ();
			SafeBeginPlay++;
			}
		}*/
		}
	}

void CGameInstance::Shutdown ()
	{
	LOG_DEBUG ( "[GAME] Shutting down GameInstance..." );
	DumpState ();

	if (CurrentWorld)
		{
		DestroyWorld ();
		}
	}
 
void CGameInstance::SaveGameInstanceState ()
	{
	LOG_DEBUG ( "Proccessing save gameinstance '", GetName (), "' state " );
	}

void CGameInstance::DumpState () const
	{
	LOG_DEBUG ( "\n=== GAME INSTANCE STATE ===" );
	LOG_DEBUG ( "Name: ", GetName () );
	LOG_DEBUG ( "UUID: ", GetShortUUID () );
	LOG_DEBUG ( "Game Time: ", GameTime, "s" );
	LOG_DEBUG ( "Delta Time: ", DeltaTime, "s" );
	LOG_DEBUG ( "Has World: ", ( CurrentWorld ? "Yes" : "No" ) );


	if (CurrentWorld)
		{
		LOG_DEBUG ( "World: ", CurrentWorld->GetName () );
		}



	LOG_DEBUG ( "Child Objects: ", GetNumOwnedObjects () );
	LOG_DEBUG ( "===========================" );
	}