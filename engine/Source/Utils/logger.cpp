#include "Utils/Logger.h"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#define GETPID _getpid
#else
#include <thread>
#include <unistd.h>
#include <limits.h>
#define GETPID getpid
#endif

namespace CE
	{

// Вспомогательная функция для безопасного получения localtime
	std::tm safe_localtime ( const std::time_t & time ) {
		std::tm tm_snapshot;

#if defined(_MSC_VER) || defined(__MINGW32__)
	// Windows версия
		localtime_s ( &tm_snapshot, &time );
#else
	// Unix/Linux версия
		localtime_r ( &time, &tm_snapshot );
#endif

		return tm_snapshot;
		}

	// Получение пути к исполняемому файлу
	std::string CLogger::getExecutablePath () {
#ifdef _WIN32
		char buffer[ MAX_PATH ];
		GetModuleFileNameA ( NULL, buffer, MAX_PATH );
		std::string::size_type pos = std::string ( buffer ).find_last_of ( "\\/" );
		return std::string ( buffer ).substr ( 0, pos );
#else
		char buffer[ PATH_MAX ];
		ssize_t len = readlink ( "/proc/self/exe", buffer, sizeof ( buffer ) - 1 );
		if (len != -1)
			{
			buffer[ len ] = '\0';
			std::string::size_type pos = std::string ( buffer ).find_last_of ( "/" );
			return std::string ( buffer ).substr ( 0, pos );
			}
		return "";
#endif
		}

	CLogger::CLogger () {
		stats_.reset ();
		}

	CLogger & CLogger::getInstance () {
		static CLogger instance;
		return instance;
		}

	CLogger::~CLogger ()
		{
		shutdown ();
		}

	void CLogger::shutdown () {
		std::lock_guard<std::mutex> lock ( logMutex_ );

		if (shuttingDown_ || !initialized_)
			{
			return;
			}

		shuttingDown_ = true;

		// Записываем статистику
		writeStatistics ();

		// Закрываем файл
		if (logFile_.is_open ())
			{
			logFile_ << "\n=== Logging session ended at " << getCurrentTimestamp () << " ===\n";
			logFile_.close ();
			}

		initialized_ = false;

		if (consoleOutput_)
			{
#ifdef _DEBUG
			std::cout << "Logger shutdown completed.\n";
#endif
			}
		}

	void CLogger::writeStatistics () {
		stats_.endTime = std::chrono::system_clock::now ();
		stats_.totalMessages = stats_.traceCount + stats_.debugCount +
			stats_.infoCount + stats_.warnCount +
			stats_.errorCount + stats_.fatalCount;

		auto duration = std::chrono::duration_cast< std::chrono::milliseconds >(
			stats_.endTime - stats_.startTime );

		std::stringstream statsMessage;
		statsMessage << "\n=== Logging session statistics ===";
		statsMessage << "\nTotal messages: " << stats_.totalMessages;
		statsMessage << "\n  TRACE: " << stats_.traceCount;
		statsMessage << "\n  DEBUG: " << stats_.debugCount;
		statsMessage << "\n  INFO:  " << stats_.infoCount;
		statsMessage << "\n  WARN:  " << stats_.warnCount;
		statsMessage << "\n  ERROR: " << stats_.errorCount;
		statsMessage << "\n  FATAL: " << stats_.fatalCount;
		statsMessage << "\nSession duration: " << duration.count () << " ms";
		statsMessage << "\nAverage messages per second: "
			<< ( duration.count () > 0 ?
				 ( stats_.totalMessages * 1000.0 / duration.count () ) : 0 );
		statsMessage << "\n==============================\n";

		// Записываем статистику в файл
		if (fileOutput_ && logFile_.is_open ())
			{
			logFile_ << statsMessage.str ();
			logFile_.flush ();
			}

			// Выводим статистику в консоль
		if (consoleOutput_)
			{
			std::cout << statsMessage.str ();
			}
		}

	void CLogger::updateStatistics ( CLogLevel level ) {
		switch (level)
			{
				case CLogLevel::TRACE: stats_.traceCount++; break;
				case CLogLevel::DEBUG: stats_.debugCount++; break;
				case CLogLevel::INFO:  stats_.infoCount++; break;
				case CLogLevel::WARN:  stats_.warnCount++; break;
				case CLogLevel::Error: stats_.errorCount++; break;
				case CLogLevel::FATAL: stats_.fatalCount++; break;
				default: break;
			}
		}

	void CLogger::writeLogEntry ( CLogLevel level, const std::string & message,
								  const std::string & file, int line ) {
		std::stringstream logEntry;

		// Добавляем временную метку
		if (showTimestamp_)
			{
			logEntry << "[" << getCurrentTimestamp () << "] ";
			}

			// Добавляем уровень логирования
		if (showLogLevel_)
			{
			logEntry << "[" << levelToString ( level ) << "] ";
			}

			// Добавляем ID потока (опционально)
		if (showThreadId_)
			{
			logEntry << "[" << formatThreadId () << "] ";
			}

			// Добавляем сообщение
		logEntry << message;

		// Добавляем информацию о файле и строке для TRACE и DEBUG в дебаг сборке
#ifdef _DEBUG
		if (( level == CLogLevel::TRACE || level == CLogLevel::DEBUG ) &&
			 !file.empty () && line != -1)
			{
// Извлекаем только имя файла из полного пути
			size_t pos = file.find_last_of ( "/\\" );
			std::string filename = ( pos != std::string::npos ) ? file.substr ( pos + 1 ) : file;
			logEntry << " (" << filename << ":" << line << ")";
			}
#endif

		std::string finalMessage = logEntry.str ();

		// Вывод в консоль
		if (consoleOutput_)
			{
			if (coloredConsole_)
				{
				std::cout << getColorCode ( level ) << finalMessage << getResetColorCode () << "\n";
				}
			else
				{
				std::cout << finalMessage << "\n";
				}
			std::cout.flush ();
			}

			// Запись в файл (без цветов)
		if (fileOutput_ && logFile_.is_open ())
			{
			logFile_ << finalMessage << "\n";
			logFile_.flush (); // Гарантируем запись на диск
			}

			// Для FATAL ошибок - дополнительное действие
		if (level == CLogLevel::FATAL)
			{
			if (fileOutput_ && logFile_.is_open ())
				{
				logFile_ << "!!! FATAL ERROR - TERMINATING !!!\n";
				logFile_.flush ();
				}

			if (consoleOutput_)
				{
				std::cerr << "\033[1;31m!!! FATAL ERROR - TERMINATING !!!\033[0m\n";
				}
			}
		}

	CLogger::Statistics CLogger::getStatistics () const {
		std::lock_guard<std::mutex> lock ( logMutex_ );
		Statistics statsCopy = stats_;
		statsCopy.totalMessages = statsCopy.traceCount + statsCopy.debugCount +
			statsCopy.infoCount + statsCopy.warnCount +
			statsCopy.errorCount + statsCopy.fatalCount;
		statsCopy.endTime = std::chrono::system_clock::now ();
		return statsCopy;
		}

	void CLogger::init ( const std::string & logName,
						 bool uniqueFilePerRun,
						 bool overwrite,
						 bool consoleOutput,
						 bool coloredConsole ) {
		std::lock_guard<std::mutex> lock ( logMutex_ );

		if (initialized_)
			{
			std::cerr << "Logger is already initialized\n";
			return;
			}

			// Сброс статистики
		stats_.reset ();

		consoleOutput_ = consoleOutput;
		coloredConsole_ = coloredConsole;
		overwrite_ = overwrite;
		shuttingDown_ = false;

		// Создаем директорию для логов
		ensureLogDirectory ();

		// Генерируем имя файла
		if (uniqueFilePerRun)
			{
			logFileName_ = generateUniqueFileName ( logName );
			}
		else
			{
			logFileName_ = logName + ".log";
			}

			// Формируем правильный путь с использованием std::filesystem
		std::filesystem::path filePath = std::filesystem::path ( logDirectory_ ) / logFileName_;

		std::ios_base::openmode mode = std::ios_base::app; // По умолчанию добавляем
		if (overwrite_)
			{
			mode = std::ios_base::trunc; // Перезаписываем если нужно
			}

		logFile_.open ( filePath.string (), mode );
		if (!logFile_.is_open ())
			{
			std::cerr << "ERROR: Cannot open log file: " << filePath.string () << std::endl;
			fileOutput_ = false;
			}
		else
			{
			logFile_ << "\n=== Logging session started at " << getCurrentTimestamp () << " ===\n";
			logFile_ << "=== Log file: " << filePath.string () << " ===\n";
			logFile_.flush ();
			}

		initialized_ = true;

		if (consoleOutput_)
			{
			std::cout << "Logger initialized. Log file: " << filePath.string ()
				<< "\n(unique: " << ( uniqueFilePerRun ? "yes" : "no" )
				<< ", overwrite: " << ( overwrite_ ? "yes" : "no" ) << ")\n";
			}
		}

	void CLogger::ensureLogDirectory () {
		try
			{
		   // Получаем путь к исполняемому файлу
			std::string exePath = getExecutablePath ();

			// Формируем полный путь к папке logs
			std::filesystem::path logsPath;

			if (!exePath.empty ())
				{
				logsPath = std::filesystem::path ( exePath ) / logDirectory_;
				}
			else
				{
					 // Fallback: используем текущую директорию
				logsPath = std::filesystem::current_path () / logDirectory_;
				}

			std::filesystem::create_directories ( logsPath );

			// Обновляем logDirectory_ на полный путь (в строковом формате)
			logDirectory_ = logsPath.string ();
			}
			catch (const std::exception & e)
				{
				std::cerr << "ERROR: Cannot create log directory: " << e.what () << std::endl;
				// Fallback: используем текущую директорию
				std::filesystem::path logsPath = std::filesystem::current_path () / logDirectory_;
				try
					{
					std::filesystem::create_directories ( logsPath );
					logDirectory_ = logsPath.string ();
					}
					catch (...)
						{
						std::cerr << "CRITICAL: Cannot create log directory at fallback location\n";
						fileOutput_ = false;
						}
				}
		}


	void CLogger::setMinLevel ( CLogLevel level ) {
		std::lock_guard<std::mutex> lock ( logMutex_ );
		minLevel_ = level;

		if (consoleOutput_ && initialized_)
			{
			std::cout << "Log level changed to: " << levelToString ( level ) << "\n";
			}
		}

	void CLogger::setConsoleOutput ( bool enabled ) {
		std::lock_guard<std::mutex> lock ( logMutex_ );
		consoleOutput_ = enabled;
		}

	void CLogger::setFileOutput ( bool enabled ) {
		std::lock_guard<std::mutex> lock ( logMutex_ );
		fileOutput_ = enabled;
		}

	void CLogger::setColoredConsole ( bool enabled ) {
		std::lock_guard<std::mutex> lock ( logMutex_ );
		coloredConsole_ = enabled;
		}

	void CLogger::setShowTimestamp ( bool enabled ) {
		std::lock_guard<std::mutex> lock ( logMutex_ );
		showTimestamp_ = enabled;
		}

	void CLogger::setShowLogLevel ( bool enabled ) {
		std::lock_guard<std::mutex> lock ( logMutex_ );
		showLogLevel_ = enabled;
		}

	void CLogger::setShowThreadId ( bool enabled ) {
		std::lock_guard<std::mutex> lock ( logMutex_ );
		showThreadId_ = enabled;
		}

	void CLogger::setLogDirectory ( const std::string & directory ) {
		std::lock_guard<std::mutex> lock ( logMutex_ );
		logDirectory_ = directory;

		if (initialized_)
			{
			std::cerr << "WARN: Log directory changed, but logger is already initialized. Changes will apply on next init.\n";
			}
		}

	std::string CLogger::getCurrentTimestamp () {
		auto now = std::chrono::system_clock::now ();
		auto time = std::chrono::system_clock::to_time_t ( now );
		auto ms = std::chrono::duration_cast< std::chrono::milliseconds >(
			now.time_since_epoch () ) % 1000;

		std::stringstream ss;

		// Используем безопасную версию localtime
		std::tm tm_snapshot = safe_localtime ( time );

		ss << std::put_time ( &tm_snapshot, "%Y-%m-%d %H:%M:%S" );
		ss << '.' << std::setfill ( '0' ) << std::setw ( 3 ) << ms.count ();
		return ss.str ();
		}

	std::string CLogger::levelToString ( CLogLevel level ) {
		switch (level)
			{
				case CLogLevel::TRACE:   return "TRACE";
				case CLogLevel::DEBUG:   return "DEBUG";
				case CLogLevel::INFO:    return "INFO";
				case CLogLevel::WARN:    return "WARN";
				case CLogLevel::Error:   return "ERROR";
				case CLogLevel::FATAL:   return "FATAL";
				default:                return "UNKNOWN";
			}
		}

	std::string CLogger::getColorCode ( CLogLevel level ) {
		// ANSI escape codes для цветов
		switch (level)
			{
				case CLogLevel::TRACE:   return "\033[90m";    // Серый
				case CLogLevel::DEBUG:   return "\033[36m";    // Голубой
				case CLogLevel::INFO:    return "\033[32m";    // Зеленый
				case CLogLevel::WARN:    return "\033[33m";    // Желтый
				case CLogLevel::Error:   return "\033[31m";    // Красный
				case CLogLevel::FATAL:   return "\033[1;31m";  // Ярко-красный
				default:                return "\033[0m";     // Сброс
			}
		}

	std::string CLogger::getResetColorCode () {
		return "\033[0m";
		}

	std::string CLogger::generateUniqueFileName ( const std::string & baseName ) {
		auto now = std::chrono::system_clock::now ();
		auto time = std::chrono::system_clock::to_time_t ( now );

		std::stringstream ss;
		ss << baseName << "_";

		// Используем безопасную версию localtime
		std::tm tm_snapshot = safe_localtime ( time );

		ss << std::put_time ( &tm_snapshot, "%Y%m%d_%H%M%S" );
		ss << "_pid" << GETPID ();
		ss << ".log";

		return ss.str ();
		}

	std::string CLogger::formatThreadId () {
#ifdef _WIN32
		return std::to_string ( GetCurrentThreadId () );
#else
		std::stringstream ss;
		ss << std::this_thread::get_id ();
		return ss.str ();
#endif
		}

	} // namespace CE