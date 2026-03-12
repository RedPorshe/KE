#include "Components/Meshes/TerrainMeshComponent.h"
#include "Components/Collisions/TerrainComponent.h"
#include "Render/Renderer.h"
#include "Core/Engine.h"
#include "Actors/Actor.h"
#include "Utils/Math/CE_MathHelpers.h"
#include "Render/Vulkan/VertexStructs/AllVertices.h"

CTerrainMeshComponent::CTerrainMeshComponent ( CObject * inOwner, const std::string & inDisplayName )
    : Super ( inOwner, inDisplayName )
    {
    m_PipelineName = "TerrainPipeline";
    LOG_DEBUG ( "TerrainMeshComponent created: ", GetName () );
    
    }

CTerrainMeshComponent::~CTerrainMeshComponent ()
    {
    m_TerrainComponent = nullptr;
    m_CachedData = nullptr;
    }

void CTerrainMeshComponent::InitComponent ()
    {
    Super::InitComponent ();

    // Пытаемся найти TerrainComponent у владельца, если ещё не установлен
    if (!m_TerrainComponent)
        {
        CActor * owner = GetOwnerActor ();
        if (owner)
            {
            m_TerrainComponent = owner->FindComponent<CTerrainComponent> ();
            if (m_TerrainComponent)
                {
                SetCollisionComponent ( m_TerrainComponent );
                LOG_DEBUG ( "[", GetName (), "] Found TerrainComponent: ", m_TerrainComponent->GetName () );
                }
            }
        }
    }

void CTerrainMeshComponent::Tick ( float DeltaTime )
    {
    Super::Tick ( DeltaTime );
    }

void CTerrainMeshComponent::OnBeginPlay ()
    {
    Super::OnBeginPlay ();

    // Создаём ресурсы после того, как террейн сгенерирован
    if (m_TerrainComponent && !m_bRenderResourcesCreated)
        {
        LOG_DEBUG ( "[", GetName (), "] Creating terrain resources in OnBeginPlay" );
        UpdateFromTerrain ();
        }
    else if (!m_TerrainComponent)
        {
        LOG_WARN ( "[", GetName (), "] No terrain component in OnBeginPlay" );
        }
    }

FTerrainRenderInfo CTerrainMeshComponent::GetTerrainInfo () 
    {
    FTerrainRenderInfo Info;

    if (!IsReadyForRender ())
        {
        return Info;
        }
    if (bIsTransformDirty) UpdateTransform ();
    UpdateTransform ();

        // Базовые Vulkan ресурсы
    Info.VertexBuffer = m_VertexBuffer;
    Info.VertexCount = m_VertexCount;
    Info.IndexBuffer = m_IndexBuffer;
    Info.IndexCount = m_IndexCount;

    // Матрица трансформации
    Info.Model = GetTransformMatrix ();
   


    // Данные о размерах террейна
    if (m_TerrainComponent)
        {
        const FTerrainData & data = m_TerrainComponent->GetTerrainData ();

        Info.Width = data.Width;
        Info.Height = data.Height;
        Info.CellSize = data.CellSize;
        Info.MinHeight = data.MinHeight;
        Info.MaxHeight = data.MaxHeight;

        // Настраиваем параметры для шейдера
        Info.Params.TilingFactor = 1.0f;
        Info.Params.HeightScale = 1.0f;
        Info.Params.FogDensity = 0.001f;
        Info.Params.UseTexture = 0.0f;

        // Высоты для слоёв текстурирования
        float range = data.MaxHeight - data.MinHeight;
        Info.Params.SandHeight = data.MinHeight + range * 0.1f;
        Info.Params.GrassHeight = data.MinHeight + range * 0.3f;
        Info.Params.RockHeight = data.MinHeight + range * 0.6f;
        Info.Params.SnowHeight = data.MinHeight + range * 0.8f;
        }

    Info.PipelineName = GetPipelineName ();
    return Info;
    }

void CTerrainMeshComponent::UpdateFromTerrain ()
    {
    if (!m_TerrainComponent)
        {
        LOG_ERROR ( "[", GetName (), "] No terrain component set!" );
        return;
        }

        // Проверяем, есть ли данные террейна
    const FTerrainData & data = m_TerrainComponent->GetTerrainData ();
    if (data.Width == 0 || data.Height == 0 || data.Heights.empty ())
        {
        LOG_WARN ( "[", GetName (), "] Terrain data not generated yet!" );
        return;
        }

        // Уничтожаем старые ресурсы
    DestroyRenderResources ();

    // Обновляем кэш данных
    m_CachedData = &data;

    // Создаём новые ресурсы
    if (CEngine::Get ().GetRenderer ())
        {
        auto * bufferManager = CEngine::Get ().GetRenderer ()->GetBufferManager ();
        if (bufferManager)
            {
            CreateRenderResources (  );
            }
        }

    LOG_DEBUG ( "[", GetName (), "] Updated from terrain: ",
                data.Width, "x", data.Height, " with ", data.Heights.size (), " height points" );
    }

void CTerrainMeshComponent::GenerateVertices ( std::vector<FMeshVertex> & OutVertices ) const
    {
    OutVertices.clear ();

    if (!m_TerrainComponent)
        {
        LOG_ERROR ( "[", GetName (), "] Cannot generate vertices - no terrain component!" );
        return;
        }

    const FTerrainData & data = m_TerrainComponent->GetTerrainData ();

    if (data.Width <= 0 || data.Height <= 0 || data.Heights.empty ())
        {
        LOG_ERROR ( "[", GetName (), "] Invalid terrain dimensions or no height data!" );
        return;
        }

    OutVertices.reserve ( data.Width * data.Height );

    for (int32 z = 0; z < data.Height; ++z)
        {
        for (int32 x = 0; x < data.Width; ++x)
            {
            FMeshVertex vert;

            // Позиция из компонента коллизии (чтобы гарантировать точное совпадение)
            vert.Position = m_TerrainComponent->GetWorldPositionAt ( x, z );

            // Нормаль
            vert.Normal = CalculateNormal ( x, z );

            // Цвет на основе высоты
            vert.Color = CalculateColor ( vert.Position.y );

            // UV координаты
            vert.UV.x = static_cast< float >( x ) / ( data.Width - 1 );
            vert.UV.y = static_cast< float >( z ) / ( data.Height - 1 );

            OutVertices.push_back ( vert );
            }
        }

    LOG_DEBUG ( "[", GetName (), "] Generated ", OutVertices.size (), " vertices" );
    }

void CTerrainMeshComponent::GenerateIndices ( std::vector<uint32_t> & OutIndices ) const
    {
    OutIndices.clear ();

    if (!m_TerrainComponent) return;

    const FTerrainData & data = m_TerrainComponent->GetTerrainData ();

    if (data.Width < 2 || data.Height < 2) return;

    OutIndices.reserve ( ( data.Width - 1 ) * ( data.Height - 1 ) * 6 );

    for (int32 z = 0; z < data.Height - 1; ++z)
        {
        for (int32 x = 0; x < data.Width - 1; ++x)
            {
            uint32_t i0 = z * data.Width + x;
            uint32_t i1 = z * data.Width + x + 1;
            uint32_t i2 = ( z + 1 ) * data.Width + x;
            uint32_t i3 = ( z + 1 ) * data.Width + x + 1;

            // Первый треугольник
            OutIndices.push_back ( i0 );
            OutIndices.push_back ( i1 );
            OutIndices.push_back ( i2 );

            // Второй треугольник
            OutIndices.push_back ( i1 );
            OutIndices.push_back ( i3 );
            OutIndices.push_back ( i2 );
            }
        }
    }

FVector CTerrainMeshComponent::CalculateNormal ( int32 x, int32 z ) const
    {
    if (!m_TerrainComponent)
        {
        return FVector ( 0.0f, 1.0f, 0.0f );
        }

    const FTerrainData & data = m_TerrainComponent->GetTerrainData ();
    float cellSize = data.CellSize;

    // Получаем позицию текущей точки
    FVector pos = m_TerrainComponent->GetWorldPositionAt ( x, z );

    // Высоты соседних точек
    float hL = ( x > 0 ) ? m_TerrainComponent->GetHeightAtWorld ( FVector ( pos.x - cellSize, 0, pos.z ) ) : pos.y;
    float hR = ( x < data.Width - 1 ) ? m_TerrainComponent->GetHeightAtWorld ( FVector ( pos.x + cellSize, 0, pos.z ) ) : pos.y;
    float hD = ( z > 0 ) ? m_TerrainComponent->GetHeightAtWorld ( FVector ( pos.x, 0, pos.z - cellSize ) ) : pos.y;
    float hU = ( z < data.Height - 1 ) ? m_TerrainComponent->GetHeightAtWorld ( FVector ( pos.x, 0, pos.z + cellSize ) ) : pos.y;

    // Вычисляем нормаль через разности высот
    FVector normal ( hL - hR, 2.0f * cellSize, hD - hU );

    float length = normal.Length ();
    if (length < 0.0001f)
        {
        return FVector ( 0.0f, 1.0f, 0.0f );
        }
   
    return normal / length;
    }

FVector CTerrainMeshComponent::CalculateColor ( float Height ) const
    {
    if (!m_TerrainComponent)
        {
        return FVector ( 0.5f, 0.5f, 0.5f );
        }

    const FTerrainData & data = m_TerrainComponent->GetTerrainData ();
    float heightRange = data.MaxHeight - data.MinHeight;

    if (heightRange < 0.0001f)
        {
        return FVector ( 0.5f, 0.5f, 0.5f );
        }

    float t = ( Height - data.MinHeight ) / heightRange;
    t = CEMath::Clamp ( t, 0.0f, 1.0f );

    // Градиент: синий (низ) -> зелёный -> красный (высокий)
    if (t < 0.5f)
        {
        float t2 = t * 2.0f;
        return FVector ( 0.0f, t2, 1.0f - t2 );
        }
    else
        {
        float t2 = ( t - 0.5f ) * 2.0f;
        return FVector ( t2, 1.0f - t2, 0.0f );
        }
    }