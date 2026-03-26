#include "KE/Vulkan/Managers/PipelineManager.h"
#include "KE/Vulkan/Managers/DeviceManager.h"
#include "KE/Vulkan/Managers/DescriptorManager.h"
#include "KE/Vulkan/VKinfo.h"
#include "KE/Vulkan/VertexStructs/AllVertices.h"
#include "KE/Vulkan/Managers/RenderPassManager.h"
#include "KE/Vulkan/Managers/PipelineConfig.h"
#include <glm/glm.hpp>

//=============================================================================
// FGraphicsPipelineConfig Constructor
//=============================================================================

FGraphicsPipelineConfig::FGraphicsPipelineConfig ()
    : VertexInput ()  // Инициализация пустым объектом
    , Topology ( VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST )
    , PrimitiveRestartEnable ( VK_FALSE )
    , PolygonMode ( VK_POLYGON_MODE_FILL )
    , LineWidth ( 1.0f )
    , CullMode ( VK_CULL_MODE_BACK_BIT )
    , FrontFace ( VK_FRONT_FACE_CLOCKWISE )
    , DepthBiasEnable ( VK_FALSE )
    , DepthBiasConstantFactor ( 0.0f )
    , DepthBiasClamp ( 0.0f )
    , DepthBiasSlopeFactor ( 0.0f )
    , Samples ( VK_SAMPLE_COUNT_1_BIT )
    , SampleShadingEnable ( VK_FALSE )
    , MinSampleShading ( 1.0f )
    , DepthTestEnable ( VK_TRUE )
    , DepthWriteEnable ( VK_TRUE )
    , DepthCompareOp ( VK_COMPARE_OP_LESS )
    , DepthBoundsTestEnable ( VK_FALSE )
    , StencilTestEnable ( VK_FALSE )
    , BlendEnable ( VK_FALSE )
    , SrcColorBlendFactor ( VK_BLEND_FACTOR_ONE )
    , DstColorBlendFactor ( VK_BLEND_FACTOR_ZERO )      // Важно: ZERO, а не ONE_MINUS_SRC_ALPHA
    , ColorBlendOp ( VK_BLEND_OP_ADD )                   // Важно: ADD
    , SrcAlphaBlendFactor ( VK_BLEND_FACTOR_ONE )
    , DstAlphaBlendFactor ( VK_BLEND_FACTOR_ZERO )
    , AlphaBlendOp ( VK_BLEND_OP_ADD )
    , DynamicStates ()  // Пустой, будет заполнен позже
    {}

    //=============================================================================
    // PipelineManager Implementation
    //=============================================================================

bool PipelineManager::Init ()
    {
    LogDebug ( "Initializing PipelineManager..." );

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (!deviceMgr || !deviceMgr->IsInitialized ())
        {
        LogError ( "DeviceManager not initialized" );
        return false;
        }

    auto RenderPassMgr = static_cast< RenderPassManager * >( m_info->Managers.RenderPassManager.get () );
    if (!RenderPassMgr->IsInitialized ())
        {
        LogError ( "RenderPass Manager not initialized" );
        return false;
        }

    auto MainRenderPass = RenderPassMgr->GetMainRenderPass ();
    if (!RegisterDefaultPipelines ( MainRenderPass ))
        {
        LogError ( "Fail to register default pipelines" );
        return false;
        }

    LogDebug ( " initialized successfully" );
    bIsInitialized = true;
    return true;
    }

const std::string & PipelineManager::GetManagerName () const
    {
    static const std::string name = "Pipeline Manager";
    return name;
    }

PipelineManager::PipelineManager ()
    : IVKManager ()
    {
    LogDebug ( "Created" );
    }

PipelineManager::~PipelineManager ()
    {
    Shutdown ();
    LogDebug ( " destroyed" );
    }

void PipelineManager::Shutdown ()
    {
    LogDebug ( "Shutting down PipelineManager..." );

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (!deviceMgr)
        {
        LogDebug ( "  DeviceManager not available, clearing caches" );
        m_PipelineResources.clear ();
        m_PipelineLayouts.clear ();
        m_ShaderCache.clear ();
        bIsInitialized = false;
        LogDebug ( "PipelineManager shutdown complete (skipped)" );
        return;
        }

    VkDevice device = deviceMgr->GetDevice ();
    if (device == VK_NULL_HANDLE)
        {
        LogDebug ( "  Device is null, clearing caches" );
        m_PipelineResources.clear ();
        m_PipelineLayouts.clear ();
        m_ShaderCache.clear ();
        bIsInitialized = false;
        LogDebug ( "PipelineManager shutdown complete (skipped)" );
        return;
        }

        // Destroy all pipelines
    LogDebug ( "  Destroying all pipelines: ", m_PipelineResources.size () );
    for (auto & [name, resource] : m_PipelineResources)
        {
        if (resource.Pipeline != VK_NULL_HANDLE)
            {
            LogDebug ( "    Destroying pipeline: ", name.c_str () );
            vkDestroyPipeline ( device, resource.Pipeline, nullptr );
            }
        }
    m_PipelineResources.clear ();

    // Destroy all pipeline layouts
    LogDebug ( "  Destroying all pipeline layouts: ", m_PipelineLayouts.size () );
    for (auto & [name, layout] : m_PipelineLayouts)
        {
        if (layout != VK_NULL_HANDLE)
            {
            LogDebug ( "    Destroying layout: ", name.c_str () );
            vkDestroyPipelineLayout ( device, layout, nullptr );
            }
        }
    m_PipelineLayouts.clear ();

    LogDebug ( "  Destroying shader modules, count: ", m_ShaderCache.size () );
    for (auto it = m_ShaderCache.begin (); it != m_ShaderCache.end (); ++it)
        {
        if (it->second != VK_NULL_HANDLE)
            {
            LogDebug ( "    Destroying shader: ", it->first.c_str () );
            vkDestroyShaderModule ( device, it->second, nullptr );
            }
        }
    m_ShaderCache.clear ();

    bIsInitialized = false;
    LogDebug ( "PipelineManager shutdown complete" );
    }

    //=============================================================================
    // Register Default Pipelines
    //=============================================================================

bool PipelineManager::RegisterDefaultPipelines ( VkRenderPass MainRenderPass )
    {
    LogDebug ( "Registering default pipelines..." );

    // Create triangle pipeline
    if (CreateTrianglePipeline ( MainRenderPass ) == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create triangle pipeline" );
        return false;
        }

        // Create mesh pipeline
    if (CreateMeshPipeLine ( MainRenderPass ) == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create Mesh pipeline" );
        return false;
        }

        // Create terrain pipeline
    if (CreateTerrainPipeline ( MainRenderPass ) == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create Terrain pipeline" );
        return false;
        }

        // Create wireframe pipeline
    if (CreateWireframePipeline ( MainRenderPass ) == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create Wireframe pipeline" );
        return false;
        }

    LogDebug ( "Default pipelines registered successfully" );
    return true;
    }

    //=============================================================================
    // Shader Loading
    //=============================================================================

VkShaderModule PipelineManager::CreateShaderModule ( const std::vector<char> & Code )
    {
    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkShaderModuleCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = Code.size ();
    createInfo.pCode = reinterpret_cast< const uint32_t * >( Code.data () );

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule ( device, &createInfo, nullptr, &shaderModule );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create shader module: ", static_cast< int >( result ) );
        return VK_NULL_HANDLE;
        }

    return shaderModule;
    }

VkShaderModule PipelineManager::LoadShader ( const std::string & Filename )
    {
    auto it = m_ShaderCache.find ( Filename );
    if (it != m_ShaderCache.end ())
        {
        return it->second;
        }

    std::ifstream file ( Filename, std::ios::ate | std::ios::binary );
    if (!file.is_open ())
        {
        LogError ( "Failed to open shader file: ", Filename );
        return VK_NULL_HANDLE;
        }

    size_t fileSize = static_cast< size_t >( file.tellg () );
    std::vector<char> buffer ( fileSize );

    file.seekg ( 0 );
    file.read ( buffer.data (), fileSize );
    file.close ();

    VkShaderModule module = CreateShaderModule ( buffer );
    if (module != VK_NULL_HANDLE)
        {
        m_ShaderCache[ Filename ] = module;
        LogDebug ( "Cached shader: ", Filename, " (", fileSize, " bytes)" );
        }

    return module;
    }

FShaderModule PipelineManager::LoadShaderModule ( const std::string & Filename, VkShaderStageFlagBits Stage )
    {
    FShaderModule result;
    result.Stage = Stage;
    result.EntryPoint = "main";

    std::string sourceFilename = Filename;
    std::string spvFilename = Filename + ".spv";

    std::ifstream sourceFile ( sourceFilename );
    bool sourceExists = sourceFile.good ();
    sourceFile.close ();

    std::ifstream spvFile ( spvFilename );
    bool spvExists = spvFile.good ();
    spvFile.close ();

    LogDebug ( "Loading shader module: ", Filename );
    LogDebug ( "  Source exists: ", sourceExists ? "Yes" : "No" );
    LogDebug ( "  SPV exists: ", spvExists ? "Yes" : "No" );
    LogDebug ( "Current working directory: ", std::filesystem::current_path ().string () );
    LogDebug ( "Full shader path: ", std::filesystem::absolute ( sourceFilename ).string () );

    if (sourceExists)
        {
        LogDebug ( "  Source file found: ", sourceFilename );

        bool needCompile = true;

        if (spvExists)
            {
            try
                {
                auto srcTime = std::filesystem::last_write_time ( sourceFilename );
                auto spvTime = std::filesystem::last_write_time ( spvFilename );

                if (spvTime >= srcTime)
                    {
                    needCompile = false;
                    LogDebug ( "  SPV is up to date (",
                               std::filesystem::file_size ( spvFilename ), " bytes)" );
                    }
                else
                    {
                    LogDebug ( "  SPV is older than source, recompiling..." );
                    }
                }
                catch (const std::exception & e)
                    {
                    LogDebug ( "  Could not check file times: ", e.what () );
                    needCompile = true;
                    }
            }
        else
            {
            LogDebug ( "  No SPV file found, will compile" );
            }

        if (needCompile)
            {
            LogDebug ( "  Compiling shader: ", sourceFilename );

            std::string command = "glslc \"" + sourceFilename + "\" -o \"" + spvFilename + "\"";
            int compileResult = system ( command.c_str () );

            if (compileResult != 0)
                {
                LogError ( "Failed to compile shader: ", sourceFilename );
                return result;
                }

            LogDebug ( "  Shader compiled successfully: ", spvFilename,
                       " (", std::filesystem::file_size ( spvFilename ), " bytes)" );
            }
        }
    else if (spvExists)
        {
        LogDebug ( "  No source file, but SPV exists - using cached SPV" );
        }
    else
        {
        LogError ( "  No source or SPV file found for shader: ", Filename );
        return result;
        }

    result.Module = LoadShader ( spvFilename );

    if (result.Module == VK_NULL_HANDLE)
        {
        LogError ( "Failed to load compiled shader: ", spvFilename );
        }
    else
        {
        LogDebug ( "  Shader module loaded successfully: ", ( void * ) result.Module );
        }

    return result;
    }

    //=============================================================================
    // Pipeline Layout Creation
    //=============================================================================

VkPipelineLayout PipelineManager::CreatePipelineLayout (
    const std::string & LayoutName,
    const std::vector<VkDescriptorSetLayout> & DescSetLayouts,
    const std::vector<VkPushConstantRange> & PushConstants )
    {
    auto it = m_PipelineLayouts.find ( LayoutName );
    if (it != m_PipelineLayouts.end ())
        {
        LogDebug ( "  Using existing pipeline layout: ", LayoutName );
        return it->second;
        }

    LogDebug ( "Creating new pipeline layout: ", LayoutName );

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkPipelineLayoutCreateInfo layoutInfo {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = static_cast< uint32_t >( DescSetLayouts.size () );
    layoutInfo.pSetLayouts = DescSetLayouts.data ();

    if (!PushConstants.empty ())
        {
        layoutInfo.pushConstantRangeCount = static_cast< uint32_t >( PushConstants.size () );
        layoutInfo.pPushConstantRanges = PushConstants.data ();
        }
    else
        {
        static VkPushConstantRange defaultPushConstants[ 1 ] = {};
        defaultPushConstants[ 0 ].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        defaultPushConstants[ 0 ].offset = 0;
        defaultPushConstants[ 0 ].size = 3 * sizeof ( glm::mat4x4 );

        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = defaultPushConstants;
        }

    VkPipelineLayout layout;
    VkResult result = vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &layout );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create pipeline layout: ", static_cast< int >( result ) );
        return VK_NULL_HANDLE;
        }

    m_PipelineLayouts[ LayoutName ] = layout;
    LogDebug ( "  Pipeline layout created and cached: ", LayoutName, " (", ( void * ) layout, ")" );

    return layout;
    }

    //=============================================================================
    // Graphics Pipeline Creation (with PipelineConfig)
    //=============================================================================

VkPipeline PipelineManager::CreateGraphicsPipeline (
    const std::string & PipelineName,
    const std::vector<FShaderModule> & Shaders,
    VkPipelineLayout Layout,
    VkRenderPass RenderPass,
    const PipelineConfig::ConfigData & Config,
    uint32_t Subpass )
    {
    FGraphicsPipelineConfig oldConfig;

    // Копируем ВСЕ поля
    oldConfig.VertexInput = Config.VertexInput;
    oldConfig.Topology = Config.Topology;
    oldConfig.PrimitiveRestartEnable = Config.PrimitiveRestartEnable;
    oldConfig.PolygonMode = Config.PolygonMode;
    oldConfig.LineWidth = Config.LineWidth;
    oldConfig.CullMode = Config.CullMode;
    oldConfig.FrontFace = Config.FrontFace;
    oldConfig.DepthBiasEnable = Config.DepthBiasEnable;
    oldConfig.DepthBiasConstantFactor = Config.DepthBiasConstantFactor;
    oldConfig.DepthBiasClamp = Config.DepthBiasClamp;
    oldConfig.DepthBiasSlopeFactor = Config.DepthBiasSlopeFactor;
    oldConfig.Samples = Config.Samples;
    oldConfig.SampleShadingEnable = Config.SampleShadingEnable;
    oldConfig.MinSampleShading = Config.MinSampleShading;
    oldConfig.DepthTestEnable = Config.DepthTestEnable;
    oldConfig.DepthWriteEnable = Config.DepthWriteEnable;
    oldConfig.DepthCompareOp = Config.DepthCompareOp;
    oldConfig.DepthBoundsTestEnable = Config.DepthBoundsTestEnable;
    oldConfig.StencilTestEnable = Config.StencilTestEnable;
    oldConfig.BlendEnable = Config.BlendEnable;
    oldConfig.SrcColorBlendFactor = Config.SrcColorBlendFactor;
    oldConfig.DstColorBlendFactor = Config.DstColorBlendFactor;
    oldConfig.ColorBlendOp = Config.ColorBlendOp;
    oldConfig.SrcAlphaBlendFactor = Config.SrcAlphaBlendFactor;
    oldConfig.DstAlphaBlendFactor = Config.DstAlphaBlendFactor;
    oldConfig.AlphaBlendOp = Config.AlphaBlendOp;
    oldConfig.DynamicStates = Config.DynamicStates;

    // Отладочный вывод
    LogDebug ( "  Pipeline config - BlendEnable: ", oldConfig.BlendEnable );
    LogDebug ( "    SrcColorBlendFactor: ", static_cast< int >( oldConfig.SrcColorBlendFactor ) );
    LogDebug ( "    DstColorBlendFactor: ", static_cast< int >( oldConfig.DstColorBlendFactor ) );
    LogDebug ( "    ColorBlendOp: ", static_cast< int >( oldConfig.ColorBlendOp ) );

    return CreateGraphicsPipeline ( PipelineName, Shaders, Layout, RenderPass, oldConfig, Subpass );
    }

    //=============================================================================
    // Graphics Pipeline Creation (with FGraphicsPipelineConfig)
    //=============================================================================

VkPipeline PipelineManager::CreateGraphicsPipeline (
    const std::string & PipelineName,
    const std::vector<FShaderModule> & Shaders,
    VkPipelineLayout Layout,
    VkRenderPass RenderPass,
    const FGraphicsPipelineConfig & Config,
    uint32_t Subpass )
    {
    auto it = m_PipelineResources.find ( PipelineName );
    if (it != m_PipelineResources.end () && it->second.IsValid ())
        {
        LogDebug ( "  Using existing pipeline: ", PipelineName );
        return it->second.Pipeline;
        }

    LogDebug ( "Creating new graphics pipeline: ", PipelineName );

    if (Shaders.empty ())
        {
        LogError ( "No shaders provided for graphics pipeline" );
        return VK_NULL_HANDLE;
        }

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    if (Layout == VK_NULL_HANDLE)
        {
        LogError ( "Pipeline layout is null" );
        return VK_NULL_HANDLE;
        }

    if (RenderPass == VK_NULL_HANDLE)
        {
        LogError ( "Render pass is null" );
        return VK_NULL_HANDLE;
        }

        // Shader stages
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for (const auto & shader : Shaders)
        {
        if (!shader.IsValid ())
            {
            LogError ( "Invalid shader module" );
            return VK_NULL_HANDLE;
            }
        shaderStages.push_back ( GetShaderStageInfo ( shader ) );
        }

        // Fixed function states
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = GetVertexInputState ( Config.VertexInput );
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = GetInputAssemblyState ( Config );
    VkPipelineRasterizationStateCreateInfo rasterizer = GetRasterizationState ( Config );
    VkPipelineMultisampleStateCreateInfo multisampling = GetMultisampleState ( Config );
    VkPipelineDepthStencilStateCreateInfo depthStencil = GetDepthStencilState ( Config );
    VkPipelineColorBlendStateCreateInfo colorBlending = GetColorBlendState ( Config );
    VkPipelineDynamicStateCreateInfo dynamicState = GetDynamicState ( Config );

    // Viewport state (dynamic)
    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast< uint32_t >( shaderStages.size () );
    pipelineInfo.pStages = shaderStages.data ();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = Layout;
    pipelineInfo.renderPass = RenderPass;
    pipelineInfo.subpass = Subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    // Validation checks
    if (Config.BlendEnable)
        {
        if (Config.DstColorBlendFactor < VK_BLEND_FACTOR_ZERO ||
             Config.DstColorBlendFactor > VK_BLEND_FACTOR_CONSTANT_ALPHA)
            {
            LogError ( "Invalid DstColorBlendFactor: ", static_cast< int >( Config.DstColorBlendFactor ) );
            return VK_NULL_HANDLE;
            }

        if (Config.ColorBlendOp < VK_BLEND_OP_ADD || Config.ColorBlendOp > VK_BLEND_OP_MIN)
            {
            LogError ( "Invalid ColorBlendOp: ", static_cast< int >( Config.ColorBlendOp ) );
            return VK_NULL_HANDLE;
            }
        }

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkResult result = vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline );

    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create graphics pipeline: ", static_cast< int >( result ) );
        switch (result)
            {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    LogError ( "  Out of host memory" );
                    break;
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    LogError ( "  Out of device memory" );
                    break;
                case VK_ERROR_INVALID_SHADER_NV:
                    LogError ( "  Invalid shader - check shader compilation and entry points" );
                    break;
                default:
                    LogError ( "  Unknown error code" );
                    break;
            }
        return VK_NULL_HANDLE;
        }

        // Save to cache
    FPipelineResource resource;
    resource.Pipeline = pipeline;
    resource.Layout = Layout;
    resource.Name = PipelineName;
    resource.RenderPass = RenderPass;
    resource.Config = Config;
    resource.bIsValid = true;

    m_PipelineResources[ PipelineName ] = resource;

    LogDebug ( "  Pipeline created and cached: ", PipelineName, " (", ( void * ) pipeline, ")" );
    return pipeline;
    }

    //=============================================================================
    // Get Pipeline Methods
    //=============================================================================

VkPipeline PipelineManager::GetPipeline ( const std::string & Name ) const
    {
    auto it = m_PipelineResources.find ( Name );
    if (it != m_PipelineResources.end () && it->second.IsValid ())
        {
        return it->second.Pipeline;
        }
    LogError ( "Pipeline not found: ", Name );
    return VK_NULL_HANDLE;
    }

VkPipelineLayout PipelineManager::GetPipelineLayout ( const std::string & Name ) const
    {
    auto it = m_PipelineLayouts.find ( Name );
    if (it != m_PipelineLayouts.end ())
        {
        return it->second;
        }
    LogError ( "Pipeline layout not found: ", Name );
    return VK_NULL_HANDLE;
    }

FPipelineResource PipelineManager::GetPipelineResource ( const std::string & Name ) const
    {
    auto it = m_PipelineResources.find ( Name );
    if (it != m_PipelineResources.end ())
        {
        return it->second;
        }
    return FPipelineResource ();
    }

bool PipelineManager::HasPipeline ( const std::string & Name ) const
    {
    auto it = m_PipelineResources.find ( Name );
    return ( it != m_PipelineResources.end () && it->second.IsValid () );
    }

bool PipelineManager::HasPipelineLayout ( const std::string & Name ) const
    {
    return ( m_PipelineLayouts.find ( Name ) != m_PipelineLayouts.end () );
    }

    //=============================================================================
    // Destroy Specific Resources
    //=============================================================================

bool PipelineManager::DestroyPipeline ( const std::string & Name )
    {
    auto it = m_PipelineResources.find ( Name );
    if (it == m_PipelineResources.end ())
        {
        LogError ( "Cannot destroy pipeline - not found: ", Name );
        return false;
        }

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (!deviceMgr) return false;

    VkDevice device = deviceMgr->GetDevice ();
    if (device == VK_NULL_HANDLE) return false;

    LogDebug ( "Destroying pipeline: ", Name );

    if (it->second.Pipeline != VK_NULL_HANDLE)
        {
        vkDestroyPipeline ( device, it->second.Pipeline, nullptr );
        }

    m_PipelineResources.erase ( it );
    return true;
    }

bool PipelineManager::DestroyPipelineLayout ( const std::string & Name )
    {
    auto it = m_PipelineLayouts.find ( Name );
    if (it == m_PipelineLayouts.end ())
        {
        LogError ( "Cannot destroy layout - not found: ", Name );
        return false;
        }

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (!deviceMgr) return false;

    VkDevice device = deviceMgr->GetDevice ();
    if (device == VK_NULL_HANDLE) return false;

    LogDebug ( "Destroying pipeline layout: ", Name );

    vkDestroyPipelineLayout ( device, it->second, nullptr );
    m_PipelineLayouts.erase ( it );
    return true;
    }

void PipelineManager::DestroyShaderModule ( VkShaderModule Module )
    {
    if (Module == VK_NULL_HANDLE) return;

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkDestroyShaderModule ( device, Module, nullptr );
    }

void PipelineManager::DestroyPipelineLayout ( VkPipelineLayout Layout )
    {
    if (Layout == VK_NULL_HANDLE) return;

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkDestroyPipelineLayout ( device, Layout, nullptr );
    }

void PipelineManager::DestroyPipeline ( VkPipeline Pipeline )
    {
    if (Pipeline == VK_NULL_HANDLE) return;

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkDestroyPipeline ( device, Pipeline, nullptr );
    }

    //=============================================================================
    // Triangle Pipeline
    //=============================================================================

VkPipeline PipelineManager::CreateTrianglePipeline ( VkRenderPass RenderPass )
    {
    if (HasPipeline ( "TrianglePipeline" ))
        {
        return GetPipeline ( "TrianglePipeline" );
        }

    std::vector<VkDescriptorSetLayout> emptyLayouts;
    std::vector<VkPushConstantRange> emptyPushConstants;

    VkPipelineLayout layout = CreatePipelineLayout ( "TriangleLayout", emptyLayouts, emptyPushConstants );
    if (layout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create triangle pipeline layout" );
        return VK_NULL_HANDLE;
        }

    FShaderModule vertShader = LoadShaderModule ( "Assets/Shaders/Triangle.vert", VK_SHADER_STAGE_VERTEX_BIT );
    FShaderModule fragShader = LoadShaderModule ( "Assets/Shaders/Triangle.frag", VK_SHADER_STAGE_FRAGMENT_BIT );

    if (!vertShader.IsValid () || !fragShader.IsValid ())
        {
        LogError ( "Failed to load shaders for triangle pipeline" );
        return VK_NULL_HANDLE;
        }

    PipelineConfig config;
    config.SetVertexInput ( GetTriangleVertexInput () )
        .SetTopology ( VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST )
        .SetPolygonMode ( VK_POLYGON_MODE_FILL )
        .SetCullMode ( VK_CULL_MODE_NONE )
        .SetFrontFace ( VK_FRONT_FACE_COUNTER_CLOCKWISE )
        .EnableDepthTest ( false )
        .SetDepthWrite ( false )
        .SetDepthCompareOp ( VK_COMPARE_OP_LESS )
        .EnableBlending ( false )
        .SetBlendFactors ( VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO );

    std::vector<FShaderModule> shaders = { vertShader, fragShader };

    return CreateGraphicsPipeline ( "TrianglePipeline", shaders, layout, RenderPass, config.Build () );
    }

    //=============================================================================
    // Mesh Pipeline
    //=============================================================================

VkPipeline PipelineManager::CreateMeshPipeLine ( VkRenderPass RenderPass )
    {
    std::string PipelineName = "StaticMesh";
    if (HasPipeline ( PipelineName ))
        {
        return GetPipeline ( PipelineName );
        }

    VkDescriptorSetLayout globalLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout perObjectLayout = VK_NULL_HANDLE;
    if (DescriptorManager * m_DescMgr = dynamic_cast< DescriptorManager * >( m_info->Managers.DescriptorManager.get () ))
        {
        globalLayout = m_DescMgr->GetGlobalLayout ();
        perObjectLayout = m_DescMgr->GetPerObjectLayout ();
        }

    if (globalLayout == VK_NULL_HANDLE || perObjectLayout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to get descriptor set layouts" );
        return VK_NULL_HANDLE;
        }

    std::vector<VkDescriptorSetLayout> descriptorLayouts = { globalLayout, perObjectLayout };

    std::vector<VkPushConstantRange> pushConstants ( 1 );
    pushConstants[ 0 ].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstants[ 0 ].offset = 0;
    pushConstants[ 0 ].size = sizeof ( glm::mat4x4 );

    VkPipelineLayout layout = CreatePipelineLayout ( PipelineName + "Layout", descriptorLayouts, pushConstants );
    if (layout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create StaticMesh pipeline layout" );
        return VK_NULL_HANDLE;
        }

    FShaderModule vertShader = LoadShaderModule ( "Assets/Shaders/Mesh.vert", VK_SHADER_STAGE_VERTEX_BIT );
    FShaderModule fragShader = LoadShaderModule ( "Assets/Shaders/Mesh.frag", VK_SHADER_STAGE_FRAGMENT_BIT );

    if (!vertShader.IsValid () || !fragShader.IsValid ())
        {
        LogError ( "Failed to load mesh shaders" );
        return VK_NULL_HANDLE;
        }

    FVertexInputDescription vertexInput;
    vertexInput.Bindings.push_back ( FMeshVertex::GetBindingDescription () );
    vertexInput.Attributes = FMeshVertex::GetAttributeDescriptions ();

    PipelineConfig config;
    config.SetVertexInput ( vertexInput )
        .SetTopology ( VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST )
        .SetPolygonMode ( VK_POLYGON_MODE_FILL )
        .SetCullMode ( VK_CULL_MODE_BACK_BIT )
        .SetFrontFace ( VK_FRONT_FACE_CLOCKWISE )
        .EnableDepthTest ( true )
        .SetDepthWrite ( true )
        .SetDepthCompareOp ( VK_COMPARE_OP_LESS )
        .EnableBlending ( true )
        .SetBlendFactors ( VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO );

    std::vector<FShaderModule> shaders = { vertShader, fragShader };

    return CreateGraphicsPipeline ( PipelineName, shaders, layout, RenderPass, config.Build () );
    }

    //=============================================================================
    // Terrain Pipeline
    //=============================================================================

VkPipeline PipelineManager::CreateTerrainPipeline ( VkRenderPass RenderPass )
    {
    if (HasPipeline ( "TerrainPipeline" ))
        {
        return GetPipeline ( "TerrainPipeline" );
        }

    VkDescriptorSetLayout globalLayout = VK_NULL_HANDLE;
    if (DescriptorManager * m_DescMgr = dynamic_cast< DescriptorManager * >( m_info->Managers.DescriptorManager.get () ))
        {
        globalLayout = m_DescMgr->GetGlobalLayout ();
        }

    if (globalLayout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to get global descriptor set layout" );
        return VK_NULL_HANDLE;
        }

    std::vector<VkDescriptorSetLayout> descriptorLayouts = { globalLayout };

    std::vector<VkPushConstantRange> pushConstants ( 1 );
    pushConstants[ 0 ].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstants[ 0 ].offset = 0;
    pushConstants[ 0 ].size = sizeof ( glm::mat4x4 ) + sizeof ( glm::vec4 );

    VkPipelineLayout layout = CreatePipelineLayout ( "TerrainLayout", descriptorLayouts, pushConstants );
    if (layout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create Terrain pipeline layout" );
        return VK_NULL_HANDLE;
        }

    FVertexInputDescription terrainVertexInput;
    VkVertexInputBindingDescription binding = FTerrainVertex::GetBindingDescription ();
    terrainVertexInput.Bindings.push_back ( binding );
    terrainVertexInput.Attributes = FTerrainVertex::GetAttributeDescriptions ();

    FShaderModule vertShader = LoadShaderModule ( "Assets/Shaders/Terrain.vert", VK_SHADER_STAGE_VERTEX_BIT );
    FShaderModule fragShader = LoadShaderModule ( "Assets/Shaders/Terrain.frag", VK_SHADER_STAGE_FRAGMENT_BIT );

    if (!vertShader.IsValid () || !fragShader.IsValid ())
        {
        LogError ( "Failed to load terrain shaders" );
        return VK_NULL_HANDLE;
        }

    PipelineConfig config;
    config.SetVertexInput ( terrainVertexInput )
        .SetTopology ( VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST )
        .SetPolygonMode ( VK_POLYGON_MODE_FILL )
        .SetCullMode ( VK_CULL_MODE_BACK_BIT )
        .SetFrontFace ( VK_FRONT_FACE_COUNTER_CLOCKWISE )
        .EnableDepthTest ( true )
        .SetDepthWrite ( true )
        .SetDepthCompareOp ( VK_COMPARE_OP_LESS )
        .EnableBlending ( false )
        .SetBlendFactors ( VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO );

    std::vector<FShaderModule> shaders = { vertShader, fragShader };

    return CreateGraphicsPipeline ( "TerrainPipeline", shaders, layout, RenderPass, config.Build () );
    }

    //=============================================================================
    // Wireframe Pipeline
    //=============================================================================

VkPipeline PipelineManager::CreateWireframePipeline ( VkRenderPass RenderPass )
    {
    std::string PipelineName = "WireframePipeline";
    if (HasPipeline ( PipelineName ))
        {
        return GetPipeline ( PipelineName );
        }

    VkDescriptorSetLayout globalLayout = VK_NULL_HANDLE;
    if (DescriptorManager * m_DescMgr = dynamic_cast< DescriptorManager * >( m_info->Managers.DescriptorManager.get () ))
        {
        globalLayout = m_DescMgr->GetGlobalLayout ();
        }

    if (globalLayout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to get global descriptor set layout" );
        return VK_NULL_HANDLE;
        }

    std::vector<VkDescriptorSetLayout> descriptorLayouts = { globalLayout };

    std::vector<VkPushConstantRange> pushConstants ( 1 );
    pushConstants[ 0 ].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstants[ 0 ].offset = 0;
    pushConstants[ 0 ].size = sizeof ( glm::mat4x4 );

    VkPipelineLayout layout = CreatePipelineLayout ( PipelineName + "Layout", descriptorLayouts, pushConstants );
    if (layout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create Wireframe pipeline layout" );
        return VK_NULL_HANDLE;
        }

    FShaderModule vertShader = LoadShaderModule ( "Assets/Shaders/Wireframe.vert", VK_SHADER_STAGE_VERTEX_BIT );
    FShaderModule fragShader = LoadShaderModule ( "Assets/Shaders/Wireframe.frag", VK_SHADER_STAGE_FRAGMENT_BIT );

    if (!vertShader.IsValid () || !fragShader.IsValid ())
        {
        LogError ( "Failed to load wireframe shaders" );
        return VK_NULL_HANDLE;
        }

    FVertexInputDescription vertexInput;
    vertexInput.Bindings.push_back ( FWireframeVertex::GetBindingDescription () );
    vertexInput.Attributes = FWireframeVertex::GetAttributeDescriptions ();

    PipelineConfig config;
    config.SetVertexInput ( vertexInput )
        .SetTopology ( VK_PRIMITIVE_TOPOLOGY_LINE_LIST )
        .SetPolygonMode ( VK_POLYGON_MODE_FILL )
        .SetCullMode ( VK_CULL_MODE_NONE )
        .SetFrontFace ( VK_FRONT_FACE_CLOCKWISE )
        .EnableDepthTest ( true )
        .SetDepthWrite ( false )
        .SetDepthCompareOp ( VK_COMPARE_OP_LESS_OR_EQUAL )
        .EnableBlending ( false )
        .SetBlendFactors ( VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO )
        .SetLineWidth ( 2.0f );

    std::vector<FShaderModule> shaders = { vertShader, fragShader };

    return CreateGraphicsPipeline ( PipelineName, shaders, layout, RenderPass, config.Build () );
    }

    //=============================================================================
    // Vertex Input Helpers
    //=============================================================================

FVertexInputDescription PipelineManager::GetTriangleVertexInput () const
    {
    FVertexInputDescription desc;

    VkVertexInputBindingDescription binding {};
    binding.binding = 0;
    binding.stride = 6 * sizeof ( float );
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    desc.Bindings.push_back ( binding );

    VkVertexInputAttributeDescription positionAttr {};
    positionAttr.binding = 0;
    positionAttr.location = 0;
    positionAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttr.offset = 0;
    desc.Attributes.push_back ( positionAttr );

    VkVertexInputAttributeDescription colorAttr {};
    colorAttr.binding = 0;
    colorAttr.location = 1;
    colorAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
    colorAttr.offset = 3 * sizeof ( float );
    desc.Attributes.push_back ( colorAttr );

    return desc;
    }




    //=============================================================================
    // State Creation Helpers
    //=============================================================================

std::vector<VkDynamicState> PipelineManager::GetDefaultDynamicStates () const
    {
    return { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    }

VkPipelineShaderStageCreateInfo PipelineManager::GetShaderStageInfo ( const FShaderModule & Module ) const
    {
    VkPipelineShaderStageCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.stage = Module.Stage;
    info.module = Module.Module;
    info.pName = Module.EntryPoint.c_str ();
    return info;
    }

VkPipelineVertexInputStateCreateInfo PipelineManager::GetVertexInputState ( const FVertexInputDescription & Desc ) const
    {
        // ДОЛЖНО БЫТЬ:
    m_cachedVertexInputState = {};
    m_cachedVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_cachedVertexInputState.vertexBindingDescriptionCount = static_cast< uint32_t >( Desc.Bindings.size () );
    m_cachedVertexInputState.pVertexBindingDescriptions = Desc.Bindings.data ();
    m_cachedVertexInputState.vertexAttributeDescriptionCount = static_cast< uint32_t >( Desc.Attributes.size () );
    m_cachedVertexInputState.pVertexAttributeDescriptions = Desc.Attributes.data ();
    return m_cachedVertexInputState;
    }

VkPipelineInputAssemblyStateCreateInfo PipelineManager::GetInputAssemblyState ( const FGraphicsPipelineConfig & Config ) const
    {
        // ДОЛЖНО БЫТЬ:
    m_cachedInputAssemblyState = {};
    m_cachedInputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_cachedInputAssemblyState.topology = Config.Topology;
    m_cachedInputAssemblyState.primitiveRestartEnable = Config.PrimitiveRestartEnable;
    return m_cachedInputAssemblyState;
    }

VkPipelineRasterizationStateCreateInfo PipelineManager::GetRasterizationState ( const FGraphicsPipelineConfig & Config ) const
    {
        // ДОЛЖНО БЫТЬ:
    m_cachedRasterizationState = {};
    m_cachedRasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    m_cachedRasterizationState.depthClampEnable = VK_FALSE;
    m_cachedRasterizationState.rasterizerDiscardEnable = VK_FALSE;
    m_cachedRasterizationState.polygonMode = Config.PolygonMode;
    m_cachedRasterizationState.lineWidth = Config.LineWidth;
    m_cachedRasterizationState.cullMode = Config.CullMode;
    m_cachedRasterizationState.frontFace = Config.FrontFace;
    m_cachedRasterizationState.depthBiasEnable = Config.DepthBiasEnable;
    m_cachedRasterizationState.depthBiasConstantFactor = Config.DepthBiasConstantFactor;
    m_cachedRasterizationState.depthBiasClamp = Config.DepthBiasClamp;
    m_cachedRasterizationState.depthBiasSlopeFactor = Config.DepthBiasSlopeFactor;
    return m_cachedRasterizationState;
    }

VkPipelineMultisampleStateCreateInfo PipelineManager::GetMultisampleState ( const FGraphicsPipelineConfig & Config ) const
    {
        // ДОЛЖНО БЫТЬ:
    m_cachedMultisampleState = {};
    m_cachedMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    m_cachedMultisampleState.pNext = NULL;
    m_cachedMultisampleState.sampleShadingEnable = Config.SampleShadingEnable;
    m_cachedMultisampleState.rasterizationSamples = Config.Samples;
    m_cachedMultisampleState.minSampleShading = Config.MinSampleShading;
    m_cachedMultisampleState.pSampleMask = nullptr;
    m_cachedMultisampleState.alphaToCoverageEnable = VK_FALSE;
    m_cachedMultisampleState.alphaToOneEnable = VK_FALSE;
    return m_cachedMultisampleState;
    }

VkPipelineDepthStencilStateCreateInfo PipelineManager::GetDepthStencilState ( const FGraphicsPipelineConfig & Config ) const
    {
        // ДОЛЖНО БЫТЬ:
    m_cachedDepthStencilState = {};
    m_cachedDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_cachedDepthStencilState.pNext = nullptr;
    m_cachedDepthStencilState.depthTestEnable = Config.DepthTestEnable;
    m_cachedDepthStencilState.depthWriteEnable = Config.DepthWriteEnable;
    m_cachedDepthStencilState.depthCompareOp = Config.DepthCompareOp;
    m_cachedDepthStencilState.depthBoundsTestEnable = Config.DepthBoundsTestEnable;
    m_cachedDepthStencilState.stencilTestEnable = Config.StencilTestEnable;

    VkStencilOpState stencilOp {};
    stencilOp.failOp = VK_STENCIL_OP_KEEP;
    stencilOp.passOp = VK_STENCIL_OP_KEEP;
    stencilOp.depthFailOp = VK_STENCIL_OP_KEEP;
    stencilOp.compareOp = VK_COMPARE_OP_ALWAYS;
    stencilOp.compareMask = 0;
    stencilOp.writeMask = 0;
    stencilOp.reference = 0;

    m_cachedDepthStencilState.front = stencilOp;
    m_cachedDepthStencilState.back = stencilOp;

    m_cachedDepthStencilState.minDepthBounds = 0.0f;
    m_cachedDepthStencilState.maxDepthBounds = 1.0f;
    return m_cachedDepthStencilState;
    }

VkPipelineColorBlendAttachmentState PipelineManager::GetColorBlendAttachment ( const FGraphicsPipelineConfig & Config ) const
    {
    VkPipelineColorBlendAttachmentState attachment {};
    attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    attachment.blendEnable = Config.BlendEnable;
    attachment.srcColorBlendFactor = Config.SrcColorBlendFactor;
    attachment.dstColorBlendFactor = Config.DstColorBlendFactor;
    attachment.colorBlendOp = Config.ColorBlendOp;
    attachment.srcAlphaBlendFactor = Config.SrcAlphaBlendFactor;
    attachment.dstAlphaBlendFactor = Config.DstAlphaBlendFactor;
    attachment.alphaBlendOp = Config.AlphaBlendOp;
    return attachment;
    }

VkPipelineColorBlendStateCreateInfo PipelineManager::GetColorBlendState ( const FGraphicsPipelineConfig & Config ) const
    {
        // Используем кэшированную структуру, которая будет жить достаточно долго
    m_cachedAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    if (!Config.BlendEnable)
        {
        m_cachedAttachment.blendEnable = VK_FALSE;
        m_cachedAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        m_cachedAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        m_cachedAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        m_cachedAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        m_cachedAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        m_cachedAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        }
    else
        {
        m_cachedAttachment.blendEnable = Config.BlendEnable;
        m_cachedAttachment.srcColorBlendFactor = Config.SrcColorBlendFactor;
        m_cachedAttachment.dstColorBlendFactor = Config.DstColorBlendFactor;
        m_cachedAttachment.colorBlendOp = Config.ColorBlendOp;
        m_cachedAttachment.srcAlphaBlendFactor = Config.SrcAlphaBlendFactor;
        m_cachedAttachment.dstAlphaBlendFactor = Config.DstAlphaBlendFactor;
        m_cachedAttachment.alphaBlendOp = Config.AlphaBlendOp;
        }

    m_cachedColorBlendState = {};  // Важно: обнуляем всю структуру
    m_cachedColorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_cachedColorBlendState.pNext = nullptr;
    m_cachedColorBlendState.flags = 0;  // Явно устанавливаем flags = 0
    m_cachedColorBlendState.logicOpEnable = VK_FALSE;
    m_cachedColorBlendState.logicOp = VK_LOGIC_OP_COPY;
    m_cachedColorBlendState.attachmentCount = 1;
    m_cachedColorBlendState.pAttachments = &m_cachedAttachment;
    m_cachedColorBlendState.blendConstants[ 0 ] = 0.0f;
    m_cachedColorBlendState.blendConstants[ 1 ] = 0.0f;
    m_cachedColorBlendState.blendConstants[ 2 ] = 0.0f;
    m_cachedColorBlendState.blendConstants[ 3 ] = 0.0f;

    return m_cachedColorBlendState;
    }



VkPipelineDynamicStateCreateInfo PipelineManager::GetDynamicState ( const FGraphicsPipelineConfig & Config ) const
    {
        // ДОЛЖНО БЫТЬ:
    m_cachedDynamicState = {};
    m_cachedDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    m_cachedDynamicState.pNext = nullptr;

    static VkDynamicState defaultStates [] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    if (Config.DynamicStates.empty ())
        {
        m_cachedDynamicState.dynamicStateCount = 2;
        m_cachedDynamicState.pDynamicStates = defaultStates;
        }
    else
        {
        m_cachedDynamicState.dynamicStateCount = static_cast< uint32_t >( Config.DynamicStates.size () );
        m_cachedDynamicState.pDynamicStates = Config.DynamicStates.data ();
        }

    return m_cachedDynamicState;
    }