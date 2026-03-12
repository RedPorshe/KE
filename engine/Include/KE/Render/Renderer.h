#pragma once
#include "Vulkan/VulkanInterface.h"
#include <vector>
#include "Vulkan/Managers/BufferManager.h"
#include "Vulkan/Managers/DescriptorManager.h"
#include "RenderInfo.h"

struct FEngineInfo;
class CSwapChainManager;
class CCommandManager;
class CSyncManager;
class CRenderPassManager;
class CPipelineManager;
class CBufferManager;
class CDescriptorManager;

class KE_API CRenderer : public IVulkanManager
    {
    public:
        CRenderer ( FEngineInfo & inInfo );
        virtual ~CRenderer ();

        bool Initialize () override;
        void Shutdown () override;
        const char * GetManagerName () const override;
        bool RenderScene ();
        CBufferManager * GetBufferManager () { return m_BufferManager; }

    private:
        bool StartFrame ( uint32_t & ImageIndex );
        bool EndFrame ( uint32_t ImageIndex );
        bool CreateSyncObjects ();
        bool CreateCommandBuffers ();
        bool RecordCommandBuffer ( VkCommandBuffer CommandBuffer, uint32_t ImageIndex );
        bool RecreateSwapChainResources ();
        void TriangleStub ( VkCommandBuffer CommandBuffer, uint32_t ImageIndex );
        void RenderWorld ( VkCommandBuffer CommandBuffer, uint32_t ImageIndex );
        void RenderMeshes ( VkCommandBuffer CommandBuffer, uint32_t ImageIndex );
        void RenderTerrain ( VkCommandBuffer CommandBuffer, uint32_t ImageIndex );
        void RenderDebugWireFrame ( VkCommandBuffer CommandBuffer, uint32_t ImageIndex );

        // Методы для работы с дескрипторами и uniform буферами
        bool CreateUniformBuffers ();
        bool CreateDescriptorSets ();
        void UpdateUniformBuffers ( uint32_t FrameIndex );
        void CleanupFrameResources ();

        // Структуры данных для uniform буферов
        struct KE_API FViewProjectionUniform
            {
            glm::mat4x4 View;
            glm::mat4x4 Projection;
            };

        struct KE_API FObjectUniform
            {
            glm::mat4x4 Model;
            glm::vec4 Color;
            glm::vec4 AdditionalData;  // Для специфических параметров
            };

            // Структура для uniform буферов каждого кадра
        struct KE_API FFrameUniformBuffers
            {
            FBuffer ViewProjectionBuffer;  // view + projection матрицы
            FBuffer ObjectBuffer;           // динамический буфер для объектов
            uint32_t CurrentObjectCount = 0;

            void * MappedObjectData = nullptr;

            bool IsValid () const
                {
                return ViewProjectionBuffer.IsValid () && ObjectBuffer.IsValid ();
                }
            };

            // Структура для дескрипторных наборов каждого кадра
        struct KE_API FFrameDescriptorSets
            {
            VkDescriptorSet GlobalSet = VK_NULL_HANDLE;      // view/projection
            std::vector<VkDescriptorSet> PerObjectSets;      // для каждого объекта

            bool IsValid () const
                {
                return GlobalSet != VK_NULL_HANDLE;
                }
            };

    private:
        // Managers (cached for fast access)
        FRenderInfo m_RenderInfo;
        CSwapChainManager * m_SwapChainManager = nullptr;
        CCommandManager * m_CommandManager = nullptr;
        CSyncManager * m_SyncManager = nullptr;
        CRenderPassManager * m_RenderPassManager = nullptr;
        CPipelineManager * m_PipelineManager = nullptr;
        CBufferManager * m_BufferManager = nullptr;
        CDescriptorManager * m_DescriptorManager = nullptr;

        // Command buffers (one per framebuffer)
        std::vector<VkCommandBuffer> m_CommandBuffers;

        // Triangle pipeline (fallback)
        VkPipeline m_TrianglePipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_TrianglePipelineLayout = VK_NULL_HANDLE;
        FBuffer m_TriangleVertexBuffer;

        // Uniform буферы для каждого кадра (double/triple buffering)
        std::vector<FFrameUniformBuffers> m_FrameUniformBuffers;

        // Дескрипторные наборы для каждого кадра
        std::vector<FFrameDescriptorSets> m_FrameDescriptorSets;

        // Layouts для дескрипторов (кэшируем для быстрого доступа)
        VkDescriptorSetLayout m_GlobalLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_PerObjectLayout = VK_NULL_HANDLE;

        // Frame sync objects
        bool m_SyncObjectsCreated = false;

        // Константы
        static constexpr uint32_t MAX_OBJECTS_PER_FRAME = 1000;
        static constexpr VkDeviceSize OBJECT_BUFFER_SIZE = sizeof ( FObjectUniform ) * MAX_OBJECTS_PER_FRAME;
        static constexpr VkDeviceSize VIEW_PROJ_BUFFER_SIZE = sizeof ( FViewProjectionUniform );
    };