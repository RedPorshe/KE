#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "KE/Vulkan/VertexStructs/FVertexInputDescription.h"

class PipelineConfig
    {
    public:
        PipelineConfig ();

        // Fluent setters
        PipelineConfig & SetVertexInput ( const FVertexInputDescription & input );
        PipelineConfig & SetTopology ( VkPrimitiveTopology topology );
        PipelineConfig & SetPrimitiveRestartEnable ( bool enable );
        PipelineConfig & SetPolygonMode ( VkPolygonMode mode );
        PipelineConfig & SetCullMode ( VkCullModeFlags mode );
        PipelineConfig & SetFrontFace ( VkFrontFace face );
        PipelineConfig & SetLineWidth ( float width );
        PipelineConfig & SetDepthBias ( bool enable, float constantFactor = 0.0f, float clamp = 0.0f, float slopeFactor = 0.0f );

        // Multisampling
        PipelineConfig & SetSamples ( VkSampleCountFlagBits samples );
        PipelineConfig & SetSampleShading ( bool enable, float minShading = 1.0f );

        // Depth/stencil
        PipelineConfig & EnableDepthTest ( bool enable = true );
        PipelineConfig & SetDepthWrite ( bool enable );
        PipelineConfig & SetDepthCompareOp ( VkCompareOp op );
        PipelineConfig & SetDepthBoundsTest ( bool enable );
        PipelineConfig & SetStencilTest ( bool enable );

        // Blending
        PipelineConfig & EnableBlending ( bool enable = true );
        PipelineConfig & SetBlendFactors ( VkBlendFactor srcColor, VkBlendFactor dstColor,
                                           VkBlendOp colorOp = VK_BLEND_OP_ADD,
                                           VkBlendFactor srcAlpha = VK_BLEND_FACTOR_ONE,
                                           VkBlendFactor dstAlpha = VK_BLEND_FACTOR_ZERO,
                                           VkBlendOp alphaOp = VK_BLEND_OP_ADD );

          // Dynamic states
        PipelineConfig & SetDynamicStates ( const std::vector<VkDynamicState> & states );
        PipelineConfig & AddDynamicState ( VkDynamicState state );
        PipelineConfig & ClearDynamicStates ();

        struct ConfigData
            {
            FVertexInputDescription VertexInput;
            VkPrimitiveTopology Topology;
            VkBool32 PrimitiveRestartEnable;
            VkPolygonMode PolygonMode;
            float LineWidth;
            VkCullModeFlags CullMode;
            VkFrontFace FrontFace;
            VkBool32 DepthBiasEnable;
            float DepthBiasConstantFactor;
            float DepthBiasClamp;
            float DepthBiasSlopeFactor;
            VkSampleCountFlagBits Samples;
            VkBool32 SampleShadingEnable;
            float MinSampleShading;
            VkBool32 DepthTestEnable;
            VkBool32 DepthWriteEnable;
            VkCompareOp DepthCompareOp;
            VkBool32 DepthBoundsTestEnable;
            VkBool32 StencilTestEnable;
            VkBool32 BlendEnable;
            VkBlendFactor SrcColorBlendFactor;
            VkBlendFactor DstColorBlendFactor;
            VkBlendOp ColorBlendOp;
            VkBlendFactor SrcAlphaBlendFactor;
            VkBlendFactor DstAlphaBlendFactor;
            VkBlendOp AlphaBlendOp;
            std::vector<VkDynamicState> DynamicStates;
            };

        ConfigData Build () const;

    private:
        ConfigData m_data;
    };