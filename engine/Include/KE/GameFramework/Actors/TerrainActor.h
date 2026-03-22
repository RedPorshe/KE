#pragma once

#include "KE/GameFramework/Actors/Actor.h"
#include "KE/Vulkan/RenderInfo.h"

class CTerrainComponent;
class CTerrainMeshComponent;

class KE_API CTerrainActor : public CActor
    {
    CHUDDO_DECLARE_CLASS ( CTerrainActor, CActor );

    public:
        CTerrainActor ( CObject * inOwner = nullptr, const std::string & inDisplayName = "TerrainActor" );
        virtual ~CTerrainActor ();

        virtual void BeginPlay () override;
        virtual void Tick ( float deltaTime ) override;
        virtual void EndPlay () override;

        // Доступ к компонентам
        CTerrainComponent * GetTerrainComponent () const;
        CTerrainMeshComponent * GetTerrainMeshComponent () const;

        // ========== МЕТОДЫ ГЕНЕРАЦИИ ТЕРРЕЙНА ==========
        // Плоский террейн
        void GenerateFlat ( int32 width, int32 height, float cellSize, float heightValue = 0.0f );

        // Из карты высот
        void GenerateFromHeightmap ( const std::vector<float> & heights, int32 width, int32 height, float cellSize );

        // Холмистый террейн (синусоидальный)
        void GenerateHilly ( int32 width, int32 height, float cellSize,
                             float amplitude = 50.0f, float frequency = 0.05f );

          // Террейн на основе шума
        void GenerateNoise ( int32 width, int32 height, float cellSize, int32 seed = 0 );

        // Террейн с кастомной функцией высоты
        void GenerateCustom ( int32 width, int32 height, float cellSize,
                              std::function<float ( int32 x, int32 z )> heightFunc );

    private:
        // Вспомогательный метод для создания компонентов
        void CreateTerrainComponents ();

    private:
        CTerrainComponent * m_TerrainComponent = nullptr;
        CTerrainMeshComponent * m_TerrainMeshComponent = nullptr;
    };

REGISTER_CLASS_FACTORY ( CTerrainActor );