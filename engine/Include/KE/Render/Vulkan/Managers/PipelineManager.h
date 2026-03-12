#pragma once
#include "Render/Vulkan/VulkanInterface.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

struct FEngineInfo;
class CDescriptorManager;


struct KE_API FShaderModule
    {
    VkShaderModule Module = VK_NULL_HANDLE;
    VkShaderStageFlagBits Stage;
    std::string EntryPoint = "main";

    bool IsValid () const { return Module != VK_NULL_HANDLE; }
    };

struct KE_API FVertexInputDescription
    {
    std::vector<VkVertexInputBindingDescription> Bindings;
    std::vector<VkVertexInputAttributeDescription> Attributes;
    };

struct KE_API FGraphicsPipelineConfig
    {
    // Vertex input
    FVertexInputDescription VertexInput;

    // Input assembly
    VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkBool32 PrimitiveRestartEnable = VK_FALSE;

    // Rasterizer
    VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
    float LineWidth = 1.0f;
    VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace FrontFace = VK_FRONT_FACE_CLOCKWISE;
    VkBool32 DepthBiasEnable = VK_FALSE;
    float DepthBiasConstantFactor = 0.0f;
    float DepthBiasClamp = 0.0f;
    float DepthBiasSlopeFactor = 0.0f;

    // Multisampling
    VkSampleCountFlagBits Samples = VK_SAMPLE_COUNT_1_BIT;
    VkBool32 SampleShadingEnable = VK_FALSE;
    float MinSampleShading = 1.0f;

    // Depth/stencil
    VkBool32 DepthTestEnable = VK_TRUE;
    VkBool32 DepthWriteEnable = VK_TRUE;
    VkCompareOp DepthCompareOp = VK_COMPARE_OP_LESS;
    VkBool32 DepthBoundsTestEnable = VK_FALSE;
    VkBool32 StencilTestEnable = VK_FALSE;

    // Color blend
    VkBool32 BlendEnable = VK_FALSE;
    VkBlendFactor SrcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor DstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendOp ColorBlendOp = VK_BLEND_OP_ADD;
    VkBlendFactor SrcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor DstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendOp AlphaBlendOp = VK_BLEND_OP_ADD;

    // Dynamic states
    std::vector<VkDynamicState> DynamicStates;
    };

// Структура для хранения пайплайна и его layout вместе
struct KE_API FPipelineResource
    {
    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkPipelineLayout Layout = VK_NULL_HANDLE;
    std::string Name;
    VkRenderPass RenderPass = VK_NULL_HANDLE;
    FGraphicsPipelineConfig Config;
    bool bIsValid = false;

    bool IsValid () const { return Pipeline != VK_NULL_HANDLE && Layout != VK_NULL_HANDLE; }
    };

class KE_API CPipelineManager final : public IVulkanManager
    {
    public:
        CPipelineManager ( FEngineInfo & Info );
        virtual ~CPipelineManager ();

        // IVulkanManager
        bool Initialize () override;
        void Shutdown () override;
        const char * GetManagerName () const override;

        // Регистрация всех стандартных пайплайнов
        bool RegisterDefaultPipelines ( VkRenderPass MainRenderPass );

        // Shader loading
        VkShaderModule CreateShaderModule ( const std::vector<char> & Code );
        VkShaderModule LoadShader ( const std::string & Filename );
        FShaderModule LoadShaderModule ( const std::string & Filename, VkShaderStageFlagBits Stage );
        void SetDescriptorManager ( CDescriptorManager * DescMgr ) { m_DescMgr = DescMgr; }
        // Pipeline layout creation with caching
        VkPipelineLayout CreatePipelineLayout (
            const std::string & LayoutName,
            const std::vector<VkDescriptorSetLayout> & DescSetLayouts = {},  
            const std::vector<VkPushConstantRange> & PushConstants = {} );

        // Graphics pipeline creation with caching
        VkPipeline CreateGraphicsPipeline (
            const std::string & PipelineName,
            const std::vector<FShaderModule> & Shaders,
            VkPipelineLayout Layout,
            VkRenderPass RenderPass,
            const FGraphicsPipelineConfig & Config = FGraphicsPipelineConfig (),
            uint32_t Subpass = 0 );

        // Get pipeline by name
        VkPipeline GetPipeline ( const std::string & Name ) const;
        VkPipelineLayout GetPipelineLayout ( const std::string & Name ) const;
        FPipelineResource GetPipelineResource ( const std::string & Name ) const;

        // Check existence
        bool HasPipeline ( const std::string & Name ) const;
        bool HasPipelineLayout ( const std::string & Name ) const;

        // Destroy specific pipeline
        bool DestroyPipeline ( const std::string & Name );
        bool DestroyPipelineLayout ( const std::string & Name );

        // Cleanup individual objects
        void DestroyShaderModule ( VkShaderModule Module );
        void DestroyPipelineLayout ( VkPipelineLayout Layout );
        void DestroyPipeline ( VkPipeline Pipeline );

        // Для обратной совместимости (временные методы)
        VkPipeline CreateTrianglePipeline ( VkRenderPass RenderPass );

    private:
        CDescriptorManager * m_DescMgr = nullptr;
        VkPipeline CreateMeshPipeLine ( VkRenderPass RenderPass );
        VkPipeline CreateTerrainPipeline ( VkRenderPass RenderPass );
        VkPipeline CreateWireframePipeline ( VkRenderPass RenderPass );
        // Vertex input helpers
        FVertexInputDescription GetTriangleVertexInput () const;
        FVertexInputDescription GetUIVertexInput () const;
        FVertexInputDescription GetSkyboxVertexInput () const;

        // State creation helpers
        std::vector<VkDynamicState> GetDefaultDynamicStates () const;
        VkPipelineShaderStageCreateInfo GetShaderStageInfo ( const FShaderModule & Module ) const;
        VkPipelineVertexInputStateCreateInfo GetVertexInputState ( const FVertexInputDescription & Desc ) const;
        VkPipelineInputAssemblyStateCreateInfo GetInputAssemblyState ( const FGraphicsPipelineConfig & Config ) const;
        VkPipelineRasterizationStateCreateInfo GetRasterizationState ( const FGraphicsPipelineConfig & Config ) const;
        VkPipelineMultisampleStateCreateInfo GetMultisampleState ( const FGraphicsPipelineConfig & Config ) const;
        VkPipelineDepthStencilStateCreateInfo GetDepthStencilState ( const FGraphicsPipelineConfig & Config ) const;
        VkPipelineColorBlendAttachmentState GetColorBlendAttachment ( const FGraphicsPipelineConfig & Config ) const;
        VkPipelineColorBlendStateCreateInfo GetColorBlendState ( const FGraphicsPipelineConfig & Config ) const;
        VkPipelineDynamicStateCreateInfo GetDynamicState ( const FGraphicsPipelineConfig & Config ) const;

    private:
        // Cache for loaded shaders
        std::unordered_map<std::string, VkShaderModule> m_ShaderCache;

        // Cache for pipeline layouts
        std::unordered_map<std::string, VkPipelineLayout> m_PipelineLayouts;

        // Main pipeline cache
        std::unordered_map<std::string, FPipelineResource> m_PipelineResources;
    };