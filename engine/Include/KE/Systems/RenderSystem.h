#pragma once
#include "KE/EngineObject.h"
#include <vulkan/vulkan.h>
#include "KE/Vulkan/Managers/BufferManager.h"
#include "KE/Vulkan/RenderInfo.h"

struct GLFWwindow;



// Forward declaration
class VulkanContext;

class KE_API RenderSystem : public IEngineSystem
    {
    public:
        RenderSystem ();
        virtual ~RenderSystem (); 

        bool PreInit () override;
        bool Init () override;
        void Shutdown () override;
        void Update ( float DeltaTime ) override;
        void SetWindow ( GLFWwindow * inWindow ) { m_window = inWindow; }
        const std::string GetSystemName () const override;
        void SetEngineName ( const std::string & inName ); 
        void SetAplicationName ( const std::string & inName );
        class DeviceManager * GetDeviceManager () const { return DeviceMgr; }
        void SetRenderInfo ( const FRenderInfo & RenderInfo ) { m_RenderInfo = RenderInfo; }
        const FRenderInfo & GetRenderInfo () const { return m_RenderInfo; }
        class BufferManager * GetBufferManager () const { return BuffMgr; }

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
        void OnWindowResize ( int width, int height );
        bool RenderTriangle ();              // Fallback - отрисовка треугольника
        bool RenderScene ();
        bool RenderWorld ();
        bool UpdateRenderInfo ();
        FRenderInfo m_RenderInfo;

        TUniquePtr<VulkanContext> m_vulkanContext;
        bool CacheManagers ();
        class SwapchainManager * Swapchain = nullptr;
        class CommandManager * CmdManager = nullptr;
        class PipelineManager * PipelineMgr = nullptr;
        class RenderPassManager * RenderPassMgr = nullptr;
        class DeviceManager * DeviceMgr = nullptr;
        class CSyncManager* SyncMgr = nullptr;
        class BufferManager * BuffMgr = nullptr;
        class DescriptorManager * DescMgr = nullptr;

        VkPipelineStageFlags m_WaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
              
        GLFWwindow * m_window = nullptr;

        void CreateTriangleBuffer ();
        FBuffer m_triangleVertexBuffer;
        static const VkClearValue m_clearValues[ 2 ];

        bool RenderMeshes ( VkCommandBuffer cmd );
        bool RenderTerrain ( VkCommandBuffer cmd );
        bool RenderWireframe ( VkCommandBuffer cmd );

        
        std::vector<FFrameUniformBuffers> m_FrameUniformBuffers;

       // Дескрипторные наборы для каждого кадра
        std::vector<FFrameDescriptorSets> m_FrameDescriptorSets;

        // Layouts для дескрипторов (кэшируем для быстрого доступа)
        VkDescriptorSetLayout m_GlobalLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_PerObjectLayout = VK_NULL_HANDLE;

        static constexpr uint32_t MAX_OBJECTS_PER_FRAME = 1000;
        static constexpr VkDeviceSize OBJECT_BUFFER_SIZE = sizeof ( FObjectUniform ) * MAX_OBJECTS_PER_FRAME;
        static constexpr VkDeviceSize VIEW_PROJ_BUFFER_SIZE = sizeof ( FViewProjectionUniform );


        bool CreateUniformBuffers ();
        bool CreateDescriptorSets ();
        void UpdateUniformBuffers ( uint32_t FrameIndex );
        void CleanupFrameResources ();


    };