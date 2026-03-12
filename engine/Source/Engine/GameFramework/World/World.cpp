#include "World/World.h"
#include "GameFramework/GameInstance.h"
#include "GameFramework/World/Level.h"
#include "GameFramework/GameMode.h"
#include "Render/RenderInfo.h"
#include "Render/Window.h"
#include "Actors/Actor.h"
#include "Camera/CameraComponent.h"
#include "Core/Engine.h"
#include <algorithm>

CWorld::CWorld ( CObject * inOwner, const std::string & displayName )
	: Super ( inOwner, displayName )
	{
	OwningGameInstance = dynamic_cast< CGameInstance * >( inOwner );

	
	
	}

CWorld::~CWorld ()
	{
	DumpState ();

	if (bIsPlaying)
		{
		EndPlay ();
		}

	CurrentGameMode = nullptr;
	Levels.clear ();
	CurrentLevel = nullptr;
	OwningGameInstance = nullptr;

	}

	// ========== LEVEL MANAGEMENT ==========

void CWorld::AddLevel ( CLevel * level )
	{
	if (!level) return;

	level->OwningWorld = this;
	Levels.push_back ( level );

	LOG_DEBUG ( "[WORLD] Level added: ", level->GetName (),
				" (Total levels: ", Levels.size (), ")" );
	}

bool CWorld::RemoveLevel ( const std::string & levelName )
	{
	auto level = FindObjectByName ( levelName );
	if (!level)
		{
		LOG_WARN ( "[WORLD] Level not found: ", levelName );
		return false;
		}

	CLevel * levelPtr = dynamic_cast< CLevel * >( level );
	if (!levelPtr)
		{
		LOG_ERROR ( "[WORLD] ERROR: Object '", levelName, "' is not a CLevel!" );
		return false;
		}

	auto it = std::find ( Levels.begin (), Levels.end (), levelPtr );
	if (it == Levels.end ())
		{
		LOG_ERROR ( "[WORLD] ERROR: Level '", levelName, "' not found in Levels vector!" );
		return false;
		}

	if (CurrentLevel == levelPtr)
		{
		LOG_DEBUG ( "[WORLD] Removing current level: ", levelName );

		if (Levels.size () > 1)
			{
			auto levelIndex = std::distance ( Levels.begin (), it );
			if (levelIndex < static_cast< int > ( Levels.size () ) - 1)
				{
				SetCurrentLevel ( Levels[ levelIndex + 1 ] );
				}
			else
				{
				SetCurrentLevel ( Levels[ levelIndex - 1 ] );
				}
			}
		else
			{
			SetCurrentLevel ( nullptr );
			}
		}

	if (bIsPlaying)
		{
		levelPtr->EndPlay ();
		}

	Levels.erase ( it );
	levelPtr->OwningWorld = nullptr;

	bool removed = RemoveOwnedObject ( levelName );
	if (removed)
		{
		LOG_DEBUG ( "[WORLD] Level removed: ", levelName,
					" (Remaining levels: ", Levels.size (), ")" );
		}

	return removed;
	}

bool CWorld::RemoveLevel ( CLevel * level )
	{
	if (!level) return false;

	if (level->OwningWorld != this)
		{
		LOG_ERROR ( "[WORLD] ERROR: Level '", level->GetName (),
					"' does not belong to this world!" );
		return false;
		}

	return RemoveLevel ( level->GetName () );
	}

void CWorld::SetCurrentLevel ( CLevel * level )
	{
	if (!level)
		{
		if (CurrentLevel)
			{
			LOG_DEBUG ( "[WORLD] Current level cleared. Was: ", CurrentLevel->GetName () );
			}
		CurrentLevel = nullptr;
		return;
		}

	bool belongsToWorld = false;
	for (auto lvl : Levels)
		{
		if (lvl == level)
			{
			belongsToWorld = true;
			break;
			}
		}

	if (!belongsToWorld)
		{
		LOG_ERROR ( "[WORLD] ERROR: Level '", level->GetName (),
					"' does not belong to this world!" );
		return;
		}

	if (CurrentLevel == level)
		{
		LOG_WARN ( "[WORLD] Level '", level->GetName (), "' is already current" );
		return;
		}

	if (bIsPlaying && CurrentLevel)
		{
		CurrentLevel->EndPlay ();
		}

	CurrentLevel = level;

	if (bIsPlaying)
		{
		level->BeginPlay ();
		}

	LOG_DEBUG ( "[WORLD] Current level set to: ", level->GetName () );
	}

	// ========== GAME MODE MANAGEMENT ==========

void CWorld::SetGameMode ( CGameMode * NewGameMode )
	{
	if (CurrentGameMode == NewGameMode) return;

	if (CurrentGameMode)
		{
		if (bIsPlaying)
			{
			CurrentGameMode->EndGame ();
			}
		RemoveOwnedObject ( CurrentGameMode->GetName () );
		}

	CurrentGameMode = NewGameMode;

	if (CurrentGameMode)
		{
		CurrentGameMode->SetWorld ( this );
		if (bIsPlaying)
			{
			CurrentGameMode->StartPlay ();
			}
		LOG_DEBUG ( "[WORLD] GameMode set to: ", CurrentGameMode->GetName () );
		}
	}



void CWorld::BeginPlay ()
	{
	if (bIsPlaying)
		{
		LOG_WARN ( "[WORLD] World is already playing!" );
		return;
		}

	bIsPlaying = true;
	LOG_DEBUG ( "[WORLD] BeginPlay: ", GetName () );

	if (!CurrentGameMode)
		{
		LOG_WARN ( "[WORLD] No GameMode found, creating default GameMode" );
		CreateGameMode<CGameMode> ( "GameModeBase" );
		}

	if (CurrentGameMode)
		{
		CurrentGameMode->StartPlay ();
		}

	for (auto & level : Levels)
		{
		level->BeginPlay ();
		}
	}

void CWorld::Tick ( float deltaTime )
	{
	CurrentDeltaTime = deltaTime;

	if (!bIsPlaying) return;

	// Tick GameMode
	if (CurrentGameMode)
		{
		CurrentGameMode->Tick ( deltaTime );
		}

		// Tick текущий уровень
	if (CurrentLevel)
		{
		CurrentLevel->Tick ( deltaTime );
		}
	}

void CWorld::EndPlay ()
	{
	if (!bIsPlaying) return;

	bIsPlaying = false;
	LOG_DEBUG ( "[WORLD] EndPlay: ", GetName () );

	// Завершаем GameMode
	if (CurrentGameMode)
		{
		CurrentGameMode->EndGame ();
		}

		// Завершаем уровни
	for (auto & level : Levels)
		{
		level->EndPlay ();
		}
	}

	// ========== RENDER INFO ==========

void CWorld::CollectRenderInfo ( FRenderInfo * Info )
	{
   // FRenderInfo Info {};   
	Info->Camera = FindActiveCamera ();


	if (CurrentLevel)
		{
		const auto & actors = CurrentLevel->GetActors ();

		for (CActor * actor : actors)
			{
			if (!actor || actor->IsHiddenInGame ()) continue;

			FRenderCollection renderCollection = actor->GetRenderInfo ();


			for (const auto & mesh : renderCollection.Meshes)
				{
				if (mesh.IsValid ())
					{
					Info->AddMesh ( mesh );
					}
				}

			for (const auto & terrain : renderCollection.Terrains)
				{
				if (terrain.IsValid ())
					{
					Info->AddTerrain ( terrain );
					}
				}


			for (const auto & collision : renderCollection.DebugCollisions)
				{
				if (collision.IsValid ())
					{
					Info->AddDebugCollision ( collision );
					}
				}

			for (const auto & TerWire : renderCollection.TerrainWireframes)
				{
				Info->AddTerrainWireframe ( TerWire );
				}
			}
		}

	Info->HasInfo = !Info->RenderMeshes.empty () || !Info->Terrains.empty ();
	Info->bDrawCollisions = HasAnyActorWithDebugCollisions ();

	if (!Info->HasInfo)
		{
		static bool bIsShowed = false;
		if(!bIsShowed)
			{
			LOG_ERROR ( "World not set Info for render" );
			bIsShowed = true;
			}
		}
	}

bool CWorld::HasAnyActorWithDebugCollisions () const
	{
	if (!CurrentLevel) return false;

	for (CActor * actor : CurrentLevel->GetActors ())
		{
		if (actor && actor->IsDrawCollisionsEnabled ())
			return true;
		}
	return false;
	}

FCameraInfo CWorld::FindActiveCamera ()
	{
	FCameraInfo Info {};

	// Сначала ищем активную камеру в уровне
	if (CurrentLevel)
		{
		const auto & actors = CurrentLevel->GetActors ();
		for (CActor * actor : actors)
			{
			CCameraComponent * camera = actor->FindComponent<CCameraComponent> ();
			if (camera && camera->IsVisible ())
				{
				float aspectRatio = CEngine::Get ().GetWindow ()->GetAspectRatio ();
				if (aspectRatio <= 0.0f) aspectRatio = 16.0f / 9.0f;

				Info = camera->GetCameraInfo ( aspectRatio );
				
				return Info;
				}
			}
		}

	LOG_WARN ("Camera not found use fallback camera");
	float aspectRatio = CEngine::Get ().GetWindow ()->GetAspectRatio ();
	if (aspectRatio <= 0.0001f) aspectRatio = 16.0f / 9.0f;

	
	Info.Location = { 500.f, 200.f, 300.f };  
	Info.ViewTarget = { 500.f, 43.f, 500.f }; 
	Info.NearPlane = 0.1f;
	Info.FarPlane = 2000.f;  
	Info.FOV = 90.f;

	
	Info.ViewMatrix = FMat4::LookAtMatrix (
		Info.Location,
		Info.ViewTarget,
		FVector::Up ()  
	);

	Info.ProjectionMatrix = FMat4::PerspectiveMatrix (
		Info.FOV * CEMath::DEG_TO_RAD,
		aspectRatio,
		Info.NearPlane,
		Info.FarPlane
	);
	
	return Info;
	}

	// ========== SEARCH/QUERY ==========

CObject * CWorld::FindObjectByName ( const std::string & name ) const
	{
	for (const auto & level : Levels)
		{
		CObject * found = level->FindObjectByName ( name );
		if (found)
			return found;
		}

	return nullptr;
	}

CObject * CWorld::FindObjectByUUID ( const std::string & uuid ) const
	{
	for (const auto & level : Levels)
		{
		CObject * found = level->FindObjectByUUID ( uuid );
		if (found)
			return found;
		}

	return nullptr;
	}

	// ========== DEBUG/UTILS ==========

void CWorld::DumpState () const
	{
	LOG_DEBUG ( "=== WORLD STATE ===" );
	LOG_DEBUG ( "Name: ", GetName () );
	LOG_DEBUG ( "UUID: ", GetShortUUID () );
	LOG_DEBUG ( "GameInstance: ", ( OwningGameInstance ? OwningGameInstance->GetName () : "None" ) );
	LOG_DEBUG ( "Is Playing: ", ( bIsPlaying ? "Yes" : "No" ) );
	LOG_DEBUG ( "Current Level: ", ( CurrentLevel ? CurrentLevel->GetName () : "None" ) );
	LOG_DEBUG ( "Has GameMode: ", ( CurrentGameMode ? "Yes" : "No" ) );

	if (CurrentGameMode)
		{
		LOG_DEBUG ( "GameMode: ", CurrentGameMode->GetName () );
		LOG_DEBUG ( "GameMode Class: ", CurrentGameMode->GetObjectClassName () );
		}

	LOG_DEBUG ( "Total Levels: ", Levels.size () );

	for (size_t i = 0; i < Levels.size (); ++i)
		{
		LOG_DEBUG ( "  [", i, "] ", Levels[ i ]->GetName (),
					" (Active: ", ( Levels[ i ] == CurrentLevel ? "Yes" : "No" ), ")" );
		}

	LOG_DEBUG ( "===================" );
	}