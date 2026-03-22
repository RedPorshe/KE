#include "KE/GameFramework/Components/Collisions/TerrainComponent.h"
#include "KE/GameFramework/Actors/Actor.h"
#include "KE/Vulkan/Managers/BufferManager.h"
#include "KE/Systems/RenderSystem.h"
#include "KE/Systems/CollisionSystem.h"
#include "KE/Engine.h"
#include "KE/Vulkan/VertexStructs/AllVertices.h"
#include <cmath>
#include <algorithm>
#include <limits>

// ============================================================================
// FTerrainData Implementation
// ============================================================================

float FTerrainData::GetInterpolatedHeight ( float worldX, float worldZ ) const
    {
    if (Width < 2 || Height < 2)
        return 0.0f;

    // Преобразуем мировые координаты в локальные относительно центра
    float localX = worldX - Origin.x;
    float localZ = worldZ - Origin.z;

    // Находим ячейку
    float cellX = ( localX / CellSize ) + ( Width - 1 ) * 0.5f;
    float cellZ = ( localZ / CellSize ) + ( Height - 1 ) * 0.5f;

    int32 x1 = static_cast< int32 >( std::floor ( cellX ) );
    int32 z1 = static_cast< int32 >( std::floor ( cellZ ) );
    int32 x2 = x1 + 1;
    int32 z2 = z1 + 1;

    // Проверяем границы
    if (x1 < 0 || x2 >= Width || z1 < 0 || z2 >= Height)
        return 0.0f;

    // Веса для интерполяции
    float fx = cellX - x1;
    float fz = cellZ - z1;

    // Получаем высоты четырёх углов
    float h11 = GetHeight ( x1, z1 );
    float h21 = GetHeight ( x2, z1 );
    float h12 = GetHeight ( x1, z2 );
    float h22 = GetHeight ( x2, z2 );

    // Билинейная интерполяция
    float h1 = h11 * ( 1.0f - fx ) + h21 * fx;
    float h2 = h12 * ( 1.0f - fx ) + h22 * fx;

    return h1 * ( 1.0f - fz ) + h2 * fz;
    }

    // ============================================================================
    // CTerrainComponent Implementation
    // ============================================================================

CTerrainComponent::CTerrainComponent ( CObject * inOwner, const std::string & InName )
    : Super ( inOwner, InName )
    {
    SetShapeType ( ECollisionShape::TERRAIN );
    SetChannelAsStatic ();
    LOG_DEBUG ( "TerrainComponent created: ", GetName () );
    }

CTerrainComponent::~CTerrainComponent ()
    {
    DestroyRenderResources ();
    m_TerrainData.Heights.clear ();
    }

void CTerrainComponent::InitComponent ()
    {
    Super::InitComponent ();
    }

void CTerrainComponent::Tick ( float DeltaTime )
    {
    Super::Tick ( DeltaTime );
    }

void CTerrainComponent::OnBeginPlay ()
    {
    Super::OnBeginPlay ();

    // Создаём рендер ресурсы после того, как террейн сгенерирован
    auto & engine = CEngine::Get ();
    if (engine.GetRenderer ())
        {
        auto bufferManager = engine.GetRenderer ()->GetBufferManager ();
        if (bufferManager)
            {
            CreateRenderResources ( bufferManager );
            }
        }
    }

    // ============================================================================
    // Terrain Generation
    // ============================================================================

void CTerrainComponent::GenerateFlat ( int32 width, int32 height, float cellSize, float heightValue )
    {
    DestroyRenderResources ();

    m_TerrainData.Width = width;
    m_TerrainData.Height = height;
    m_TerrainData.CellSize = cellSize;
    m_TerrainData.MinHeight = heightValue;
    m_TerrainData.MaxHeight = heightValue;

    size_t totalPoints = width * height;
    m_TerrainData.Heights.resize ( totalPoints, heightValue );

    // Вычисляем центр
    float totalWidth = ( width - 1 ) * cellSize;
    float totalDepth = ( height - 1 ) * cellSize;
    m_TerrainData.Origin = FVector ( totalWidth * 0.5f, 0.0f, totalDepth * 0.5f );

    LOG_DEBUG ( "Terrain generated: ", width, "x", height, " cells, size: ",
                totalWidth, " x ", totalDepth );
    }

void CTerrainComponent::GenerateFromHeightmap ( const std::vector<float> & heights,
                                                int32 width, int32 height, float cellSize )
    {
    DestroyRenderResources ();

    if (heights.size () != static_cast< size_t >( width * height ))
        {
        LOG_ERROR ( "Heightmap size mismatch! Expected ", width * height, " got ", heights.size () );
        return;
        }

    m_TerrainData.Heights = heights;
    m_TerrainData.Width = width;
    m_TerrainData.Height = height;
    m_TerrainData.CellSize = cellSize;

    // Находим мин/макс высоты
    auto [minIt, maxIt] = std::minmax_element ( heights.begin (), heights.end () );
    m_TerrainData.MinHeight = *minIt;
    m_TerrainData.MaxHeight = *maxIt;

    // Вычисляем центр
    float totalWidth = ( width - 1 ) * cellSize;
    float totalDepth = ( height - 1 ) * cellSize;
    m_TerrainData.Origin = FVector ( totalWidth * 0.5f, 0.0f, totalDepth * 0.5f );

    LOG_DEBUG ( "Terrain generated from heightmap: ", width, "x", height,
                ", height range: ", m_TerrainData.MinHeight, " - ", m_TerrainData.MaxHeight );
    }

void CTerrainComponent::SetHeightAt ( int32 x, int32 z, float height )
    {
    if (x >= 0 && x < m_TerrainData.Width && z >= 0 && z < m_TerrainData.Height)
        {
        m_TerrainData.Heights[ z * m_TerrainData.Width + x ] = height;

        // Обновляем мин/макс
        m_TerrainData.MinHeight = std::min ( m_TerrainData.MinHeight, height );
        m_TerrainData.MaxHeight = std::max ( m_TerrainData.MaxHeight, height );
        }
    }

void CTerrainComponent::GenerateHilly ( int32 width, int32 height, float cellSize,
                                        float amplitude, float frequency )
    {
    DestroyRenderResources ();

    m_TerrainData.Width = width;
    m_TerrainData.Height = height;
    m_TerrainData.CellSize = cellSize;

    size_t totalPoints = width * height;
    m_TerrainData.Heights.resize ( totalPoints );

    m_TerrainData.MinHeight = std::numeric_limits<float>::max ();
    m_TerrainData.MaxHeight = -std::numeric_limits<float>::max ();

    LOG_DEBUG ( "[TERRAIN] Generating hilly terrain: ", width, "x", height,
                ", amp=", amplitude, ", freq=", frequency );

      // Генерируем холмы с помощью синусоид
    for (int32 z = 0; z < height; ++z)
        {
        for (int32 x = 0; x < width; ++x)
            {
                // Нормализуем координаты для более равномерных холмов
            float nx = ( float ) x / ( width - 1 ) - 0.5f;
            float nz = ( float ) z / ( height - 1 ) - 0.5f;

            // Комбинация нескольких синусоид для более естественного вида
            float h1 = sin ( nx * 10.0f * frequency ) * cos ( nz * 10.0f * frequency ) * amplitude;
            float h2 = sin ( nx * 23.0f * frequency ) * cos ( nz * 23.0f * frequency ) * amplitude * 0.5f;
            float h3 = sin ( nx * 7.0f * frequency ) * cos ( nz * 7.0f * frequency ) * amplitude * 0.8f;

            // Добавляем шум для реалистичности
            float noise = ( rand () % 1000 ) / 1000.0f * amplitude * 0.2f;

            float heightValue = h1 + h2 + h3 + noise + amplitude * 0.5f;

            m_TerrainData.Heights[ z * width + x ] = heightValue;

            // Обновляем мин/макс
            if (heightValue < m_TerrainData.MinHeight) m_TerrainData.MinHeight = heightValue;
            if (heightValue > m_TerrainData.MaxHeight) m_TerrainData.MaxHeight = heightValue;
            }
        }

        // Вычисляем центр
    float totalWidth = ( width - 1 ) * cellSize;
    float totalDepth = ( height - 1 ) * cellSize;
    m_TerrainData.Origin = FVector ( totalWidth * 0.5f, 0.0f, totalDepth * 0.5f );

    LOG_DEBUG ( "[TERRAIN] Generated height range: ",
                m_TerrainData.MinHeight, " - ", m_TerrainData.MaxHeight );
    }

    // Простая реализация шума
static float Noise ( int32 x, int32 y, int32 seed )
    {
    int32 n = x + y * 57 + seed * 131;
    n = ( n << 13 ) ^ n;
    return ( 1.0f - ( ( n * ( n * n * 60493 + 19990303 ) + 1376312589 ) & 0x7fffffff ) / 1073741824.0f );
    }

void CTerrainComponent::GenerateNoise ( int32 width, int32 height, float cellSize, int32 seed )
    {
    DestroyRenderResources ();

    m_TerrainData.Width = width;
    m_TerrainData.Height = height;
    m_TerrainData.CellSize = cellSize;

    size_t totalPoints = width * height;
    m_TerrainData.Heights.resize ( totalPoints );

    m_TerrainData.MinHeight = std::numeric_limits<float>::max ();
    m_TerrainData.MaxHeight = -std::numeric_limits<float>::max ();

    float frequency = 0.05f;
    float amplitude = 50.0f;

    for (int32 z = 0; z < height; ++z)
        {
        for (int32 x = 0; x < width; ++x)
            {
                // Простой шум с несколькими октавами
            float heightValue = 0;
            float amp = amplitude;
            float freq = frequency;

            for (int octave = 0; octave < 4; ++octave)
                {
                heightValue += Noise ( static_cast< int32 > ( x * freq ),
                                       static_cast< int32 > ( z * freq ), seed ) * amp;
                amp *= 0.5f;
                freq *= 2.0f;
                }

            heightValue += amplitude * 0.5f; // Смещаем вверх

            m_TerrainData.Heights[ z * width + x ] = heightValue;

            if (heightValue < m_TerrainData.MinHeight) m_TerrainData.MinHeight = heightValue;
            if (heightValue > m_TerrainData.MaxHeight) m_TerrainData.MaxHeight = heightValue;
            }
        }

    float totalWidth = ( width - 1 ) * cellSize;
    float totalDepth = ( height - 1 ) * cellSize;
    m_TerrainData.Origin = FVector ( totalWidth * 0.5f, 0.0f, totalDepth * 0.5f );

    LOG_DEBUG ( "Noise terrain generated: ", width, "x", height,
                ", height range: ", m_TerrainData.MinHeight, " - ", m_TerrainData.MaxHeight );
    }

void CTerrainComponent::GenerateCustom ( int32 width, int32 height, float cellSize,
                                         std::function<float ( int32 x, int32 z )> heightFunc )
    {
    DestroyRenderResources ();

    m_TerrainData.Width = width;
    m_TerrainData.Height = height;
    m_TerrainData.CellSize = cellSize;

    size_t totalPoints = width * height;
    m_TerrainData.Heights.resize ( totalPoints );

    m_TerrainData.MinHeight = std::numeric_limits<float>::max ();
    m_TerrainData.MaxHeight = -std::numeric_limits<float>::max ();

    for (int32 z = 0; z < height; ++z)
        {
        for (int32 x = 0; x < width; ++x)
            {
            float heightValue = heightFunc ( x, z );
            m_TerrainData.Heights[ z * width + x ] = heightValue;

            if (heightValue < m_TerrainData.MinHeight) m_TerrainData.MinHeight = heightValue;
            if (heightValue > m_TerrainData.MaxHeight) m_TerrainData.MaxHeight = heightValue;
            }
        }

    float totalWidth = ( width - 1 ) * cellSize;
    float totalDepth = ( height - 1 ) * cellSize;
    m_TerrainData.Origin = FVector ( totalWidth * 0.5f, 0.0f, totalDepth * 0.5f );

    LOG_DEBUG ( "Custom terrain generated: ", width, "x", height,
                ", height range: ", m_TerrainData.MinHeight, " - ", m_TerrainData.MaxHeight );
    }

bool CTerrainComponent::LoadFromHeightmap ( const std::string & filename, float cellSize )
    {
        // TODO: Реализовать загрузку изображения
    LOG_ERROR ( "LoadFromHeightmap not implemented yet" );
    return false;
    }

    // ============================================================================
    // Height Queries
    // ============================================================================

float CTerrainComponent::GetHeightAtWorld ( const FVector & worldPos ) const
    {
    // Сначала проверяем, находится ли точка в пределах террейна
    float halfWidth = ( m_TerrainData.Width - 1 ) * m_TerrainData.CellSize * 0.5f;
    float halfDepth = ( m_TerrainData.Height - 1 ) * m_TerrainData.CellSize * 0.5f;

    float minX = m_TerrainData.Origin.x - halfWidth;
    float maxX = m_TerrainData.Origin.x + halfWidth;
    float minZ = m_TerrainData.Origin.z - halfDepth;
    float maxZ = m_TerrainData.Origin.z + halfDepth;

    // Если точка за границами террейна, возвращаем специальное значение
    if (worldPos.x < minX || worldPos.x > maxX || worldPos.z < minZ || worldPos.z > maxZ)
        {
        // Возвращаем -INF или очень большое отрицательное число, чтобы объект падал
        return -std::numeric_limits<float>::max ();
        }

    return m_TerrainData.GetInterpolatedHeight ( worldPos.x, worldPos.z );
    }

float CTerrainComponent::GetHeightAtLocal ( float localX, float localZ ) const
    {
    return m_TerrainData.GetInterpolatedHeight (
        localX + m_TerrainData.Origin.x,
        localZ + m_TerrainData.Origin.z
    );
    }

FVector CTerrainComponent::GetWorldPositionAt ( int32 x, int32 z ) const
    {
    float worldX = m_TerrainData.Origin.x + ( x - ( m_TerrainData.Width - 1 ) * 0.5f ) * m_TerrainData.CellSize;
    float worldZ = m_TerrainData.Origin.z + ( z - ( m_TerrainData.Height - 1 ) * 0.5f ) * m_TerrainData.CellSize;
    float worldY = m_TerrainData.GetHeight ( x, z );

    return FVector ( worldX, worldY, worldZ );
    }

FVector CTerrainComponent::GetBoundingBox () const
    {
    float totalWidth = ( m_TerrainData.Width - 1 ) * m_TerrainData.CellSize;
    float totalDepth = ( m_TerrainData.Height - 1 ) * m_TerrainData.CellSize;
    float heightRange = m_TerrainData.MaxHeight - m_TerrainData.MinHeight;

    return FVector ( totalWidth, heightRange, totalDepth );
    }

    // ============================================================================
    // Collision Checks
    // ============================================================================

bool CTerrainComponent::CheckCollision ( CBaseCollisionComponent * other, FCollisionInfo & outInfo ) const
    {
    if (!other || !IsCollisionEnabled () || !other->IsCollisionEnabled ())
        return false;

    if (!CanCollideWith ( other ))
        return false;

    ECollisionShape otherShape = other->GetShapeType ();
    CTerrainComponent * nonConstThis = const_cast< CTerrainComponent * >( this );

    switch (otherShape)
        {
            case ECollisionShape::NONE:
                return false;

            case ECollisionShape::SPHERE:
                return COLLISION_SYSTEM.CheckSphereTerrain ( other, nonConstThis, outInfo );

            case ECollisionShape::BOX:
                return COLLISION_SYSTEM.CheckBoxTerrain ( other, nonConstThis, outInfo );

            case ECollisionShape::CAPSULE:
                return COLLISION_SYSTEM.CheckCapsuleTerrain ( other, nonConstThis, outInfo );

            case ECollisionShape::CYLINDER:
                return COLLISION_SYSTEM.CheckCylinderTerrain ( other, nonConstThis, outInfo );

            case ECollisionShape::CONE:
                return COLLISION_SYSTEM.CheckConeTerrain ( other, nonConstThis, outInfo );

            case ECollisionShape::COMPOUND:
            case ECollisionShape::MESH:
            case ECollisionShape::TERRAIN:
            case ECollisionShape::RAY:
            case ECollisionShape::PLANE:
            case ECollisionShape::MAX:
            default:
                return false;
        }
    }

bool CTerrainComponent::RaycastTerrain ( const FVector & start, const FVector & dir, float maxDist,
                                         FVector & outHit, FVector & outNormal, float & outDist ) const
    {
    float step = m_TerrainData.CellSize * 0.5f;
    FVector current = start;
    float traveled = 0.0f;

    while (traveled < maxDist)
        {
        float terrainY = GetHeightAtWorld ( current );

        if (current.y <= terrainY)
            {
            outHit = current;
            outNormal = FVector ( 0.0f, 1.0f, 0.0f ); // Упрощённая нормаль
            outDist = traveled;
            return true;
            }

        current += dir * step;
        traveled += step;
        }

    return false;
    }

    // ============================================================================
    // Render Resources
    // ============================================================================

void CTerrainComponent::CreateRenderResources ( BufferManager * BufferManager )
    {
    if (!BufferManager || m_TerrainData.Heights.empty ())
        {
        return;
        }

    if (HasRenderResources ())
        {
        return;
        }

        // Генерируем вершины
    std::vector<FTerrainVertex> vertices;
    GenerateVertices ( vertices );
    m_VertexCount = static_cast< uint32_t >( vertices.size () );

    // Генерируем индексы
    std::vector<uint32_t> indices;
    GenerateIndices ( indices );
    m_IndexCount = static_cast< uint32_t >( indices.size () );

    // Создаём вершинный буфер
    FBuffer vertexBuffer = BufferManager->CreateVertexBuffer ( vertices );
    if (vertexBuffer.IsValid ())
        {
        m_VertexBuffer = vertexBuffer.Buffer;
        }

        // Создаём индексный буфер
    if (!indices.empty ())
        {
        FBuffer indexBuffer = BufferManager->CreateIndexBuffer ( indices );
        if (indexBuffer.IsValid ())
            {
            m_IndexBuffer = indexBuffer.Buffer;
            }
        }

    LOG_DEBUG ( "[", GetName (), "] Created render resources: ",
                m_VertexCount, " vertices, ", m_IndexCount, " indices" );
    }

void CTerrainComponent::DestroyRenderResources ()
    {
    m_VertexBuffer = VK_NULL_HANDLE;
    m_IndexBuffer = VK_NULL_HANDLE;
    m_VertexCount = 0;
    m_IndexCount = 0;
    }

void CTerrainComponent::GenerateVertices ( std::vector<FTerrainVertex> & vertices ) const
    {
    vertices.clear ();
    vertices.reserve ( m_TerrainData.Width * m_TerrainData.Height );

    for (int32 z = 0; z < m_TerrainData.Height; ++z)
        {
        for (int32 x = 0; x < m_TerrainData.Width; ++x)
            {
            FTerrainVertex vert;
            vert.Position = GetWorldPositionAt ( x, z );

            // Вычисляем нормаль через соседние точки
            float hL = ( x > 0 ) ? GetHeightAtWorld ( FVector ( vert.Position.x - m_TerrainData.CellSize, 0, vert.Position.z ) ) : vert.Position.y;
            float hR = ( x < m_TerrainData.Width - 1 ) ? GetHeightAtWorld ( FVector ( vert.Position.x + m_TerrainData.CellSize, 0, vert.Position.z ) ) : vert.Position.y;
            float hD = ( z > 0 ) ? GetHeightAtWorld ( FVector ( vert.Position.x, 0, vert.Position.z - m_TerrainData.CellSize ) ) : vert.Position.y;
            float hU = ( z < m_TerrainData.Height - 1 ) ? GetHeightAtWorld ( FVector ( vert.Position.x, 0, vert.Position.z + m_TerrainData.CellSize ) ) : vert.Position.y;

            FVector normal ( hL - hR, 2.0f * m_TerrainData.CellSize, hD - hU );
            float normalLength = normal.Length ();
            vert.Normal = ( normalLength > 0.0001f ) ? normal / normalLength : FVector ( 0.0f, 1.0f, 0.0f );

            // Цвет по высоте
            float heightRange = m_TerrainData.MaxHeight - m_TerrainData.MinHeight;
            float t = ( heightRange > 0.0001f ) ? ( vert.Position.y - m_TerrainData.MinHeight ) / heightRange : 0.5f;

            if (t < 0.5f)
                {
                float t2 = t * 2.0f;
                vert.Color = FVector ( 0.0f, t2, 1.0f - t2 );
                }
            else
                {
                float t2 = ( t - 0.5f ) * 2.0f;
                vert.Color = FVector ( t2, 1.0f - t2, 0.0f );
                }

            vert.UV.x = static_cast< float >( x ) / ( m_TerrainData.Width - 1 );
            vert.UV.y = static_cast< float >( z ) / ( m_TerrainData.Height - 1 );

            vertices.push_back ( vert );
            }
        }
    }

void CTerrainComponent::GenerateIndices ( std::vector<uint32_t> & indices ) const
    {
    indices.clear ();

    if (m_TerrainData.Width < 2 || m_TerrainData.Height < 2)
        return;

    indices.reserve ( ( m_TerrainData.Width - 1 ) * ( m_TerrainData.Height - 1 ) * 6 );

    for (int32 z = 0; z < m_TerrainData.Height - 1; ++z)
        {
        for (int32 x = 0; x < m_TerrainData.Width - 1; ++x)
            {
            uint32_t i0 = z * m_TerrainData.Width + x;
            uint32_t i1 = z * m_TerrainData.Width + x + 1;
            uint32_t i2 = ( z + 1 ) * m_TerrainData.Width + x;
            uint32_t i3 = ( z + 1 ) * m_TerrainData.Width + x + 1;

            // Первый треугольник
            indices.push_back ( i0 );
            indices.push_back ( i1 );
            indices.push_back ( i2 );

            // Второй треугольник
            indices.push_back ( i1 );
            indices.push_back ( i3 );
            indices.push_back ( i2 );
            }
        }
    }