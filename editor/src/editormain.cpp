#include <KE/Engine.h>
#include <CoreMinimal.h>


int main ( int argc, char * argv [] )
	{
	setlocale ( LC_ALL, "ru_RU.UTF-8" );
	LOG_INIT ( "Editor", false, true );
#ifdef _DEBUG
	LOG_SET_LEVEL ( CE::CLogger::CLogLevel::TRACE );
#else
	LOG_SET_LEVEL ( CE::CLogger::CLogLevel::INFO );
#endif // _DEBUG

	CEngine Engine {};
	int ErrorLevel = Engine.PreInit ( argc, argv );
	if (ErrorLevel == 0)
		{
		LOG_INFO ( "Starting Editor init engine"); // in game another initializing
		}
	LOG_SHUTDOWN ();
	return ErrorLevel;
	}