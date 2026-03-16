#include <KE/Engine.h>
#include <CoreMinimal.h>

int main ( int argc, char * argv [] )
	{
	LOG_INIT ( "Game", false, true );
#ifdef _DEBUG
	LOG_SET_LEVEL ( CE::CLogger::CLogLevel::TRACE );
#else
	LOG_SET_LEVEL ( CE::CLogger::CLogLevel::INFO );
#endif // _DEBUG
	CEngine Engine {};
	int ErrorLevel = Engine.PreInit (argc,argv );
	if (ErrorLevel == 0)
		{
		LOG_INFO (  "Starting Game init engine"); // in editor another initializing
		ErrorLevel = Engine.Init ();
		if (ErrorLevel == 0)
			{
			Engine.Run ();
			}
		}
	Engine.Shutdown ();
	LOG_SHUTDOWN ();
	return ErrorLevel;
	}