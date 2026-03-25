#pragma once

#include "KE/GameFramework/Components/SceneComponent.h"
#include "KE/Vulkan/RenderInfo.h"
#include "KE/GameFramework/Meshes/BaseMesh.h"

// Forward declarations
class CBufferManager;
struct FVertexInputDescription;

/**
 * Базовый класс для всех компонентов, которые могут рендерить меши
 * Наследуется от CSceneComponent для поддержки трансформаций
 * Использует CBaseMesh как источник данных о геометрии
 */
class KE_API CBaseMeshComponent : public CSceneComponent
    {
    CHUDDO_DECLARE_ABSTRACT_CLASS ( CBaseMeshComponent, CSceneComponent )

    public:
        CBaseMeshComponent ( CObject * inOwner = nullptr, const std::string & inDisplayName = "BaseMeshComponent" );
        virtual ~CBaseMeshComponent ();

        // ========== Жизненный цикл ==========
        virtual void InitComponent () override;
        virtual void Tick ( float DeltaTime ) override;
        virtual void OnBeginPlay () override;
        virtual void OnEndPlay () override;

        // ========== Управление мешем ==========
        /**
         * Установить меш по пути к файлу
         * @param MeshPath - путь к файлу меша (относительно ModelsPath)
         */
        void SetMesh ( const std::string & MeshPath );

        /**
         * Установить готовый меш
         * @param InMeshData - указатель на данные меша
         */
        void SetMesh ( CBaseMesh * InMeshData );

        /**
         * Получить указатель на меш
         */
        CBaseMesh * GetMesh () const { return MeshData; }

        /**
         * Проверить, есть ли данные меша
         */
        bool HasMeshData () const { return MeshData && MeshData->IsValid (); }

        // ========== Интерфейс для рендера ==========
        /**
         * Получить информацию о меше для рендера
         * Использует мировую матрицу трансформации из CTransformComponent
         * @return структура с информацией о меше
         */
        virtual FMeshInfo GetMeshInfo ();

        virtual FTerrainRenderInfo GetTerrainInfo ()
            {
            return FTerrainRenderInfo ();
            }

            /**
             * Проверить, готов ли компонент к рендеру
             */
        virtual bool IsReadyForRender () const;

        /**
         * Создать Vulkan ресурсы (буферы и т.д.)
         */
        virtual void CreateRenderResources ();

        /**
         * Уничтожить Vulkan ресурсы
         */
        virtual void DestroyRenderResources ();

        // ========== Управление материалами ==========
        void SetMaterialID ( uint32_t MaterialID ) { m_MaterialID = MaterialID; }
        uint32_t GetMaterialID () const { return m_MaterialID; }

        // ========== Видимость ==========
        void SetVisible ( bool bVisible ) { m_bVisible = bVisible; }
        bool IsVisible () const { return m_bVisible; }

        // ========== Геттеры для Vulkan ресурсов ==========
        VkBuffer GetVertexBuffer () const { return m_VertexBuffer; }
        VkBuffer GetIndexBuffer () const { return m_IndexBuffer; }
        uint32_t GetVertexCount () const { return m_VertexCount; }
        uint32_t GetIndexCount () const { return m_IndexCount; }

        bool HasRenderResources () const
            {
            return m_VertexBuffer != VK_NULL_HANDLE && m_VertexCount > 0;
            }

            // ========== Имя пайплайна ==========
        void SetPipelineName ( const std::string & PipelineName ) { m_PipelineName = PipelineName; }
        virtual const std::string & GetPipelineName () const { return m_PipelineName; }

    protected:
        // ========== Методы для переопределения в наследниках ==========
        /**
         * Генерировать вершины меша (если не используется CBaseMesh)
         * Должен быть переопределён в наследниках для процедурной генерации
         */
        virtual void GenerateVertices ( std::vector<struct FMeshVertex> & OutVertices ) const;

        /**
         * Генерировать индексы меша (если не используется CBaseMesh)
         * Должен быть переопределён в наследниках для процедурной генерации
         */
        virtual void GenerateIndices ( std::vector<uint32_t> & OutIndices ) const;

        /**
         * Получить описание вершин для Vulkan
         * Можно переопределить в наследниках для кастомного формата
         */
        virtual FVertexInputDescription GetVertexInputDescription () const;

        /**
         * Обновить данные из меша
         */
        void UpdateFromMeshData ();

    protected:
        // ========== Данные меша ==========
        CBaseMesh* MeshData;  // Данные меша (вершины, индексы)

        // ========== Vulkan ресурсы ==========
        VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
        VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_VertexMemory = VK_NULL_HANDLE;
        VkDeviceMemory m_IndexMemory = VK_NULL_HANDLE;
        void * m_MappedVertexMemory = nullptr;
        void * m_MappedIndexMemory = nullptr;
        VkDeviceSize m_VertexBufferSize = 0;
        VkDeviceSize m_IndexBufferSize = 0;

        uint32_t m_VertexCount = 0;
        uint32_t m_IndexCount = 0;

        // ========== Материал и пайплайн ==========
        uint32_t m_MaterialID = 0;
        std::string m_PipelineName = "StaticMesh";

        // ========== Состояние ==========
        bool m_bVisible = true;
        bool m_bRenderResourcesCreated = false;
        bool m_bUseMeshData = false;  // Используем ли MeshData
    };

REGISTER_CLASS_FACTORY ( CBaseMeshComponent );