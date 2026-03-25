#include "KE/GameFramework/Components/Meshes/StaticMeshComponent.h"
#include "KE/Vulkan/VertexStructs/AllVertices.h"
#include "KE/GameFramework/Components/Collisions/BoxComponent.h"
#include "KE/Systems/RenderSystem.h"
#include "KE/Engine.h"

CStaticMeshComponent::CStaticMeshComponent ( CObject * inOwner, const std::string & inDisplayName )
    : Super ( inOwner, inDisplayName )
    {
    LOG_DEBUG ( "StaticMeshComponent created: ", GetName () );
   
    }

CStaticMeshComponent::~CStaticMeshComponent ()
    {}

void CStaticMeshComponent::InitComponent ()
    {
    if (this == nullptr) return;
    Super::InitComponent ();
    LOG_DEBUG ( "Initializing Mesh component" );
    SetPipelineName ( "StaticMesh" ); 

    auto * bufferManager = CEngine::Get ().GetRenderer ()->GetBufferManager ();
    if (bufferManager)
        {
        if (MeshData)
            {
            StaticMesh_vertices = MeshData->GetVertices ();
            StaticMesh_indices = MeshData->GetIndices ();
            }
        if (StaticMesh_vertices.empty ())
            {
            CreateFallBackCube ();
            }

        CreateRenderResources (  );
        }

    }

void CStaticMeshComponent::Tick ( float DeltaTime )
    {
    Super::Tick ( DeltaTime );
    }

void CStaticMeshComponent::OnBeginPlay ()
    { 
    Super::OnBeginPlay ();
    }

void CStaticMeshComponent::GenerateVertices ( std::vector<FMeshVertex> & OutVertices ) const
    {
    if (StaticMesh_vertices.empty ())
        {
            // Fallback куб (если по какой-то причине CreateFallBackCube не был вызван)
        OutVertices.clear ();

        OutVertices.push_back ( { {-0.5f, -0.5f,  0.5f}, {0,0,1}, {1,0,0}, {0,0} } );
        OutVertices.push_back ( { { 0.5f, -0.5f,  0.5f}, {0,0,1}, {0,1,0}, {1,0} } );
        OutVertices.push_back ( { { 0.5f,  0.5f,  0.5f}, {0,0,1}, {0,0,1}, {1,1} } );
        OutVertices.push_back ( { {-0.5f,  0.5f,  0.5f}, {0,0,1}, {1,1,0}, {0,1} } );
        OutVertices.push_back ( { {-0.5f, -0.5f, -0.5f}, {0,0,-1}, {0,1,1}, {1,0} } );
        OutVertices.push_back ( { { 0.5f, -0.5f, -0.5f}, {0,0,-1}, {1,0,1}, {0,0} } );
        OutVertices.push_back ( { { 0.5f,  0.5f, -0.5f}, {0,0,-1}, {1,1,1}, {0,1} } );
        OutVertices.push_back ( { {-0.5f,  0.5f, -0.5f}, {0,0,-1}, {0.5,0.5,0.5}, {1,1} } );

        LOG_DEBUG ( "[", GetName (), "] Generated fallback ", OutVertices.size (), " vertices for cube" );
        }
    else
        {
        OutVertices = StaticMesh_vertices; // Используем существующие вершины
        }
    }

void CStaticMeshComponent::GenerateIndices ( std::vector<uint32_t> & OutIndices ) const
    {
    if (StaticMesh_indices.empty ())
        {
            // Fallback индексы
        OutIndices.clear ();

        OutIndices.push_back ( 0 ); OutIndices.push_back ( 1 ); OutIndices.push_back ( 2 );
        OutIndices.push_back ( 0 ); OutIndices.push_back ( 2 ); OutIndices.push_back ( 3 );
        OutIndices.push_back ( 1 ); OutIndices.push_back ( 5 ); OutIndices.push_back ( 6 );
        OutIndices.push_back ( 1 ); OutIndices.push_back ( 6 ); OutIndices.push_back ( 2 );
        OutIndices.push_back ( 5 ); OutIndices.push_back ( 4 ); OutIndices.push_back ( 7 );
        OutIndices.push_back ( 5 ); OutIndices.push_back ( 7 ); OutIndices.push_back ( 6 );
        OutIndices.push_back ( 4 ); OutIndices.push_back ( 0 ); OutIndices.push_back ( 3 );
        OutIndices.push_back ( 4 ); OutIndices.push_back ( 3 ); OutIndices.push_back ( 7 );
        OutIndices.push_back ( 3 ); OutIndices.push_back ( 2 ); OutIndices.push_back ( 6 );
        OutIndices.push_back ( 3 ); OutIndices.push_back ( 6 ); OutIndices.push_back ( 7 );
        OutIndices.push_back ( 4 ); OutIndices.push_back ( 5 ); OutIndices.push_back ( 1 );
        OutIndices.push_back ( 4 ); OutIndices.push_back ( 1 ); OutIndices.push_back ( 0 );

        LOG_DEBUG ( "[", GetName (), "] Generated fallback ", OutIndices.size (), " indices for cube" );
        }
    else
        {
        OutIndices = StaticMesh_indices; // Используем существующие индексы
        }
    }

void CStaticMeshComponent::CreateFallBackCube ( float Size /* = 10.0f */ )
    {
     // Очищаем старые данные
    StaticMesh_vertices.clear ();
    StaticMesh_indices.clear ();

    float h = Size; // половина размера (от -h до +h)

    // ============================================================================
    // Вершины куба (24 вершины - по 4 на каждую грань)
    // Каждая вершина имеет: позицию, нормаль, цвет, UV
    // ============================================================================

    // Передняя грань (Z+) - нормаль (0,0,1)
    StaticMesh_vertices.push_back ( { {-h, -h,  h}, {0,0,1}, {1,0,0}, {0,0} } ); // 0: красный
    StaticMesh_vertices.push_back ( { { h, -h,  h}, {0,0,1}, {0,1,0}, {1,0} } ); // 1: зелёный
    StaticMesh_vertices.push_back ( { { h,  h,  h}, {0,0,1}, {0,0,1}, {1,1} } ); // 2: синий
    StaticMesh_vertices.push_back ( { {-h,  h,  h}, {0,0,1}, {1,1,0}, {0,1} } ); // 3: жёлтый

    // Задняя грань (Z-) - нормаль (0,0,-1)
    StaticMesh_vertices.push_back ( { {-h, -h, -h}, {0,0,-1}, {0,1,1}, {1,0} } ); // 4: голубой
    StaticMesh_vertices.push_back ( { { h, -h, -h}, {0,0,-1}, {1,0,1}, {0,0} } ); // 5: розовый
    StaticMesh_vertices.push_back ( { { h,  h, -h}, {0,0,-1}, {1,1,1}, {0,1} } ); // 6: белый
    StaticMesh_vertices.push_back ( { {-h,  h, -h}, {0,0,-1}, {0.5,0.5,0.5}, {1,1} } ); // 7: серый

    // Левая грань (X-) - нормаль (-1,0,0)
    StaticMesh_vertices.push_back ( { {-h, -h, -h}, {-1,0,0}, {1,0,0}, {0,0} } ); // 8: красный
    StaticMesh_vertices.push_back ( { {-h, -h,  h}, {-1,0,0}, {0,1,0}, {1,0} } ); // 9: зелёный
    StaticMesh_vertices.push_back ( { {-h,  h,  h}, {-1,0,0}, {0,0,1}, {1,1} } ); // 10: синий
    StaticMesh_vertices.push_back ( { {-h,  h, -h}, {-1,0,0}, {1,1,0}, {0,1} } ); // 11: жёлтый

    // Правая грань (X+) - нормаль (1,0,0)
    StaticMesh_vertices.push_back ( { { h, -h, -h}, {1,0,0}, {0,1,1}, {1,0} } ); // 12: голубой
    StaticMesh_vertices.push_back ( { { h, -h,  h}, {1,0,0}, {1,0,1}, {0,0} } ); // 13: розовый
    StaticMesh_vertices.push_back ( { { h,  h,  h}, {1,0,0}, {1,1,1}, {0,1} } ); // 14: белый
    StaticMesh_vertices.push_back ( { { h,  h, -h}, {1,0,0}, {0.5,0.5,0.5}, {1,1} } ); // 15: серый

    // Нижняя грань (Y-) - нормаль (0,-1,0)
    StaticMesh_vertices.push_back ( { {-h, -h, -h}, {0,-1,0}, {1,0,0}, {0,0} } ); // 16: красный
    StaticMesh_vertices.push_back ( { { h, -h, -h}, {0,-1,0}, {0,1,0}, {1,0} } ); // 17: зелёный
    StaticMesh_vertices.push_back ( { { h, -h,  h}, {0,-1,0}, {0,0,1}, {1,1} } ); // 18: синий
    StaticMesh_vertices.push_back ( { {-h, -h,  h}, {0,-1,0}, {1,1,0}, {0,1} } ); // 19: жёлтый

    // Верхняя грань (Y+) - нормаль (0,1,0)
    StaticMesh_vertices.push_back ( { {-h,  h, -h}, {0,1,0}, {0,1,1}, {1,0} } ); // 20: голубой
    StaticMesh_vertices.push_back ( { { h,  h, -h}, {0,1,0}, {1,0,1}, {0,0} } ); // 21: розовый
    StaticMesh_vertices.push_back ( { { h,  h,  h}, {0,1,0}, {1,1,1}, {0,1} } ); // 22: белый
    StaticMesh_vertices.push_back ( { {-h,  h,  h}, {0,1,0}, {0.5,0.5,0.5}, {1,1} } ); // 23: серый

    // ============================================================================
    // Индексы для 6 граней (по 2 треугольника на грань)
    // ============================================================================

    // Передняя грань (Z+) - индексы 0-3
    StaticMesh_indices.push_back ( 0 ); StaticMesh_indices.push_back ( 1 ); StaticMesh_indices.push_back ( 2 );
    StaticMesh_indices.push_back ( 0 ); StaticMesh_indices.push_back ( 2 ); StaticMesh_indices.push_back ( 3 );

    // Задняя грань (Z-) - индексы 4-7
    StaticMesh_indices.push_back ( 4 ); StaticMesh_indices.push_back ( 5 ); StaticMesh_indices.push_back ( 6 );
    StaticMesh_indices.push_back ( 4 ); StaticMesh_indices.push_back ( 6 ); StaticMesh_indices.push_back ( 7 );

    // Левая грань (X-) - индексы 8-11
    StaticMesh_indices.push_back ( 8 ); StaticMesh_indices.push_back ( 9 ); StaticMesh_indices.push_back ( 10 );
    StaticMesh_indices.push_back ( 8 ); StaticMesh_indices.push_back ( 10 ); StaticMesh_indices.push_back ( 11 );

    // Правая грань (X+) - индексы 12-15
    StaticMesh_indices.push_back ( 12 ); StaticMesh_indices.push_back ( 13 ); StaticMesh_indices.push_back ( 14 );
    StaticMesh_indices.push_back ( 12 ); StaticMesh_indices.push_back ( 14 ); StaticMesh_indices.push_back ( 15 );

    // Нижняя грань (Y-) - индексы 16-19
    StaticMesh_indices.push_back ( 16 ); StaticMesh_indices.push_back ( 17 ); StaticMesh_indices.push_back ( 18 );
    StaticMesh_indices.push_back ( 16 ); StaticMesh_indices.push_back ( 18 ); StaticMesh_indices.push_back ( 19 );

    // Верхняя грань (Y+) - индексы 20-23
    StaticMesh_indices.push_back ( 20 ); StaticMesh_indices.push_back ( 21 ); StaticMesh_indices.push_back ( 22 );
    StaticMesh_indices.push_back ( 20 ); StaticMesh_indices.push_back ( 22 ); StaticMesh_indices.push_back ( 23 );

    SetCollisionEnabled ();

    LOG_DEBUG ( "[", GetName (), "] Created fallback cube with ",
                StaticMesh_vertices.size (), " vertices and ",
                StaticMesh_indices.size (), " indices" );
    }

void CStaticMeshComponent::ResizeCube ( float NewSize )
    {
    CreateFallBackCube ( NewSize );

    // Пересоздаём рендер ресурсы если компонент уже инициализирован
    if (m_bRenderResourcesCreated)
        {
        CreateRenderResources ();
        }
    }

void CStaticMeshComponent::CreateBolt ()
    {
    StaticMesh_vertices.clear ();
    StaticMesh_indices.clear ();

 // Вершины увеличенного куба (от -10 до 10)
    StaticMesh_vertices.push_back ( { {-10.f, -10.f,  10.f}, {0,0,1}, {1,0,0}, {0,0} } ); // 0: красный
    StaticMesh_vertices.push_back ( { { 10.f, -10.f,  10.f}, {0,0,1}, {0,1,0}, {1,0} } ); // 1: зелёный
    StaticMesh_vertices.push_back ( { { 10.f,  10.f,  10.f}, {0,0,1}, {0,0,1}, {1,1} } ); // 2: синий
    StaticMesh_vertices.push_back ( { {-10.f,  10.f,  10.f}, {0,0,1}, {1,1,0}, {0,1} } ); // 3: жёлтый
    StaticMesh_vertices.push_back ( { {-10.f, -10.f, -10.f}, {0,0,-1}, {0,1,1}, {1,0} } ); // 4: голубой
    StaticMesh_vertices.push_back ( { { 10.f, -10.f, -10.f}, {0,0,-1}, {1,0,1}, {0,0} } ); // 5: розовый
    StaticMesh_vertices.push_back ( { { 10.f,  10.f, -10.f}, {0,0,-1}, {1,1,1}, {0,1} } ); // 6: белый
    StaticMesh_vertices.push_back ( { {-10.f,  10.f, -10.f}, {0,0,-1}, {0.5,0.5,0.5}, {1,1} } ); // 7: серый

    // Индексы
    StaticMesh_indices.push_back ( 0 ); StaticMesh_indices.push_back ( 1 ); StaticMesh_indices.push_back ( 2 );
    StaticMesh_indices.push_back ( 0 ); StaticMesh_indices.push_back ( 2 ); StaticMesh_indices.push_back ( 3 );
    StaticMesh_indices.push_back ( 1 ); StaticMesh_indices.push_back ( 5 ); StaticMesh_indices.push_back ( 6 );
    StaticMesh_indices.push_back ( 1 ); StaticMesh_indices.push_back ( 6 ); StaticMesh_indices.push_back ( 2 );
    StaticMesh_indices.push_back ( 5 ); StaticMesh_indices.push_back ( 4 ); StaticMesh_indices.push_back ( 7 );
    StaticMesh_indices.push_back ( 5 ); StaticMesh_indices.push_back ( 7 ); StaticMesh_indices.push_back ( 6 );
    StaticMesh_indices.push_back ( 4 ); StaticMesh_indices.push_back ( 0 ); StaticMesh_indices.push_back ( 3 );
    StaticMesh_indices.push_back ( 4 ); StaticMesh_indices.push_back ( 3 ); StaticMesh_indices.push_back ( 7 );
    StaticMesh_indices.push_back ( 3 ); StaticMesh_indices.push_back ( 2 ); StaticMesh_indices.push_back ( 6 );
    StaticMesh_indices.push_back ( 3 ); StaticMesh_indices.push_back ( 6 ); StaticMesh_indices.push_back ( 7 );
    StaticMesh_indices.push_back ( 4 ); StaticMesh_indices.push_back ( 5 ); StaticMesh_indices.push_back ( 1 );
    StaticMesh_indices.push_back ( 4 ); StaticMesh_indices.push_back ( 1 ); StaticMesh_indices.push_back ( 0 );

    LOG_DEBUG ( "[", GetName (), "] Created fallback cube with ",
                StaticMesh_vertices.size (), " vertices and ",
                StaticMesh_indices.size (), " indices" );
    }
