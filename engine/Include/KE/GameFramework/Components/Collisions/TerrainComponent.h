#pragma once

#include "KE/GameFramework/Components/Collisions/BaseCollisionComponent.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

class BufferManager;
struct FTerrainVertex;
// Структура для хранения данных ландшафта
struct FTerrainData
    {
    std::vector<float> Heights;      // Высоты в каждой точке
    int32 Width = 0;                  // Количество точек по X
    int32 Height = 0;                  // Количество точек по Z
    float CellSize = 100.0f;           // Размер ячейки в мировых единицах
    FVector Origin = FVector::Zero ();  // Центр ландшафта

    // Минимальная/максимальная высота
    float MinHeight = 0.0f;
    float MaxHeight = 0.0f;

    // Получить высоту в точке (x, z) в локальных координатах
    float GetHeight ( int32 x, int32 z ) const
        {
        if (x < 0 || x >= Width || z < 0 || z >= Height)
            return 0.0f;
        return Heights[ z * Width + x ];
        }

        // Получить интерполированную высоту в произвольной точке
    float GetInterpolatedHeight ( float worldX, float worldZ ) const;
    };

    // Структура вершины для рендера террейна


class KE_API CTerrainComponent : public CBaseCollisionComponent
    {
    CHUDDO_DECLARE_CLASS ( CTerrainComponent, CBaseCollisionComponent );

    public:
        CTerrainComponent ( CObject * inOwner = nullptr, const std::string & InName = "TerrainComponent" );
        virtual ~CTerrainComponent ();

        virtual void InitComponent () override;
        virtual void Tick ( float DeltaTime ) override;
        virtual void OnBeginPlay () override;

        // ========== КОЛЛИЗИИ ==========
        virtual bool CheckCollision ( CBaseCollisionComponent * other, FCollisionInfo & outInfo ) const override;
        virtual float GetCollisionRadius () const override { return 0.0f; }
        virtual FVector GetBoundingBox () const override;

        // ========== ГЕНЕРАЦИЯ ТЕРРЕЙНА ==========
        void GenerateFlat ( int32 width, int32 height, float cellSize, float heightValue = 0.0f );
        void GenerateFromHeightmap ( const std::vector<float> & heights, int32 width, int32 height, float cellSize );
        void SetHeightAt ( int32 x, int32 z, float height );

        /**
         * Создать холмистый террейн с использованием синусоидальной функции
         */
        void GenerateHilly ( int32 width, int32 height, float cellSize,
                             float amplitude = 50.0f, float frequency = 0.05f );

          /**
           * Создать холмистый террейн с использованием шума Перлина (упрощённый)
           */
        void GenerateNoise ( int32 width, int32 height, float cellSize, int32 seed = 0 );

        /**
         * Создать террейн с кастомной функцией высоты
         */
        void GenerateCustom ( int32 width, int32 height, float cellSize,
                              std::function<float ( int32 x, int32 z )> heightFunc );

          /**
           * Загрузить террейн из heightmap изображения (черно-белого)
           */
        bool LoadFromHeightmap ( const std::string & filename, float cellSize );

        // ========== ЗАПРОСЫ ВЫСОТЫ ==========
        float GetHeightAtWorld ( const FVector & worldPos ) const;
        float GetHeightAtLocal ( float localX, float localZ ) const;
        FVector GetWorldPositionAt ( int32 x, int32 z ) const;

        // ========== ДАННЫЕ ТЕРРЕЙНА ==========
        const FTerrainData & GetTerrainData () const { return m_TerrainData; }

        // ========== РЕНДЕР РЕСУРСЫ ==========
        void CreateRenderResources ( BufferManager * BufferManager );
        void DestroyRenderResources ();

        bool HasRenderResources () const
            {
            return m_VertexBuffer != VK_NULL_HANDLE && m_VertexCount > 0;
            }

        VkBuffer GetVertexBuffer () const { return m_VertexBuffer; }
        VkBuffer GetIndexBuffer () const { return m_IndexBuffer; }
        uint32_t GetVertexCount () const { return m_VertexCount; }
        uint32_t GetIndexCount () const { return m_IndexCount; }

        // ========== ОТЛАДКА ==========
        void EnableDebugDraw ( bool enable ) { m_bDebugDraw = enable; }

    private:
        // ========== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ==========
        bool RaycastTerrain ( const FVector & start, const FVector & dir, float maxDist,
                              FVector & outHit, FVector & outNormal, float & outDist ) const;

          // ========== ГЕНЕРАЦИЯ ГЕОМЕТРИИ ДЛЯ РЕНДЕРА ==========
        void GenerateVertices ( std::vector<FTerrainVertex> & vertices ) const;
        void GenerateIndices ( std::vector<uint32_t> & indices ) const;

    private:
        // ========== ДАННЫЕ ТЕРРЕЙНА ==========
        FTerrainData m_TerrainData;

        // ========== РЕНДЕР РЕСУРСЫ ==========
        VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
        VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
        uint32_t m_VertexCount = 0;
        uint32_t m_IndexCount = 0;

        // ========== ОТЛАДКА ==========
        bool m_bDebugDraw = false;
    };

REGISTER_CLASS_FACTORY ( CTerrainComponent );