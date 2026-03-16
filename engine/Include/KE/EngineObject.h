#pragma once
#include "CoreMinimal.h"

class CEngine;

class KE_API IEngineSystem
	{
	public:
		IEngineSystem () {};
		virtual ~IEngineSystem () =default;
		virtual bool PreInit () = 0;
		virtual bool Init () = 0;
		virtual void Shutdown () = 0;
		virtual void Update ( float DeltaTime ) = 0;

		virtual const std::string GetSystemName () const = 0;

		bool IsInitialized () const { return bIsInitialized; }
		CEngine * GetEngine () const { return m_Engine; }
		void SetEnginePtr ( CEngine * ptr ) { m_Engine = ptr; }
	protected:
		CEngine * m_Engine = nullptr;
		bool bIsInitialized = false;

		template<typename... Args>
		void LogDebug ( Args&&... args ) const
			{
			LOG_DEBUG ( "[", GetSystemName (), "] ", std::forward<Args> ( args )... );
			}

		template<typename... Args>
		void LogError ( Args&&... args ) const
			{
			LOG_ERROR ( "[", GetSystemName (), "]", std::forward<Args> ( args )... );
			}
		template <typename... Args>
		void LogInfo ( Args&&... args ) const
			{
			LOG_INFO ("[",GetSystemName(),"]",std::forward<Args>(args)...);
			}
		template <typename... Args>
		void LogWarn ( Args&&... args ) const
			{
			LOG_WARN ( "[", GetSystemName (), "]", std::forward<Args> ( args )... );
			}
		template <typename... Args>
		void LogFatal ( Args&&... args ) const
			{
			LOG_FATAL ( "[", GetSystemName (), "]", std::forward<Args> ( args )... );
			}
		template <typename... Args>
		void LogTrace ( Args&&... args ) const
			{
			LOG_TRACE ( "[", GetSystemName (), "]", std::forward<Args> ( args )... );
			}
	};