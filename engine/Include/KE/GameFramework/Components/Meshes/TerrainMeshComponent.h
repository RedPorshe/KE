#pragma once

#include "Components/Meshes/BaseMeshComponent.h"
#include "Components/Collisions/TerrainComponent.h"

class KE_API CTerrainMeshComponent : public CBaseMeshComponent
    {
    CHUDDO_DECLARE_CLASS ( CTerrainMeshComponent, CBaseMeshComponent )

    public:
        CTerrainMeshComponent ( CObject * inOwner = nullptr, const std::string & inDisplayName = "TerrainMeshComponent" );
        virtual ~CTerrainMeshComponent ();

        // ========== Жизненный цикл ==========
        virtual void InitComponent () override;
        virtual void Tick ( float DeltaTime ) override;
        virtual void OnBeginPlay () override;

        // ========== Привязка к террейну ==========
        /**
         * Установить компонент террейна, из которого брать данные
         */
        void SetTerrainComponent ( CTerrainComponent * TerrainComp ) { m_TerrainComponent = TerrainComp; }

        /**
         * Получить информацию о террейне для рендера
         */
        virtual FTerrainRenderInfo GetTerrainInfo ()  override;

        /**
         * Получить привязанный компонент террейна
         */
        CTerrainComponent * GetTerrainComponent () const { return m_TerrainComponent; }

        /**
         * Обновить геометрию из данных террейна
         * Вызывается после изменений ландшафта
         */
        void UpdateFromTerrain ();

    protected:
        // ========== Переопределённые методы из CBaseMeshComponent ==========
        virtual void GenerateVertices ( std::vector<FMeshVertex> & OutVertices ) const override;
        virtual void GenerateIndices ( std::vector<uint32_t> & OutIndices ) const override;
        virtual const std::string & GetPipelineName () const override { return m_PipelineName; }

    private:
        // ========== Вспомогательные методы ==========
        FVector CalculateNormal ( int32 x, int32 z ) const;
        FVector CalculateColor ( float Height ) const;

    private:
        // ========== Данные ==========
        CTerrainComponent * m_TerrainComponent = nullptr;

        // Кэшированные данные для быстрого доступа
        mutable const FTerrainData * m_CachedData = nullptr;
    };

REGISTER_CLASS_FACTORY ( CTerrainMeshComponent );