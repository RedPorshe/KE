#include "KE/Vulkan/Managers/PipelineConfig.h"

PipelineConfig::PipelineConfig ()
    {
    m_data.VertexInput.Bindings.clear ();
    m_data.VertexInput.Attributes.clear ();
    m_data.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    m_data.PrimitiveRestartEnable = VK_FALSE;
    m_data.PolygonMode = VK_POLYGON_MODE_FILL;
    m_data.LineWidth = 1.0f;
    m_data.CullMode = VK_CULL_MODE_BACK_BIT;
    m_data.FrontFace = VK_FRONT_FACE_CLOCKWISE;
    m_data.DepthBiasEnable = VK_FALSE;
    m_data.DepthBiasConstantFactor = 0.0f;
    m_data.DepthBiasClamp = 0.0f;
    m_data.DepthBiasSlopeFactor = 0.0f;
    m_data.Samples = VK_SAMPLE_COUNT_1_BIT;
    m_data.SampleShadingEnable = VK_FALSE;
    m_data.MinSampleShading = 1.0f;
    m_data.DepthTestEnable = VK_TRUE;
    m_data.DepthWriteEnable = VK_TRUE;
    m_data.DepthCompareOp = VK_COMPARE_OP_LESS;
    m_data.DepthBoundsTestEnable = VK_FALSE;
    m_data.StencilTestEnable = VK_FALSE;
    m_data.BlendEnable = VK_FALSE;
    m_data.SrcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    m_data.DstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    m_data.ColorBlendOp = VK_BLEND_OP_ADD;
    m_data.SrcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    m_data.DstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    m_data.AlphaBlendOp = VK_BLEND_OP_ADD;
    m_data.DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    }

PipelineConfig & PipelineConfig::SetVertexInput ( const FVertexInputDescription & input )
    {
    m_data.VertexInput = input;
    return *this;
    }

PipelineConfig & PipelineConfig::SetTopology ( VkPrimitiveTopology topology )
    {
    m_data.Topology = topology;
    return *this;
    }

PipelineConfig & PipelineConfig::SetPrimitiveRestartEnable ( bool enable )
    {
    m_data.PrimitiveRestartEnable = enable ? VK_TRUE : VK_FALSE;
    return *this;
    }

PipelineConfig & PipelineConfig::SetPolygonMode ( VkPolygonMode mode )
    {
    m_data.PolygonMode = mode;
    return *this;
    }

PipelineConfig & PipelineConfig::SetCullMode ( VkCullModeFlags mode )
    {
    m_data.CullMode = mode;
    return *this;
    }

PipelineConfig & PipelineConfig::SetFrontFace ( VkFrontFace face )
    {
    m_data.FrontFace = face;
    return *this;
    }

PipelineConfig & PipelineConfig::SetLineWidth ( float width )
    {
    m_data.LineWidth = width;
    return *this;
    }

PipelineConfig & PipelineConfig::SetDepthBias ( bool enable, float constantFactor, float clamp, float slopeFactor )
    {
    m_data.DepthBiasEnable = enable ? VK_TRUE : VK_FALSE;
    m_data.DepthBiasConstantFactor = constantFactor;
    m_data.DepthBiasClamp = clamp;
    m_data.DepthBiasSlopeFactor = slopeFactor;
    return *this;
    }

PipelineConfig & PipelineConfig::SetSamples ( VkSampleCountFlagBits samples )
    {
    m_data.Samples = samples;
    return *this;
    }

PipelineConfig & PipelineConfig::SetSampleShading ( bool enable, float minShading )
    {
    m_data.SampleShadingEnable = enable ? VK_TRUE : VK_FALSE;
    m_data.MinSampleShading = minShading;
    return *this;
    }

PipelineConfig & PipelineConfig::EnableDepthTest ( bool enable )
    {
    m_data.DepthTestEnable = enable ? VK_TRUE : VK_FALSE;
    return *this;
    }

PipelineConfig & PipelineConfig::SetDepthWrite ( bool enable )
    {
    m_data.DepthWriteEnable = enable ? VK_TRUE : VK_FALSE;
    return *this;
    }

PipelineConfig & PipelineConfig::SetDepthCompareOp ( VkCompareOp op )
    {
    m_data.DepthCompareOp = op;
    return *this;
    }

PipelineConfig & PipelineConfig::SetDepthBoundsTest ( bool enable )
    {
    m_data.DepthBoundsTestEnable = enable ? VK_TRUE : VK_FALSE;
    return *this;
    }

PipelineConfig & PipelineConfig::SetStencilTest ( bool enable )
    {
    m_data.StencilTestEnable = enable ? VK_TRUE : VK_FALSE;
    return *this;
    }

PipelineConfig & PipelineConfig::EnableBlending ( bool enable )
    {
    m_data.BlendEnable = enable ? VK_TRUE : VK_FALSE;
    return *this;
    }

PipelineConfig & PipelineConfig::SetBlendFactors ( VkBlendFactor srcColor, VkBlendFactor dstColor,
                                                   VkBlendOp colorOp,
                                                   VkBlendFactor srcAlpha, VkBlendFactor dstAlpha,
                                                   VkBlendOp alphaOp )
    {
    m_data.SrcColorBlendFactor = srcColor;
    m_data.DstColorBlendFactor = dstColor;
    m_data.ColorBlendOp = colorOp;
    m_data.SrcAlphaBlendFactor = srcAlpha;
    m_data.DstAlphaBlendFactor = dstAlpha;
    m_data.AlphaBlendOp = alphaOp;
    return *this;
    }

PipelineConfig & PipelineConfig::SetDynamicStates ( const std::vector<VkDynamicState> & states )
    {
    m_data.DynamicStates = states;
    return *this;
    }

PipelineConfig & PipelineConfig::AddDynamicState ( VkDynamicState state )
    {
    m_data.DynamicStates.push_back ( state );
    return *this;
    }

PipelineConfig & PipelineConfig::ClearDynamicStates ()
    {
    m_data.DynamicStates.clear ();
    return *this;
    }

PipelineConfig::ConfigData PipelineConfig::Build () const
    {
    return m_data;
    }