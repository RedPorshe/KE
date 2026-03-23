#include <KE/Engine.h>
#include <KE/GameFramework/GameInstance.h>
#include <CoreMinimal.h>
#include "test.h"

int main ( int argc, char * argv [] )
	{
	setlocale ( LC_ALL, "ru_RU.UTF-8" );

	LOG_INIT ( "Game", false, true );
#ifdef _DEBUG
	LOG_SET_LEVEL ( CE::CLogger::CLogLevel::TRACE );
#else
	LOG_SET_LEVEL ( CE::CLogger::CLogLevel::TRACE );
#endif // _DEBUG
	CEngine Engine {};
	int ErrorLevel = Engine.PreInit (argc,argv );
	if (ErrorLevel == 0)
		{		
		ErrorLevel = 0;
		if (ErrorLevel == Engine.Init ())
			{
			CreateTestWorld ();
			auto GI = CEngine::Get ().GetGameInstance ();
			if(GI)GI->Init ();
			Engine.Run ();
			}
		}
	Engine.Shutdown ();
	LOG_SHUTDOWN ();
	return ErrorLevel;
	}