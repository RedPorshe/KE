#pragma once
#include "KE/Vulkan/VKManager.h"
#include "KE/Vulkan/VertexStructs/AllVertices.h"
#include "PipelineConfig.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

struct FEngineInfo;
class DescriptorManager;

struct KE_API FShaderModule
    {
    VkShaderModule Module = VK_NULL_HANDLE;
    VkShaderStageFlagBits Stage;
    std::string EntryPoint = "main";

    bool IsValid () const { return Module != VK_NULL_HANDLE; }
    };



    // Для обратной совместимости
struct KE_API FGraphicsPipelineConfig
    {
    FVertexInputDescription VertexInput;
    VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkBool32 PrimitiveRestartEnable = VK_FALSE;
    VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
    float LineWidth = 1.0f;
    VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace FrontFace = VK_FRONT_FACE_CLOCKWISE;
    VkBool32 DepthBiasEnable = VK_FALSE;
    float DepthBiasConstantFactor = 0.0f;
    float DepthBiasClamp = 0.0f;
    float DepthBiasSlopeFactor = 0.0f;
    VkSampleCountFlagBits Samples = VK_SAMPLE_COUNT_1_BIT;
    VkBool32 SampleShadingEnable = VK_FALSE;
    float MinSampleShading = 1.0f;
    VkBool32 DepthTestEnable = VK_TRUE;
    VkBool32 DepthWriteEnable = VK_TRUE;
    VkCompareOp DepthCompareOp = VK_COMPARE_OP_LESS;
    VkBool32 DepthBoundsTestEnable = VK_FALSE;
    VkBool32 StencilTestEnable = VK_FALSE;
    VkBool32 BlendEnable = VK_FALSE;
    VkBlendFactor SrcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    VkBlendFactor DstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    VkBlendOp ColorBlendOp = VK_BLEND_OP_ADD;
    VkBlendFactor SrcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    VkBlendFactor DstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    VkBlendOp AlphaBlendOp = VK_BLEND_OP_ADD;
    std::vector<VkDynamicState> DynamicStates;

    FGraphicsPipelineConfig ();
    };

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

class KE_API PipelineManager final : public IVKManager
    {
    public:
        PipelineManager ();
        virtual ~PipelineManager ();

        void Shutdown () override;

        bool RegisterDefaultPipelines ( VkRenderPass MainRenderPass );

        VkShaderModule CreateShaderModule ( const std::vector<char> & Code );
        VkShaderModule LoadShader ( const std::string & Filename );
        FShaderModule LoadShaderModule ( const std::string & Filename, VkShaderStageFlagBits Stage );
        void SetDescriptorManager ( DescriptorManager * DescMgr ) { m_DescMgr = DescMgr; }

        VkPipelineLayout CreatePipelineLayout (
            const std::string & LayoutName,
            const std::vector<VkDescriptorSetLayout> & DescSetLayouts = {},
            const std::vector<VkPushConstantRange> & PushConstants = {} );

        // Новая версия с PipelineConfig
        VkPipeline CreateGraphicsPipeline (
            const std::string & PipelineName,
            const std::vector<FShaderModule> & Shaders,
            VkPipelineLayout Layout,
            VkRenderPass RenderPass,
            const PipelineConfig::ConfigData & Config,
            uint32_t Subpass = 0 );

        // Старая версия для обратной совместимости
        VkPipeline CreateGraphicsPipeline (
            const std::string & PipelineName,
            const std::vector<FShaderModule> & Shaders,
            VkPipelineLayout Layout,
            VkRenderPass RenderPass,
            const FGraphicsPipelineConfig & Config,
            uint32_t Subpass = 0 );

        VkPipeline GetPipeline ( const std::string & Name ) const;
        VkPipelineLayout GetPipelineLayout ( const std::string & Name ) const;
        FPipelineResource GetPipelineResource ( const std::string & Name ) const;

        bool HasPipeline ( const std::string & Name ) const;
        bool HasPipelineLayout ( const std::string & Name ) const;

        bool DestroyPipeline ( const std::string & Name );
        bool DestroyPipelineLayout ( const std::string & Name );

        void DestroyShaderModule ( VkShaderModule Module );
        void DestroyPipelineLayout ( VkPipelineLayout Layout );
        void DestroyPipeline ( VkPipeline Pipeline );

        VkPipeline CreateTrianglePipeline ( VkRenderPass RenderPass );

    private:
        DescriptorManager * m_DescMgr = nullptr;
        VkPipeline CreateMeshPipeLine ( VkRenderPass RenderPass );
        VkPipeline CreateTerrainPipeline ( VkRenderPass RenderPass );
        VkPipeline CreateWireframePipeline ( VkRenderPass RenderPass );

        FVertexInputDescription GetTriangleVertexInput () const;
       

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
        std::unordered_map<std::string, VkShaderModule> m_ShaderCache;
        std::unordered_map<std::string, VkPipelineLayout> m_PipelineLayouts;
        std::unordered_map<std::string, FPipelineResource> m_PipelineResources;


        mutable VkPipelineColorBlendAttachmentState m_cachedAttachment;
        mutable VkPipelineColorBlendStateCreateInfo m_cachedColorBlendState;
        mutable VkPipelineVertexInputStateCreateInfo m_cachedVertexInputState;
        mutable VkPipelineInputAssemblyStateCreateInfo m_cachedInputAssemblyState;
        mutable VkPipelineRasterizationStateCreateInfo m_cachedRasterizationState;
        mutable VkPipelineMultisampleStateCreateInfo m_cachedMultisampleState;
        mutable VkPipelineDepthStencilStateCreateInfo m_cachedDepthStencilState;
        mutable VkPipelineDynamicStateCreateInfo m_cachedDynamicState;
        mutable std::vector<VkDynamicState> m_cachedDynamicStates;
    public:
        bool Init () override;
        const std::string & GetManagerName () const override;
    };