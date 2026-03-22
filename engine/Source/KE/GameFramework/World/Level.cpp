// Level.cpp - исправленная версия
#include "KE/GameFramework/World/Level.h"
#include "KE/GameFramework/World/World.h"
#include "KE/GameFramework/Actors/Actor.h"
#include "KE/GameFramework/Actors/TerrainActor.h"
#include "KE/GameFramework/Components/Collisions/TerrainComponent.h"


#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

CLevel::CLevel ( CObject * owner, const std::string & inName )
	: CObject ( owner, inName )
	{
	OwningWorld = dynamic_cast< CWorld * >( owner );
	}

CLevel::~CLevel ()
	{
	DumpState ();
	if (bIsPlaying)
		{
		EndPlay ();
		}
	Actors.clear ();
	SpawnQueue.clear ();
	}

void CLevel::BeginPlay ()
	{
	if (bIsPlaying)
		{
		LOG_ERROR ( "[LEVEL] ERROR: Level is already playing!" );
		return;
		}

	bIsPlaying = true;
	LOG_DEBUG ( " BeginPlay for '", GetName (), "'" );
	for (CActor * actor : Actors)
		{
		if (actor)
			{
			actor->BeginPlay ();
			}
		}
	}

void CLevel::Tick ( float DeltaTime )
	{
	ProccessPendingActors ();
	if (!bIsPlaying)
		{
		LOG_WARN ( "[LEVEL] Level is not playing, skipping tick" );
		return;
		}

		// Сбрасываем счетчик созданных акторов в начале тика
	ActorsSpawnedThisTick = 0;

	if (!bSkipNextSpawnTick)
		{
		ProcessSpawnQueue ();
		}
	else
		{
		bSkipNextSpawnTick = false;
		LOG_DEBUG ( "[LEVEL] Skipping spawn processing for this tick" );
		}

	for (CActor * actor : Actors)
		{
		if (actor)
			{
			if (!actor->IsAttached ())
				{
				actor->Tick ( DeltaTime );
				}
			if (!actor->ActorStartedBeginPlay ())
				{
				actor->BeginPlay ();
				}
			}
		}
	CollectAllPendingActors ();
	CollectAllPendingActors ();
	}

void CLevel::EndPlay ()
	{
	if (!bIsPlaying)
		return;

	bIsPlaying = false;
	LOG_DEBUG ( "[LEVEL] End Play for '", GetName (), "'" );
	for (CActor * actor : Actors)
		{
		if (actor)
			{
			actor->EndPlay ();
			}
		}
	}

bool CLevel::RemoveActorFromVector ( CActor * actor )
	{
	if (!actor)
		return false;

	auto it = std::find ( Actors.begin (), Actors.end (), actor );
	if (it != Actors.end ())
		{
		Actors.erase ( it );
		return true;
		}
	return false;
	}

bool CLevel::PendingDestoyActor ( CActor * actor )
	{
	if (!actor)
		return false;

	if (!RemoveActorFromVector ( actor ))
		{
		LOG_ERROR ( "[LEVEL] Error: Actor not found in level: "
					, actor->GetName () );
		return false;
		}

	if (bIsPlaying)
		{
		actor->EndPlay ();
		}

	LOG_DEBUG ( "[LEVEL] Actor destroyed: ", actor->GetName () );

	auto it = std::find_if ( OwnedObjects.begin (), OwnedObjects.end (),
							 [ actor ] ( const std::unique_ptr<CObject> & obj )
							 {
							 return obj.get () == actor;
							 } );

	if (it != OwnedObjects.end ())
		{
		OwnedObjects.erase ( it );
		return true;
		}
	else
		{
		LOG_ERROR ( "[LEVEL] Warning: Actor not found in OwnedObjects: "
					, actor->GetName () );
		return false;
		}
	}

bool CLevel::PendingDestoyActor ( const std::string & actorName )
	{
	for (CActor * actor : Actors)
		{
		if (actor && actor->GetName () == actorName)
			{
			return PendingDestoyActor ( actor );
			}
		}

	LOG_ERROR ( "[LEVEL] Actor not found: ", actorName );
	return false;
	}

void CLevel::CollectAllPendingActors ()
	{
	ActorsPendingToDestroy.clear ();

	std::vector<CActor *> actorsCopy = Actors;

	for (auto act : actorsCopy)
		{
		if (act->IsPendingToDestroy ())
			{
			ActorsPendingToDestroy.push_back ( act );
			}
		}
	if (!ActorsPendingToDestroy.empty ())
		{
		int count = 1;
		LOG_DEBUG ( "Actors collected for Destroy List of actors:" );
		for (auto act : ActorsPendingToDestroy)
			{

			LOG_DEBUG ( count, ": ", act->GetName () );
			count++;
			}
		}
	}

bool CLevel::DestroyActor ( CActor * actor )
	{
	if (actor == nullptr)
		{
		LOG_ERROR ( "actor is NULL" );
		return false;
		}
	actor->SetPendingToDestroy ();
	return true;
	}

bool CLevel::DestroyActor ( const std::string & actorName )
	{
	for (CActor * actor : Actors)
		{
		if (actor && actor->GetName () == actorName)
			{
			return DestroyActor ( actor );
			}
		}

	LOG_ERROR ( "[LEVEL] Actor not found: ", actorName );
	return false;
	}



CActor * CLevel::SpawnActorAtLocation ( const std::string & ClassName, const std::string & ActorName, const FVector & loc )
	{
	return SpawnActorByClass ( ClassName, ActorName, loc );
	}

CActor * CLevel::SpawnActorByClass ( const std::string & ClassName, const std::string & ActorName,
									 const FVector & SpawnLocation )
	{
	CActor * NewActor = dynamic_cast< CActor * >( OBJECT_FACTORY.Create ( ClassName, this, ActorName ) );
	if (NewActor)
		{
		Actors.push_back ( NewActor );
		ActorsSpawnedThisTick++;

		
		NewActor->SetActorLocation ( SpawnLocation, true );  
		if (NewActor->GetRootComponent ())
			{
			NewActor->GetRootComponent ()->UpdateTransform ();
			}

		if (bIsPlaying)
			{
			NewActor->BeginPlay ();
			}
		return NewActor;
		}
	return nullptr;
	}

void CLevel::ProccessPendingActors ()
	{
	if (!ActorsPendingToDestroy.empty ())
		{
		LOG_DEBUG ( "Proccessing destroy Actors" );
		for (auto actor : ActorsPendingToDestroy)
			{
			if (actor)
				{
				std::string ActorName = actor->GetName ();
				PendingDestoyActor ( ActorName );
				}
			}
		}
	ActorsPendingToDestroy.clear ();
	}

CObject * CLevel::FindObjectByName ( const std::string & name ) const
	{
	if (GetName () == name)
		return const_cast< CLevel * >( this );

	for (CActor * actor : Actors)
		{
		if (actor && actor->GetName () == name)
			return actor;
		}
	for (CActor * actor : Actors)
		{
		if (actor)
			{
			for (auto comp : actor->GetActorComponents ())
				{
				if (comp && comp->GetName () == name)
					{
					return comp;
					}
				else if (comp)
					{
					auto found = comp->FindOwned ( name );
					if (found)
						{
						return found;
						}
					}
				}
			}
		}

	return nullptr;
	}

CObject * CLevel::FindObjectByUUID ( const std::string & uuid )
	{
	if (GetUUID () == uuid)
		return const_cast< CLevel * >( this );

	for (CActor * actor : Actors)
		{
		if (actor && actor->GetUUID () == uuid)
			{
			LOG_INFO ( "Founded actor: ", actor->GetName () );
			return actor;
			}
		}

	for (auto & obj : OwnedObjects)
		{
		auto founded = FindByUUIDRecursive ( uuid );
		if (founded)
			{
			LOG_INFO ( "Founded Object: ", founded->GetName () );
			return founded;
			}
		}

	return nullptr;
	}


void CLevel::ProcessSpawnQueue ()
	{
	if (SpawnQueue.empty ())
		return;

	size_t spawnedThisTick = 0;
	size_t availableSlots = MaxActorsPerTick - ActorsSpawnedThisTick;
	size_t totalToProcess = std::min ( SpawnQueue.size (), availableSlots );

	if (totalToProcess == 0)
		{
		LOG_DEBUG ( "[LEVEL] Cannot spawn more actors this tick. Limit reached (",
					ActorsSpawnedThisTick, "/", MaxActorsPerTick, ")." );
		return;
		}

	LOG_DEBUG ( "[LEVEL] Processing spawn queue. Pending: ",
				SpawnQueue.size (), ", Available slots: ", availableSlots,
				", Will process: ", totalToProcess );

	for (size_t i = 0; i < totalToProcess; i++)
		{
		if (SpawnQueue.empty ())
			break;

		SpawnRequest & request = SpawnQueue.front ();
		CActor * newActor = request.Func ();

		if (newActor)
			{
			Actors.push_back ( newActor );
			ActorsSpawnedThisTick++;

			if (bIsPlaying)
				{
				newActor->BeginPlay ();
				}

			LOG_DEBUG ( "[LEVEL] Spawned queued actor: ",
						newActor->GetName (),
						" (", spawnedThisTick + 1, "/", totalToProcess, " this tick)" );
			spawnedThisTick++;
			}

		SpawnQueue.pop_front ();
		}

	if (HasPendingSpawns ())
		{
		LOG_DEBUG ( "[LEVEL] Still have ", GetPendingSpawnCount (),
					" actors pending for next tick" );
		}
	}

void CLevel::ProcessAllPendingSpawns ()
	{
	if (SpawnQueue.empty ())
		return;

	LOG_DEBUG ( "[LEVEL] Processing all pending spawns: ",
				SpawnQueue.size (), " actors" );

	  // Временно увеличиваем лимит до максимума
	size_t originalLimit = MaxActorsPerTick;
	MaxActorsPerTick = std::numeric_limits<size_t>::max ();

	ProcessSpawnQueue ();

	// Восстанавливаем оригинальный лимит
	MaxActorsPerTick = originalLimit;
	}

void CLevel::DumpState () const
	{
	std::stringstream ss;
	ss << "=== LEVEL STATE ===" << "\n";
	ss << "Name: " << GetName () << "\n";
	ss << "UUID: " << GetShortUUID () << "\n";
	ss << "World: " << ( OwningWorld ? OwningWorld->GetName () : "None" ) << "\n";
	ss << "Is Playing: " << ( bIsPlaying ? "Yes" : "No" ) << "\n";
	ss << "Total Actors: " << Actors.size () << "\n";
	ss << "Pending Spawns: " << SpawnQueue.size () << "\n";
	ss << "Max Actors Per Tick: " << MaxActorsPerTick << "\n";
	ss << "Total Owned Objects: " << OwnedObjects.size () << "\n";

	ss << "Actors list:" << "\n";
	for (size_t i = 0; i < Actors.size (); ++i)
		{
		if (Actors[ i ])
			{
			ss << "  [" << std::setw ( 3 ) << i << "] " << Actors[ i ]->GetName ()
				<< " (" << Actors[ i ]->GetObjectClassName () << ")" << "\n";
			}
		else
			{
			ss << "  [" << std::setw ( 3 ) << i << "] NULL pointer!" << "\n";
			}
		}

	ss << "Pending spawn requests:" << "\n";
	if (SpawnQueue.empty ())
		{
		ss << "  None" << "\n";
		}
	else
		{
		size_t index = 0;
		for (const auto & request : SpawnQueue)
			{
			ss << "  [" << std::setw ( 3 ) << index++ << "] " << request.Name << "\n";
			}
		}

	ss << "Owned Objects list:" << "\n";
	for (size_t i = 0; i < OwnedObjects.size (); ++i)
		{
		if (OwnedObjects[ i ])
			{
			ss << "  [" << std::setw ( 3 ) << i << "] " << OwnedObjects[ i ]->GetName ()
				<< " (" << OwnedObjects[ i ]->GetObjectClassName () << ")" << "\n";
			}
		}

	ss << "===================";

	std::cout << ss.str () << std::endl;
	}


CTerrainActor * CLevel::SpawnTerrainActor ( const std::string & name,
											int32 width,
											int32 height,
											float cellSize,
											float heightValue )
	{
		// Создаём актор террейна
	CTerrainActor * terrainActor = SpawnActor<CTerrainActor> ( name );

	if (!terrainActor)
		{
		LOG_ERROR ( "[LEVEL] Failed to spawn TerrainActor: ", name );
		return nullptr;
		}

		// Получаем компонент террейна
	CTerrainComponent * terrainComp = terrainActor->GetTerrainComponent ();
	if (!terrainComp)
		{
		LOG_ERROR ( "[LEVEL] TerrainActor has no TerrainComponent!" );
		return nullptr;
		}

		// Генерируем плоский террейн
	terrainComp->GenerateFlat ( width, height, cellSize, heightValue );

	LOG_DEBUG ( "[LEVEL] TerrainActor spawned: ", name,
				" Size: ", width, "x", height,
				" CellSize: ", cellSize,
				" Height: ", heightValue );

	return terrainActor;
	}

CTerrainActor * CLevel::SpawnTerrainActorFromHeightmap ( const std::string & name,
														 const std::vector<float> & heights,
														 int32 width,
														 int32 height,
														 float cellSize )
	{
		// Создаём актор террейна
	CTerrainActor * terrainActor = SpawnActor<CTerrainActor> ( name );

	if (!terrainActor)
		{
		LOG_ERROR ( "[LEVEL] Failed to spawn TerrainActor: ", name );
		return nullptr;
		}

		// Получаем компонент террейна
	CTerrainComponent * terrainComp = terrainActor->GetTerrainComponent ();
	if (!terrainComp)
		{
		LOG_ERROR ( "[LEVEL] TerrainActor has no TerrainComponent!" );
		return nullptr;
		}

		// Загружаем террейн из карты высот
	terrainComp->GenerateFromHeightmap ( heights, width, height, cellSize );

	LOG_DEBUG ( "[LEVEL] TerrainActor spawned from heightmap: ", name,
				" Size: ", width, "x", height,
				" CellSize: ", cellSize,
				" Height range: ", terrainComp->GetTerrainData ().MinHeight,
				" - ", terrainComp->GetTerrainData ().MaxHeight );

	return terrainActor;
	}

bool CLevel::SpawnTerrain ()
	{
		// Упрощённый метод - создаём стандартный террейн
	return SpawnTerrainActor ( "Terrain", 100, 100, 100.0f, 0.0f ) != nullptr;
	}