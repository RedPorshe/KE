#pragma once

#include "Components/SceneComponent.h"
#include "Render/RenderInfo.h"


// Forward declarations
class CBufferManager;
struct FVertexInputDescription;
/**
 * Базовый класс для всех компонентов, которые могут рендерить меши
 * Наследуется от CSceneComponent для поддержки трансформаций
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

        // ========== Интерфейс для рендера ==========
        /**
         * Получить информацию о меше для рендера
         * Использует мировую матрицу трансформации из CTransformComponent
         * @return структура с информацией о меше
         */
        virtual FMeshInfo GetMeshInfo () ;
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
         * @param BufferManager - менеджер буферов для создания ресурсов
         */
        virtual void CreateRenderResources (  );
        
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
         * Генерировать вершины меша
         * Должен быть переопределён в наследниках
         */
        virtual void GenerateVertices ( std::vector<struct FMeshVertex> & OutVertices ) const;

        /**
         * Генерировать индексы меша
         * Должен быть переопределён в наследниках
         */
        virtual void GenerateIndices ( std::vector<uint32_t> & OutIndices ) const;

        /**
         * Получить описание вершин для Vulkan
         * Можно переопределить в наследниках для кастомного формата
         */
        virtual FVertexInputDescription GetVertexInputDescription () const;  // Убрали = 0, добавили реализацию в cpp

    protected:
        // ========== Данные меша ==========
        std::vector<float> m_VerticesData;      // Временные данные (до создания буферов)
        std::vector<uint32_t> m_Indices;        // Временные данные (до создания буферов)

        // ========== Vulkan ресурсы ==========
        VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
        VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
        uint32_t m_VertexCount = 0;
        uint32_t m_IndexCount = 0;

        // ========== Материал и пайплайн ==========
        uint32_t m_MaterialID = 0;
        std::string m_PipelineName = "StaticMesh";

        // ========== Состояние ==========
        bool m_bVisible = true;
        bool m_bRenderResourcesCreated = false;
    };

   


REGISTER_CLASS_FACTORY ( CBaseMeshComponent );