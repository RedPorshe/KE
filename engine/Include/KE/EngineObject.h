#pragma once
#include <string>

class IEngineSystem
	{
	public:
		IEngineSystem () {};
		virtual ~IEngineSystem () =default;
		virtual bool PreInit () = 0;
		virtual bool Init () = 0;
		virtual void Shutdown () = 0;
		virtual void Update ( float DeltaTime ) = 0;

		virtual const std::string GetSystemName () const = 0;
	};