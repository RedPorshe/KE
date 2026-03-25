#pragma once
#include "KE/EngineObject.h"

class IResourceManager : public IEngineSystem
	{
	public:
		IResourceManager () : IEngineSystem () {}
		virtual ~IResourceManager () override = default;
		virtual bool PreInit () override = 0;
		virtual bool Init ()override = 0;
		virtual void Shutdown () override = 0;
		virtual void Update ( float DeltaTime ) override {};
		const std::string GetSystemName () const override = 0;

	protected:
		virtual void CleanupUnusedResources () = 0;


		template<typename T>
		std::shared_ptr<T> LoadResource (
			const std::string & Path,
			std::unordered_map<std::string, std::weak_ptr<T>> & Cache,
			std::function<std::shared_ptr<T> ( const std::string & )> Loader )
			{
			std::lock_guard<std::mutex> lock ( m_Mutex );

			auto it = Cache.find ( Path );
			if (it != Cache.end ())
				{
				auto resource = it->second.lock ();
				if (resource)
					return resource;
				}

			auto resource = Loader ( Path );
			if (resource)
				{
				Cache[ Path ] = resource;
				}

			return resource;
			}

	protected:
		std::mutex m_Mutex;
	};