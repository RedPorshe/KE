#pragma once
#include "Core/Object.h"
#include <vector>
#include <memory>
#include "KE/Vulkan/RenderInfo.h"
// Forward declarations
class CGameInstance;
class CLevel;
class CGameMode;


class KE_API CWorld : public CObject
    {
    CHUDDO_DECLARE_CLASS ( CWorld, CObject )

    private:
        CGameInstance * OwningGameInstance = nullptr;
        std::vector<CLevel *> Levels;
        CLevel * CurrentLevel = nullptr;

        // GAME MODE - принадлежит World!
        CGameMode * CurrentGameMode = nullptr;

        // Состояние
        bool bIsPlaying = false;
        float CurrentDeltaTime = 0.0f;

    public:
        CWorld ( CObject * inOwner = nullptr, const std::string & displayName = "World" );
        virtual ~CWorld ();

        // ========== GAME INSTANCE ACCESS ==========
        CGameInstance * GetGameInstance () const { return OwningGameInstance; }
        CWorld * GetWorld () { return this; }
        float GetDeltaSeconds () const { return CurrentDeltaTime; }

        // ========== LEVEL MANAGEMENT ==========
        void AddLevel ( CLevel * level );
        bool RemoveLevel ( const std::string & levelName );
        bool RemoveLevel ( CLevel * level );
        void SetCurrentLevel ( CLevel * level );
        CLevel * GetCurrentLevel () const { return CurrentLevel; }
        size_t GetNumLevels () const { return Levels.size (); }
        bool HasLevels () const { return !Levels.empty (); }

        // ========== RENDER INFO ==========
        void CollectRenderInfo (FRenderInfo * Info);
        FCameraInfo FindActiveCamera ();

        // ========== GAME MODE MANAGEMENT ==========
        template<typename GameModeType, typename... Args>
        GameModeType * CreateGameMode ( const std::string & name = "GameMode", Args&&... args );

        void SetGameMode ( CGameMode * NewGameMode );
        CGameMode * GetGameMode () const { return CurrentGameMode; }

        // ========== WORLD LIFECYCLE ==========
        virtual void BeginPlay ();
        virtual void Tick ( float deltaTime );
        virtual void EndPlay ();

        // ========== SEARCH/QUERY ==========
        CObject * FindObjectByName ( const std::string & name ) const;
        CObject * FindObjectByUUID ( const std::string & uuid ) const;

        // ========== DEBUG/UTILS ==========
        void DumpState () const;

        template<typename LevelType, typename... Args>
        LevelType * CreateLevel ( const std::string & name = "Level", Args&&... args );

    protected:
        bool HasAnyActorWithDebugCollisions () const;
        FCameraInfo CachedCamera {};
    };

#include "KE/GameFramework/GameMode.h"

template<typename GameModeType, typename... Args>
GameModeType * CWorld::CreateGameMode ( const std::string & name, Args&&... args )
    {
    static_assert( std::is_base_of<CGameMode, GameModeType>::value,
                   "GameMode type must be derived from CGameMode" );

     // Удаляем старый GameMode если есть
    if (CurrentGameMode)
        {
        LOG_DEBUG ( "[WORLD] Replacing existing GameMode: ", CurrentGameMode->GetName () );
        if (bIsPlaying)
            {
            CurrentGameMode->EndGame ();
            }
        RemoveOwnedObject ( CurrentGameMode->GetName () );
        CurrentGameMode = nullptr;
        }

        // World создает GameMode (владелец - World!)
    GameModeType * NewGameMode = this->AddSubObject<GameModeType> ( name, std::forward<Args> ( args )... );

    if (NewGameMode)
        {
        SetGameMode ( NewGameMode );
        NewGameMode->SetWorld ( this );
        NewGameMode->InitGame ();

        LOG_DEBUG ( "[WORLD] GameMode created: ", NewGameMode->GetName (),
                    " (Class: ", NewGameMode->GetObjectClassName (), ")" );
        }

    return NewGameMode;
    }

template<typename LevelType, typename... Args>
LevelType * CWorld::CreateLevel ( const std::string & name, Args&&... args )
    {
    static_assert( std::is_base_of<CLevel, LevelType>::value,
                   "Level type must be derived from CLevel" );

    LevelType * newLevel = this->AddSubObject<LevelType> ( name, std::forward<Args> ( args )... );
    if (!newLevel)
        {
        LOG_ERROR ( "[WORLD] Error: Failed to spawn level '", name, "'" );
        return nullptr;
        }

    newLevel->OwningWorld = this;
    Levels.push_back ( newLevel );

    if (CurrentLevel == nullptr)
        {
        SetCurrentLevel ( newLevel );
        }

    return newLevel;
    }

REGISTER_CLASS_FACTORY ( CWorld );