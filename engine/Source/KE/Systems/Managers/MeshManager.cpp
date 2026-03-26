// MeshManager.cpp
#include "KE/Systems/Managers/MeshManager.h"
#include "KE/GameFramework/Meshes/BaseMesh.h" 
#include "KE/Engine.h"

MeshManager::MeshManager () : IResourceManager ()
    {
    LogDebug ( "Created" );
    }

MeshManager::~MeshManager ()
    {
    Shutdown ();
    }

bool MeshManager::PreInit ()
    {
    LogDebug ( "PreInit" );
    return true;
    }

void MeshManager::Update ( float DeltaTime )
    {
    static float timer = 0.f;
    timer += DeltaTime;
    if (timer >= 5.f)
        {
        CleanupUnusedResources ();
        timer = 0.f;
        }
    }

bool MeshManager::Init ()
    {
    LogDebug ( "Init" );
    bIsInitialized = true;
    return true;
    }

void MeshManager::Shutdown ()
    {
    LogDebug ( "Shutting down" );
    CleanupUnusedResources (); // Очищаем неиспользуемые
    m_Meshes.clear ();         // Очищаем всё
    }

const std::string MeshManager::GetSystemName () const
    {
    return "Mesh Manager";
    }

void MeshManager::CleanupUnusedResources ()
    {
    std::lock_guard<std::mutex> lock ( m_Mutex );

    size_t beforeCount = m_Meshes.size ();

    // Удаляем все expired weak_ptr (меши, которые никто не использует)
    for (auto it = m_Meshes.begin (); it != m_Meshes.end ();)
        {
        if (it->second.expired ())
            {
            LogTrace ( "Removing unused mesh: ", it->first );
            it = m_Meshes.erase ( it );
            }
        else
            {
            ++it;
            }
        }

    size_t afterCount = m_Meshes.size ();
    if (beforeCount != afterCount)
        {
        LogDebug ( "Cleaned up ", beforeCount - afterCount, " unused meshes" );
        }
    }

std::shared_ptr<CBaseMesh> MeshManager::LoadMeshFromFile ( const std::string & Path )
    {
     
    std::string MeshName = Path;   
    size_t lastSlash = MeshName.find_last_of ( "/\\" );
    if (lastSlash != std::string::npos)
        {
        MeshName = MeshName.substr ( lastSlash + 1 );
        }     
    size_t lastDot = MeshName.find_last_of ( '.' );
    if (lastDot != std::string::npos)
        {
        MeshName = MeshName.substr ( 0, lastDot );
        }

    auto mesh = std::make_shared<CBaseMesh> ( nullptr, MeshName );

    if (!mesh)
        {
        LogError ( "Failed to create mesh object" );
        return nullptr;
        }
    mesh->SetMeshManager ( this );

    m_Meshes[ Path ] = mesh;
    auto MeshesPath = m_Engine->GetModelsPath ();

   
    if (!mesh->LoadMesh ( MeshesPath+Path ))
        {
        LogError ( "Failed to load mesh: ", Path );
        return nullptr;
        }

    LogDebug ( "Loaded mesh: ", Path );
    return mesh;
    }

std::shared_ptr<CBaseMesh> MeshManager::LoadMesh ( const std::string & MeshPath )
    {
    if (MeshPath.empty ())
        {
        LogError ( "LoadMesh: empty path" );
        return nullptr;
        }

    std::lock_guard<std::mutex> lock ( m_Mutex );

    // Проверяем, не загружен ли уже меш
    auto it = m_Meshes.find ( MeshPath );
    if (it != m_Meshes.end ())
        {
        auto existingMesh = it->second.lock ();
        if (existingMesh)
            {
            LogTrace ( "Mesh already loaded: ", MeshPath );
            return existingMesh;
            }
        else
            {
                // weak_ptr expired, удаляем его
            m_Meshes.erase ( it );
            }
        }

        // Загружаем новый меш
    auto mesh = LoadMeshFromFile ( MeshPath );
    if (mesh)
        {
        m_Meshes[ MeshPath ] = mesh;  // Сохраняем weak_ptr
        LogDebug ( "Mesh loaded and cached: ", MeshPath );
        }
    else
        {
        LogError ( "Failed to load mesh: ", MeshPath );
        }

    return m_Meshes[ MeshPath ].lock();
    }

void MeshManager::UnloadMesh ( const std::string & MeshPath )
    {
    std::lock_guard<std::mutex> lock ( m_Mutex );

    auto it = m_Meshes.find ( MeshPath );
    if (it != m_Meshes.end ())
        {
        m_Meshes.erase ( it );
        LogDebug ( "Mesh unloaded: ", MeshPath );
        }
    else
        {
        LogWarn ( "Mesh not found for unloading: ", MeshPath );
        }
    }