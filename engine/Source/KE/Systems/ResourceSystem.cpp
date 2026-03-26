#include "KE/Systems/ResourceSystem.h"
#include "KE/Systems/Managers/MeshManager.h"
#include "KE/ResourceManagerInterface.h"

ResourceSystem::ResourceSystem ()
	{
	LogDebug ( "Created" );
	}
ResourceSystem::~ResourceSystem ()
	{
	Shutdown ();
	}

bool ResourceSystem::PreInit () 
	{
	if (bIsInitialized)
		{
		LogWarn ( "ResourceSystem already initialized!" );
		return true;
		}

	RegisterAllManagers ();

	if(!PreInitAllManagers())
		{
		LogError ("Fail to preInit  Resource Managers");
		return false;
		}
	return true;
	}

MeshManager * ResourceSystem::GetMeshManager () const
	{	 
	return GetManager<MeshManager> ().get();
	}

bool ResourceSystem::Init ()
	{
	if (!InitAllManagers ())
		{
		LogError ( "Fail to Init Resource Managers" );
		return false;
		}
	bIsInitialized = true;
	return true;
	}

void ResourceSystem::Update ( float DeltaTime )
	{
	for (auto& manager : m_Managers)
		{
		manager->Update (DeltaTime);
		}
	}

void ResourceSystem::Shutdown ()
	{
	LogDebug ( "Shutting down" );
	if (m_Managers.empty ()) return ;
	for (const auto& Manager : m_Managers)
		{
		Manager->Shutdown ();
		}
	m_Managers.clear();
	}

const std::string ResourceSystem::GetSystemName () const
	{
	return "Resource System";
	}

void ResourceSystem::RegisterAllManagers ()
	{
	
	LogDebug ( "now stub" );
	auto MeshMgr = RegisterManager<MeshManager> ();

	
	LogDebug ( "All Managers registered: (", m_Managers.size (), ") managers" );
	if (!m_Managers.empty ()&& m_Engine != nullptr)
		{
		for (const auto& Manager : m_Managers)
			{
			if (Manager.get ()->GetEngine () != nullptr)
				{
				LogDebug ( Manager->GetSystemName (), " has engine ptr" ); // TODO: delete this later
				}
			}
		}
	}

bool ResourceSystem::PreInitAllManagers ()
	{
	if (m_Managers.empty ()) return true;
	for (auto & manager : m_Managers)
		{
		if (!manager->PreInit ())
			{
			LogError ( "Manager PreInit failed: {}", manager->GetSystemName () );
			return false;
			}
		}
	return true;
	}
bool ResourceSystem::InitAllManagers ()
	{
	if (m_Managers.empty ()) return true;
	for (auto & manager : m_Managers)
		{
		if (!manager->Init ())
			{
			LogError ( "Manager Init failed: {}", manager->GetSystemName());
			return false;
			}
		}
	return true;
	}