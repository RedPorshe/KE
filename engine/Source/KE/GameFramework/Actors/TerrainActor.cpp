#include "KE/GameFramework/Actors/TerrainActor.h"
#include "KE/GameFramework/Components/Collisions/TerrainComponent.h"
#include "KE/GameFramework/Components/Meshes/TerrainMeshComponent.h"
#include "KE/GameFramework/Components/GravityComponent.h"

CTerrainActor::CTerrainActor ( CObject * inOwner, const std::string & inDisplayName )
    : Super ( inOwner, inDisplayName )
    {
    bIsTerrain = true;
    LOG_DEBUG ( "[TERRAIN ACTOR] Created: ", GetName () );
    SetMovableState ( EMovableState::STATIC );
    DestroyGravity ();
    }

CTerrainActor::~CTerrainActor ()
    {
    m_TerrainComponent = nullptr;
    m_TerrainMeshComponent = nullptr;
    }

void CTerrainActor::BeginPlay ()
    {
    Super::BeginPlay ();
    // Компоненты создаются при вызове методов генерации
    LOG_DEBUG ( "[TERRAIN ACTOR] BeginPlay - waiting for generation" );
    }

void CTerrainActor::Tick ( float deltaTime )
    {
    Super::Tick ( deltaTime );
    }

void CTerrainActor::EndPlay ()
    {
    Super::EndPlay ();
    }

CTerrainMeshComponent * CTerrainActor::GetTerrainMeshComponent () const
    {
    return m_TerrainMeshComponent;
    }

CTerrainComponent * CTerrainActor::GetTerrainComponent () const
    {
    return m_TerrainComponent;
    }

    // ============================================================================
    // Вспомогательные методы для создания компонентов
    // ============================================================================

void CTerrainActor::CreateTerrainComponents ()
    {
    if (!m_TerrainComponent)
        {
        m_TerrainComponent = AddDefaultSubObject<CTerrainComponent> ( GetName () + "_TerrainComponent" );
        m_TerrainComponent->AttachTo ( RootComponent );
        LOG_DEBUG ( "[TERRAIN ACTOR] Created TerrainComponent: ", m_TerrainComponent->GetName () );
        }

    if (!m_TerrainMeshComponent)
        {
        m_TerrainMeshComponent = AddDefaultSubObject<CTerrainMeshComponent> ( GetName () + "_TerrainMesh" );
        m_TerrainMeshComponent->AttachTo ( RootComponent );
        LOG_DEBUG ( "[TERRAIN ACTOR] Created TerrainMeshComponent: ", m_TerrainMeshComponent->GetName () );
        }

        // Настраиваем связи между компонентами
    if (m_TerrainComponent && m_TerrainMeshComponent)
        {
        m_TerrainMeshComponent->SetTerrainComponent ( m_TerrainComponent );
        LOG_DEBUG ( "[TERRAIN ACTOR] Linked TerrainMesh to TerrainComponent" );
        }

        // Устанавливаем коллизию для корневого компонента
    if (m_TerrainComponent && GetRootComponent ())
        {
        GetRootComponent ()->SetCollisionComponent ( m_TerrainComponent );
        }

        // Террейну не нужна гравитация
    DestroyGravity ();
    }

    // ============================================================================
    // Методы генерации террейна
    // ============================================================================

void CTerrainActor::GenerateFlat ( int32 width, int32 height, float cellSize, float heightValue )
    {
    CreateTerrainComponents ();

    if (m_TerrainComponent)
        {
        LOG_DEBUG ( "[TERRAIN] Generating flat terrain: ", width, "x", height );
        m_TerrainComponent->GenerateFlat ( width, height, cellSize, heightValue );

        if (m_TerrainMeshComponent)
            {
            m_TerrainMeshComponent->UpdateFromTerrain ();
            }
        }
    }

void CTerrainActor::GenerateFromHeightmap ( const std::vector<float> & heights, int32 width, int32 height, float cellSize )
    {
    CreateTerrainComponents ();

    if (m_TerrainComponent)
        {
        LOG_DEBUG ( "[TERRAIN] Generating from heightmap: ", width, "x", height );
        m_TerrainComponent->GenerateFromHeightmap ( heights, width, height, cellSize );

        if (m_TerrainMeshComponent)
            {
            m_TerrainMeshComponent->UpdateFromTerrain ();
            }
        }
    }

void CTerrainActor::GenerateHilly ( int32 width, int32 height, float cellSize,
                                    float amplitude, float frequency )
    {
    CreateTerrainComponents ();

    if (m_TerrainComponent)
        {
        LOG_DEBUG ( "[TERRAIN] Generating hilly terrain: ", width, "x", height,
                    ", amp=", amplitude, ", freq=", frequency );

        m_TerrainComponent->GenerateHilly ( width, height, cellSize, amplitude, frequency );

        if (m_TerrainMeshComponent)
            {
            m_TerrainMeshComponent->UpdateFromTerrain ();
            }

        const FTerrainData & data = m_TerrainComponent->GetTerrainData ();
        LOG_DEBUG ( "[TERRAIN] Generated with height range: ",
                    data.MinHeight, " - ", data.MaxHeight );
        }
    }

void CTerrainActor::GenerateNoise ( int32 width, int32 height, float cellSize, int32 seed )
    {
    CreateTerrainComponents ();

    if (m_TerrainComponent)
        {
        LOG_DEBUG ( "[TERRAIN] Generating noise terrain: ", width, "x", height, ", seed=", seed );
        m_TerrainComponent->GenerateNoise ( width, height, cellSize, seed );

        if (m_TerrainMeshComponent)
            {
            m_TerrainMeshComponent->UpdateFromTerrain ();
            }
        }
    }

void CTerrainActor::GenerateCustom ( int32 width, int32 height, float cellSize,
                                     std::function<float ( int32 x, int32 z )> heightFunc )
    {
    CreateTerrainComponents ();

    if (m_TerrainComponent)
        {
        LOG_DEBUG ( "[TERRAIN] Generating custom terrain: ", width, "x", height );
        m_TerrainComponent->GenerateCustom ( width, height, cellSize, heightFunc );

        if (m_TerrainMeshComponent)
            {
            m_TerrainMeshComponent->UpdateFromTerrain ();
            }
        }
    }