#include <KE/Core/Engine.h>
#include <KE/Core/EngineInfo.h>
#include <CoreMinimal.h>
#include <GameFramework/GameInstance.h>
#include <GameFramework/World/World.h>
#include <GameFramework/World/Level.h>
#include <Actors/PlayerStart.h>
#include <Actors/TerrainActor.h>



int main () {
    setlocale ( LC_ALL, "ru" );

    LOG_INIT ( "KE_Game", false, true );
    LOG_SET_LEVEL ( CE::CLogger::CLogLevel::DEBUG );
    FEngineInfo engineInfo;
    engineInfo.EngineName = "KE Engine";
    engineInfo.WindowInfo.Title = "KE Game";
    engineInfo.WindowInfo.Width = 1280;
    engineInfo.WindowInfo.Height = 720;

    if (!CEngine::InitializeEngine ( engineInfo ))
        {
        LOG_FATAL ( "Failed to initialize engine!" );
        return EXIT_FAILURE;
        }

    auto & GI = CEngine::Get ().GetGameInstance ();
    auto world = GI.CreateWorld ();
    if (world == nullptr)
        {
        CEngine::ShutdownEngine ();

        LOG_SHUTDOWN ();
        return EXIT_FAILURE;
        }
    auto gm = world->CreateGameMode<CGameMode> ();
    world->SetGameMode ( gm );
    auto level = world->CreateLevel<CLevel> ();
    auto Ps = level->SpawnActor<CPlayerStart> ();
    auto terra = level->SpawnActor<CTerrainActor> ( "Terrain" );
    terra->GenerateHilly ( 25, 25, 6.f );
    terra->SetActorLocation ( { 0.f, 0.f, 0.f }, true );
    Ps->SetActorLocation ( 0.f, 200.f, 0.f, true );
    gm->SetDefaultPawnClass ( "MyCharacter" );
    
    GI.Init ();

    CEngine::Get ().Start ();
    CEngine::ShutdownEngine ();

    LOG_SHUTDOWN ();
    return EXIT_SUCCESS;
    }