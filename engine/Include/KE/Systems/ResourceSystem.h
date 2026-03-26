#pragma once
#include "KE/EngineObject.h"

class IResourceManager;

class ResourceSystem : public IEngineSystem
	{
	public:
		ResourceSystem ();
		virtual ~ResourceSystem () override;
		 bool PreInit () override ;
		 bool Init ()override ;
		 void Shutdown () override ;
		 void Update ( float DeltaTime ) override;

		 class MeshManager * GetMeshManager () const;

		 template<typename T>
		 TSharedPtr<T> GetManager () const
			 {
			 for (auto & manager : m_Managers)
				 {
				 if (auto casted = std::dynamic_pointer_cast< T >( manager ))
					 return casted;
				 }
			 return nullptr;
			 }

	protected:
		 const std::string GetSystemName () const override ;

		 bool InitAllManagers ();
		 bool PreInitAllManagers ();
		 void RegisterAllManagers ();

		 template<typename T, typename... Args>
		 TSystemPtr<T> RegisterManager ( Args&&... args ) {
			 auto manager = MakeShared<T> ( std::forward<Args> ( args )... );
			 if (m_Engine)
				 {
				 manager->SetEnginePtr ( m_Engine );
				 }
			 m_Managers.push_back ( manager );
			 LogDebug ( "Registered Manager: ", manager->GetSystemName () );
			 return manager;
			 }

		 TVector<TSystemPtr<IResourceManager>> m_Managers;
	};