// Level.h - исправленная версия с работающей очередью
#pragma once
#include "Core/Object.h"
#include <vector>
#include <memory>
#include <deque>
#include <functional>
#include <string>
#include <limits>
#include "KE/Engine.h"

// Forward declarations
class CWorld;
class CActor;
class CTerrainActor;


class KE_API CLevel : public CObject
    {
    CHUDDO_DECLARE_CLASS ( CLevel, CObject )

    private:
        std::vector<CActor *> Actors;
        std::vector<CActor *> ActorsPendingToDestroy;

        using SpawnFunction = std::function<CActor * ( )>;
        struct SpawnRequest
            {
            std::string Name;
            SpawnFunction Func;

            SpawnRequest ( const std::string & name, SpawnFunction func )
                : Name ( name ), Func ( func ) { }
            };

        std::deque<SpawnRequest> SpawnQueue;
        size_t MaxActorsPerTick = 10;
        size_t ActorsSpawnedThisTick = 0; // Счетчик акторов, созданных в текущем тике

    public:
        CLevel ( CObject * owner = nullptr, const std::string & inName = "Level" );
        virtual ~CLevel ();

        virtual void BeginPlay ();
        virtual void Tick ( float DeltaTime );
        virtual void EndPlay ();

        CWorld * GetOwningWorld () const { return OwningWorld; }

        template<typename ActorType, typename... Args>
        ActorType * SpawnActor ( const std::string & name = "Actor",const FVector& SpawnLocation = FVector::Zero(), Args&&... args)
            {
            static_assert( std::is_base_of<CActor, ActorType>::value,
                           "ActorType must be derived from CActor" );

             // Проверяем, можем ли спавнить сразу (лимит не превышен)
            if (CanSpawnImmediately ())
                {
                ActorType* ActorToReturn = SpawnActorImmediate<ActorType> ( name, std::forward<Args> ( args )... );
                ActorToReturn->SetActorLocation ( SpawnLocation, true );
                ActorToReturn->GetRootComponent ()->UpdateTransform ();
                return ActorToReturn;
                }
            else
                {
                    // Добавляем в очередь
                EnqueueSpawnRequest<ActorType> ( name, std::forward<Args> ( args )... );
                return nullptr;
                }
            }

       

        void ProcessSpawnQueue ();
        void SetMaxActorsPerTick ( size_t max ) { MaxActorsPerTick = max; }
        size_t GetMaxActorsPerTick () const { return MaxActorsPerTick; }
        size_t GetPendingSpawnCount () const { return SpawnQueue.size (); }
        bool HasPendingSpawns () const { return !SpawnQueue.empty (); }

        void ClearSpawnQueue () { SpawnQueue.clear (); }
        void ProcessAllPendingSpawns ();
        void SkipNextSpawnTick () { bSkipNextSpawnTick = true; }
    private:
        bool PendingDestoyActor ( CActor * actor );
        bool PendingDestoyActor ( const std::string & actorName );
        void CollectAllPendingActors ();
        void ProccessPendingActors ();
    public:

        CTerrainActor * SpawnTerrainActor ( const std::string & name = "Terrain",
                                            int32 width = 100,
                                            int32 height = 100,
                                            float cellSize = 100.0f,
                                            float heightValue = 0.0f );

          // Создание террейна из карты высот
        CTerrainActor * SpawnTerrainActorFromHeightmap ( const std::string & name,
                                                         const std::vector<float> & heights,
                                                         int32 width,
                                                         int32 height,
                                                         float cellSize );

        bool DestroyActor ( CActor * actor );
        bool DestroyActor ( const std::string & actorName );
        bool SpawnTerrain ();
        CActor * SpawnActorAtLocation ( const std::string & ClassName, const std::string & ActorName, const FVector & loc );
        CActor* SpawnActorByClass ( const std::string & ClassName, const std::string & ActorName,
                                    const FVector & SpawnLocation = FVector::Zero () );

        const std::vector<CActor *> & GetActors () const { return Actors; }
        size_t GetNumActors () const { return Actors.size (); }

        CObject * FindObjectByName ( const std::string & name ) const;
        CObject * FindObjectByUUID ( const std::string & uuid );

        template<typename ActorType>
        std::vector<ActorType *> GetAllActorsOfClass () const
            {
            static_assert( std::is_base_of<CActor, ActorType>::value,
                           "Class must be derived from CActor" );

            std::vector<ActorType *> result;
            result.reserve ( Actors.size () );

            for (CActor * actor : Actors)
                {
                if (ActorType * typedActor = dynamic_cast< ActorType * >( actor ))
                    {
                    result.push_back ( typedActor );
                    }
                }

            return result;
            }

        template<typename ActorType>
        ActorType * FindActorByType () const
            {
            auto actor = CObject::FindObjectByType<ActorType> ();
            ActorType * typedActor = dynamic_cast< ActorType * >( actor );
            {
            return typedActor;
            }

            return nullptr;
            }

        virtual void DumpState () const;

        CWorld * OwningWorld = nullptr;

    private:
        bool bIsPlaying = false;
        bool bSkipNextSpawnTick = false;
        std::vector< CTerrainActor *> Terrains;
        bool RemoveActorFromVector ( CActor * actor );

        // Проверка возможности немедленного спавна
        bool CanSpawnImmediately () const
            {
                // Можем спавнить, если создали менее MaxActorsPerTick в этом тике
                // И очередь пуста (нет ожидающих запросов)
            return ( ActorsSpawnedThisTick < MaxActorsPerTick ) && SpawnQueue.empty ();
            }

        template<typename ActorType, typename... Args>
        ActorType * SpawnActorImmediate ( const std::string & name, Args&&... args )
            {
            ActorType * newActor = this->AddSubObject<ActorType> ( name, std::forward<Args> ( args )... );

            if (!newActor)
                {
                LOG_ERROR ( "[LEVEL] Error: Failed to spawn actor '", name, "'" );
                return nullptr;
                }

            Actors.push_back ( newActor );
            ActorsSpawnedThisTick++; // Увеличиваем счетчик
            
            if (auto * transform = newActor->GetRootComponent ())
                {
                transform->UpdateTransform ();  // Вызываем немедленно
                }

            if (bIsPlaying)
                {
                newActor->BeginPlay ();      
               
                }

            return newActor;
            }

        template<typename ActorType, typename... Args>
        void EnqueueSpawnRequest ( const std::string & name, Args&&... args )
            {
                // Создаем лямбду для отложенного спавна
            auto spawnFunc = [ this, name, args... ] () -> CActor *
                {
                return this->AddSubObject<ActorType> ( name, args... );
                };

            SpawnQueue.emplace_back ( name, spawnFunc );

            LOG_DEBUG ( "[LEVEL] Enqueued spawn request for '", name,
                        "'. Queue size: ", SpawnQueue.size () );
            }
    };


    REGISTER_CLASS_FACTORY ( CLevel );