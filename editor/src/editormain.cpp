#include <KE/Core/Engine.h>
#include <KE/Core/EngineInfo.h>
#include <CoreMinimal.h>

int main () {
    setlocale ( LC_ALL, "ru" );

    LOG_INIT ( "KE_Editor", false, true );
    LOG_SET_LEVEL (CE::CLogger::CLogLevel::DEBUG);
    FEngineInfo engineInfo;
    engineInfo.EngineName = "KE Editor";
    engineInfo.WindowInfo.Title = "KE Editor";
    engineInfo.WindowInfo.Width = 1600;
    engineInfo.WindowInfo.Height = 900;

    if (!CEngine::InitializeEngine ( engineInfo ))
        {
        LOG_FATAL ( "Failed to initialize editor engine!" );
        return EXIT_FAILURE;
        }

    CEngine::Get ().Start ();
    CEngine::ShutdownEngine ();

    LOG_SHUTDOWN ();
    return EXIT_SUCCESS;
    }