#pragma once
#include "Core/KEExport.h"
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <type_traits>

namespace CE
    {

    class KE_API CLogger
        {
        public:
            enum class CLogLevel
                {
                TRACE,      // Детальная отладочная информация
                DEBUG,      // Отладочная информация
                INFO,       // Информационные сообщения
                WARN,       // Предупреждения
                Error,      // Ошибки
                FATAL,      // Критические ошибки
                DISABLED    // Логирование отключено
                };

                // Структура статистики
            struct KE_API Statistics
                {
                size_t traceCount = 0;
                size_t debugCount = 0;
                size_t infoCount = 0;
                size_t warnCount = 0;
                size_t errorCount = 0;
                size_t fatalCount = 0;
                size_t totalMessages = 0;
                std::chrono::system_clock::time_point startTime;
                std::chrono::system_clock::time_point endTime;

                void reset () {
                    traceCount = 0;
                    debugCount = 0;
                    infoCount = 0;
                    warnCount = 0;
                    errorCount = 0;
                    fatalCount = 0;
                    totalMessages = 0;
                    startTime = std::chrono::system_clock::now ();
                    }
                };

                // Получение единственного экземпляра
            static CLogger & getInstance ();

            // Основной метод инициализации
            void init ( const std::string & logName = "engine",
                        bool uniqueFilePerRun = false,
                        bool overwrite = false,
                        bool consoleOutput = true,
                        bool coloredConsole = true );

              // Variadic методы логирования (основные)
            template<typename... Args>
            void trace ( const std::string & file, int line, Args&&... args ) {
                log ( CLogLevel::TRACE, file, line, std::forward<Args> ( args )... );
                }

            template<typename... Args>
            void debug ( const std::string & file, int line, Args&&... args ) {
                log ( CLogLevel::DEBUG, file, line, std::forward<Args> ( args )... );
                }

            template<typename... Args>
            void info ( const std::string & file, int line, Args&&... args ) {
                log ( CLogLevel::INFO, file, line, std::forward<Args> ( args )... );
                }

            template<typename... Args>
            void warn ( const std::string & file, int line, Args&&... args ) {
                log ( CLogLevel::WARN, file, line, std::forward<Args> ( args )... );
                }

            template<typename... Args>
            void error ( const std::string & file, int line, Args&&... args ) {
                log ( CLogLevel::Error, file, line, std::forward<Args> ( args )... );
                }

            template<typename... Args>
            void fatal ( const std::string & file, int line, Args&&... args ) {
                log ( CLogLevel::FATAL, file, line, std::forward<Args> ( args )... );
                }

                // Установка уровня логирования
            void setMinLevel ( CLogLevel level );

            // Получение текущего уровня
            CLogLevel getMinLevel () const { return minLevel_; }

            // Явное завершение работы логгера
            void shutdown ();

            // Проверка инициализации
            bool isInitialized () const { return initialized_; }

            // Получение статистики
            Statistics getStatistics () const;

            // Установка дополнительных параметров
            void setConsoleOutput ( bool enabled );
            void setFileOutput ( bool enabled );
            void setColoredConsole ( bool enabled );
            void setShowTimestamp ( bool enabled );
            void setShowLogLevel ( bool enabled );
            void setShowThreadId ( bool enabled );
            void setLogDirectory ( const std::string & directory );

        private:
            CLogger ();
            ~CLogger ();
            CLogger ( const CLogger & ) = delete;
            CLogger & operator=( const CLogger & ) = delete;

            // Получение пути к исполняемому файлу
            static std::string getExecutablePath ();

            // Основной variadic метод логирования
            template<typename... Args>
            void log ( CLogLevel level, const std::string & file, int line, Args&&... args ) {
                if (!initialized_ || shuttingDown_ || level < minLevel_)
                    {
                    return;
                    }

                std::lock_guard<std::mutex> lock ( logMutex_ );

                if (shuttingDown_)
                    {
                    return;
                    }

                    // Обновляем статистику
                updateStatistics ( level );

                // Формируем сообщение
                std::string message = concatArgs ( std::forward<Args> ( args )... );
                writeLogEntry ( level, message, file, line );
                }

                // Вспомогательная функция для конкатенации аргументов
            template<typename T>
            std::string toString ( const T & value ) {
                std::ostringstream oss;
                oss << value;
                return oss.str ();
                }

            std::string concatArgs () {
                return "";
                }

            template<typename First, typename... Rest>
            std::string concatArgs ( First && first, Rest&&... rest ) {
                return toString ( std::forward<First> ( first ) ) + concatArgs ( std::forward<Rest> ( rest )... );
                }

                // Обновление статистики
            void updateStatistics ( CLogLevel level );

            // Запись лог-записи
            void writeLogEntry ( CLogLevel level, const std::string & message,
                                 const std::string & file, int line );

            std::string getCurrentTimestamp ();
            std::string levelToString ( CLogLevel level );
            std::string getColorCode ( CLogLevel level );
            std::string getResetColorCode ();
            void ensureLogDirectory ();
            std::string generateUniqueFileName ( const std::string & baseName );
            std::string formatThreadId ();
            void writeStatistics ();

            // Конфигурация
            std::string logDirectory_ = "logs";
            std::string logFileName_;
            bool consoleOutput_ = true;
            bool fileOutput_ = true;
            bool coloredConsole_ = true;
            bool showTimestamp_ = true;
            bool showLogLevel_ = true;
            bool showThreadId_ = false;
            bool overwrite_ = false;

            CLogLevel minLevel_ = CLogLevel::INFO;

            std::ofstream logFile_;
            mutable std::mutex logMutex_;
            bool initialized_ = false;
            bool shuttingDown_ = false;

            // Статистика
            Statistics stats_;
        };

        // Вспомогательная функция для безопасного получения localtime
        KE_API   std::tm safe_localtime ( const std::time_t & time );

    } // namespace CE

    // Макросы для удобной инициализации
#define LOG_INIT(logName, uniqueFilePerRun, overwrite) \
    CE::CLogger::getInstance().init(logName, uniqueFilePerRun, overwrite)

#define LOG_INIT_DEFAULT(logName) \
    CE::CLogger::getInstance().init(logName, false, false)

#define LOG_SET_LEVEL(level) \
    CE::CLogger::getInstance().setMinLevel(level)

#define LOG_SHUTDOWN() \
    CE::CLogger::getInstance().shutdown()

// Основные макросы логирования
#define LOG_TRACE(...) CE::CLogger::getInstance().trace(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) CE::CLogger::getInstance().debug(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...)  CE::CLogger::getInstance().info(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  CE::CLogger::getInstance().warn(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) CE::CLogger::getInstance().error(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) CE::CLogger::getInstance().fatal(__FILE__, __LINE__, __VA_ARGS__)

// Макросы для проверки инициализации и статистики
#define LOG_IS_INITIALIZED() CE::CLogger::getInstance().isInitialized()
#define LOG_GET_STATS() CE::CLogger::getInstance().getStatistics()