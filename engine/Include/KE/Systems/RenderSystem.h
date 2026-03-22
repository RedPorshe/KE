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

        void SetRenderInfo ( const FRenderInfo & RenderInfo ) { m_RenderInfo = RenderInfo; }
        const FRenderInfo & GetRenderInfo () const { return m_RenderInfo; }
        class BufferManager * GetBufferManager () const { return BuffMgr; }
    private:
        void OnWindowResize ( int width, int height );
        bool RenderTriangle ();              // Fallback - отрисовка треугольника
        bool RenderScene ();

        FRenderInfo m_RenderInfo;

        TUniquePtr<VulkanContext> m_vulkanContext;
        bool CacheManagers ();
        class SwapchainManager * Swapchain = nullptr;
        class CommandManager * CmdManager = nullptr;
        class PipelineManager * PipelineMgr = nullptr;
        class RenderPassManager * RenderPassMgr = nullptr;
        class CSyncManager* SyncMgr = nullptr;
        class BufferManager * BuffMgr = nullptr;

        VkPipelineStageFlags m_WaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
              
        GLFWwindow * m_window = nullptr;

        void CreateTriangleBuffer ();
        FBuffer m_triangleVertexBuffer;
        static const VkClearValue m_clearValues[ 2 ];
    };