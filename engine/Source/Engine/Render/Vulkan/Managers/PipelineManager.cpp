#include "Render/Vulkan/Managers/PipelineManager.h"
#include "Render/Vulkan/Managers/WireframePipeline.h"
#include "Render/Vulkan/Managers/DescriptorManager.h"
#include "Core/EngineInfo.h"
#include "Render/Vulkan/Managers/DeviceManager.h"
#include "Render/Vulkan/Managers/RenderPassManager.h"
#include "Components/Collisions/TerrainComponent.h"
#include "Components/Meshes/BaseMeshComponent.h"
#include "Render/Vulkan/VertexStructs/AllVertices.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <shaderc/shaderc.hpp>

CPipelineManager::CPipelineManager ( FEngineInfo & Info )
    : IVulkanManager ( Info )
    {}

CPipelineManager::~CPipelineManager ()
    {
    Shutdown ();
    LogDebug ( GetManagerName (), " destroyed" );
    }

bool CPipelineManager::Initialize ()
    {
    LogDebug ( "Initializing PipelineManager..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr || !deviceMgr->IsInitialized ())
        {
        LogError ( "DeviceManager not initialized" );
        return false;
        }

    LogDebug ( "PipelineManager initialized successfully" );
    m_bInitialized = true;
    return true;
    }

void CPipelineManager::Shutdown ()
    {
    if (!m_bInitialized) return;

    LogDebug ( "Shutting down PipelineManager..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr)
        {
        LogDebug ( "  DeviceManager not available, clearing caches" );
        m_PipelineResources.clear ();
        m_PipelineLayouts.clear ();
        m_ShaderCache.clear ();
        m_bInitialized = false;
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
        m_bInitialized = false;
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

    // Destroy all shader modules
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

    m_bInitialized = false;
    LogDebug ( "PipelineManager shutdown complete" );
    }

const char * CPipelineManager::GetManagerName () const
    {
    return "PipelineManager";
    }

//=============================================================================
// Register Default Pipelines
//=============================================================================

bool CPipelineManager::RegisterDefaultPipelines ( VkRenderPass MainRenderPass )
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

        // НОВОЕ: Create wireframe pipeline для отладки коллизий
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

VkShaderModule CPipelineManager::CreateShaderModule ( const std::vector<char> & Code )
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
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

VkShaderModule CPipelineManager::LoadShader ( const std::string & Filename )
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
        // Можно оставить один лог при первой загрузке
        LogDebug ( "Cached shader: ", Filename, " (", fileSize, " bytes)" );
        }

    return module;
    }

FShaderModule CPipelineManager::LoadShaderModule ( const std::string & Filename, VkShaderStageFlagBits Stage )
    {
    FShaderModule result;
    result.Stage = Stage;
    result.EntryPoint = "main";

    // Определяем пути для исходного и скомпилированного файлов
    std::string sourceFilename = Filename;
    std::string spvFilename = Filename + ".spv";

    // Проверяем существование исходного файла
    std::ifstream sourceFile ( sourceFilename );
    bool sourceExists = sourceFile.good ();
    sourceFile.close ();

    // Проверяем существование .spv файла
    std::ifstream spvFile ( spvFilename );
    bool spvExists = spvFile.good ();
    spvFile.close ();

    LogDebug ( "Loading shader module: ", Filename );
    LogDebug ( "  Source exists: ", sourceExists ? "Yes" : "No" );
    LogDebug ( "  SPV exists: ", spvExists ? "Yes" : "No" );

   
    if (sourceExists)
        {
        LogDebug ( "  Source file found: ", sourceFilename );

        bool needCompile = true;

        // Если есть .spv файл, проверяем даты
        if (spvExists)
            {
            try
                {
                auto srcTime = std::filesystem::last_write_time ( sourceFilename );
                auto spvTime = std::filesystem::last_write_time ( spvFilename );

                if (spvTime >= srcTime)
                    {
                    needCompile = false;  // .spv новее или равен исходнику
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
                    // Если не можем проверить даты, компилируем
                    needCompile = true;
                    }
            }
        else
            {
            LogDebug ( "  No SPV file found, will compile" );
            }

            // Компилируем если нужно
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

    VkPipelineLayout CPipelineManager::CreatePipelineLayout (
        const std::string & LayoutName,
        const std::vector<VkDescriptorSetLayout> & DescSetLayouts,
        const std::vector<VkPushConstantRange> & PushConstants )
        {
            // Check if layout already exists
        auto it = m_PipelineLayouts.find ( LayoutName );
        if (it != m_PipelineLayouts.end ())
            {
            LogDebug ( "  Using existing pipeline layout: ", LayoutName );
            return it->second;
            }

        LogDebug ( "Creating new pipeline layout: ", LayoutName );

        auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
        VkDevice device = deviceMgr->GetDevice ();

        VkPipelineLayoutCreateInfo layoutInfo {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = static_cast< uint32_t >( DescSetLayouts.size () );
        layoutInfo.pSetLayouts = DescSetLayouts.data ();

        // Добавляем push constants, если они переданы
        if (!PushConstants.empty ())
            {
            layoutInfo.pushConstantRangeCount = static_cast< uint32_t >( PushConstants.size () );
            layoutInfo.pPushConstantRanges = PushConstants.data ();
            }
        else
            {
                // Если не переданы, создаём push constants по умолчанию
            static VkPushConstantRange defaultPushConstants[ 1 ] = {};
            defaultPushConstants[ 0 ].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            defaultPushConstants[ 0 ].offset = 0;
            defaultPushConstants[ 0 ].size = 3 * sizeof ( FMat4 ); // view, projection, model

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

            // Save to cache
        m_PipelineLayouts[ LayoutName ] = layout;
        LogDebug ( "  Pipeline layout created and cached: ", LayoutName, " (", ( void * ) layout, ")" );

        return layout;
        }

//=============================================================================
// Graphics Pipeline Creation
//=============================================================================

VkPipeline CPipelineManager::CreateGraphicsPipeline (
    const std::string & PipelineName,
    const std::vector<FShaderModule> & Shaders,
    VkPipelineLayout Layout,
    VkRenderPass RenderPass,
    const FGraphicsPipelineConfig & Config,
    uint32_t Subpass )
    {
    // Check if pipeline already exists
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

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    // Check Layout
    if (Layout == VK_NULL_HANDLE)
        {
        LogError ( "Pipeline layout is null" );
        return VK_NULL_HANDLE;
        }

    // Check Render pass
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

    // Check shader modules
    for (size_t i = 0; i < shaderStages.size (); i++)
        {
        if (shaderStages[ i ].module == VK_NULL_HANDLE)
            {
            LogError ( "  Shader stage ", i, " has null module" );
            return VK_NULL_HANDLE;
            }
        }

    // Fixed function states
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = GetVertexInputState ( Config.VertexInput );
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = GetInputAssemblyState ( Config );
    VkPipelineRasterizationStateCreateInfo rasterizer = GetRasterizationState ( Config );
    VkPipelineMultisampleStateCreateInfo multisampling = GetMultisampleState ( Config );
    VkPipelineDepthStencilStateCreateInfo depthStencil = GetDepthStencilState ( Config );
    VkPipelineColorBlendAttachmentState colorBlendAttachment = GetColorBlendAttachment ( Config );
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

    // Check all pointers
    if (!pipelineInfo.pStages || !pipelineInfo.pVertexInputState || !pipelineInfo.pInputAssemblyState ||
         !pipelineInfo.pViewportState || !pipelineInfo.pRasterizationState || !pipelineInfo.pMultisampleState ||
         !pipelineInfo.pDepthStencilState || !pipelineInfo.pColorBlendState || !pipelineInfo.pDynamicState)
        {
        LogError ( "  One or more pipeline state pointers are null!" );
        return VK_NULL_HANDLE;
        }

    if (pipelineInfo.pStages == nullptr)
        {
        LogError ( "  FATAL: pStages is NULL!" );
        return VK_NULL_HANDLE;
        } 

    for (uint32_t i = 0; i < pipelineInfo.stageCount; i++)
        {
        if (pipelineInfo.pStages[ i ].module == VK_NULL_HANDLE)
            {
            LogError ( "  FATAL: Shader module ", i, " is VK_NULL_HANDLE!" );
            return VK_NULL_HANDLE;
            }
        if (pipelineInfo.pStages[ i ].pName == nullptr)
            {
            LogError ( "  FATAL: Shader entry point ", i, " is NULL!" );
            return VK_NULL_HANDLE;
            }
        }
     
    if (pipelineInfo.pVertexInputState == nullptr)
        {
        LogError ( "  FATAL: pVertexInputState is NULL!" );
        return VK_NULL_HANDLE;
        }
     
    if (pipelineInfo.pVertexInputState->vertexBindingDescriptionCount > 0 &&
         pipelineInfo.pVertexInputState->pVertexBindingDescriptions == nullptr)
        {
        LogError ( "  FATAL: pVertexBindingDescriptions is NULL but count > 0!" );
        return VK_NULL_HANDLE;
        }

    if (pipelineInfo.pVertexInputState->vertexAttributeDescriptionCount > 0 &&
         pipelineInfo.pVertexInputState->pVertexAttributeDescriptions == nullptr)
        {
        LogError ( "  FATAL: pVertexAttributeDescriptions is NULL but count > 0!" );
        return VK_NULL_HANDLE;
        }
     
    if (pipelineInfo.pInputAssemblyState == nullptr)
        {
        LogError ( "  FATAL: pInputAssemblyState is NULL!" );
        return VK_NULL_HANDLE;
        }

    if (pipelineInfo.pViewportState == nullptr)
        {
        LogError ( "  FATAL: pViewportState is NULL!" );
        return VK_NULL_HANDLE;
        }

    if (pipelineInfo.pRasterizationState == nullptr)
        {
        LogError ( "  FATAL: pRasterizationState is NULL!" );
        return VK_NULL_HANDLE;
        }

    if (pipelineInfo.pMultisampleState == nullptr)
        {
        LogError ( "  FATAL: pMultisampleState is NULL!" );
        return VK_NULL_HANDLE;
        }

    if (pipelineInfo.pDepthStencilState == nullptr)
        {
        LogError ( "  FATAL: pDepthStencilState is NULL!" );
        return VK_NULL_HANDLE;
        }

    if (pipelineInfo.pColorBlendState == nullptr)
        {
        LogError ( "  FATAL: pColorBlendState is NULL!" );
        return VK_NULL_HANDLE;
        }
     
    if (pipelineInfo.pColorBlendState->attachmentCount > 0 &&
         pipelineInfo.pColorBlendState->pAttachments == nullptr)
        {
        LogError ( "  FATAL: pAttachments is NULL but attachmentCount > 0!" );
        return VK_NULL_HANDLE;
        }

    if (pipelineInfo.pDynamicState == nullptr)
        {
        LogError ( "  FATAL: pDynamicState is NULL!" );
        return VK_NULL_HANDLE;
        }
     
    if (pipelineInfo.pDynamicState->dynamicStateCount > 0 &&
         pipelineInfo.pDynamicState->pDynamicStates == nullptr)
        {
        LogError ( "  FATAL: pDynamicStates is NULL but dynamicStateCount > 0!" );
        return VK_NULL_HANDLE;
        }
     
    if (pipelineInfo.layout == VK_NULL_HANDLE)
        {
        LogError ( "  FATAL: layout is VK_NULL_HANDLE!" );
        return VK_NULL_HANDLE;
        }

    if (pipelineInfo.renderPass == VK_NULL_HANDLE)
        {
        LogError ( "  FATAL: renderPass is VK_NULL_HANDLE!" );
        return VK_NULL_HANDLE;
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

VkPipeline CPipelineManager::GetPipeline ( const std::string & Name ) const
    {
    auto it = m_PipelineResources.find ( Name );
    if (it != m_PipelineResources.end () && it->second.IsValid ())
        {
        return it->second.Pipeline;
        }
    LogError ( "Pipeline not found: ", Name );
    return VK_NULL_HANDLE;
    }

VkPipelineLayout CPipelineManager::GetPipelineLayout ( const std::string & Name ) const
    {
    auto it = m_PipelineLayouts.find ( Name );
    if (it != m_PipelineLayouts.end ())
        {
        return it->second;
        }
    LogError ( "Pipeline layout not found: ", Name );
    return VK_NULL_HANDLE;
    }

FPipelineResource CPipelineManager::GetPipelineResource ( const std::string & Name ) const
    {
    auto it = m_PipelineResources.find ( Name );
    if (it != m_PipelineResources.end ())
        {
        return it->second;
        }
    return FPipelineResource ();
    }

bool CPipelineManager::HasPipeline ( const std::string & Name ) const
    {
    auto it = m_PipelineResources.find ( Name );
    return ( it != m_PipelineResources.end () && it->second.IsValid () );
    }

bool CPipelineManager::HasPipelineLayout ( const std::string & Name ) const
    {
    return ( m_PipelineLayouts.find ( Name ) != m_PipelineLayouts.end () );
    }

//=============================================================================
// Destroy Specific Resources
//=============================================================================

bool CPipelineManager::DestroyPipeline ( const std::string & Name )
    {
    auto it = m_PipelineResources.find ( Name );
    if (it == m_PipelineResources.end ())
        {
        LogError ( "Cannot destroy pipeline - not found: ", Name );
        return false;
        }

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
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

bool CPipelineManager::DestroyPipelineLayout ( const std::string & Name )
    {
    auto it = m_PipelineLayouts.find ( Name );
    if (it == m_PipelineLayouts.end ())
        {
        LogError ( "Cannot destroy layout - not found: ", Name );
        return false;
        }

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr) return false;

    VkDevice device = deviceMgr->GetDevice ();
    if (device == VK_NULL_HANDLE) return false;

    LogDebug ( "Destroying pipeline layout: ", Name );

    vkDestroyPipelineLayout ( device, it->second, nullptr );
    m_PipelineLayouts.erase ( it );
    return true;
    }

void CPipelineManager::DestroyShaderModule ( VkShaderModule Module )
    {
    if (Module == VK_NULL_HANDLE) return;

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkDestroyShaderModule ( device, Module, nullptr );
    }

void CPipelineManager::DestroyPipelineLayout ( VkPipelineLayout Layout )
    {
    if (Layout == VK_NULL_HANDLE) return;

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkDestroyPipelineLayout ( device, Layout, nullptr );
    }

void CPipelineManager::DestroyPipeline ( VkPipeline Pipeline )
    {
    if (Pipeline == VK_NULL_HANDLE) return;

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkDestroyPipeline ( device, Pipeline, nullptr );
    }

//=============================================================================
// Triangle Pipeline
//=============================================================================

VkPipeline CPipelineManager::CreateTrianglePipeline ( VkRenderPass RenderPass )
    {   
    if (HasPipeline ( "TrianglePipeline" ))
        {         
        return GetPipeline ( "TrianglePipeline" );
        }
     
    VkPipelineLayout layout = CreatePipelineLayout ( "TriangleLayout" );
    if (layout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create triangle pipeline layout" );
        return VK_NULL_HANDLE;
        }
 
    FShaderModule vertShader = LoadShaderModule ( "Assets/Shaders/Triangle.vert", VK_SHADER_STAGE_VERTEX_BIT );
    FShaderModule fragShader = LoadShaderModule ( "Assets/Shaders/Triangle.frag", VK_SHADER_STAGE_FRAGMENT_BIT );
 

    if (vertShader.Module == VK_NULL_HANDLE || fragShader.Module == VK_NULL_HANDLE)
        {
        LogError ( "  Shader modules are invalid!" );
        return VK_NULL_HANDLE;
        }

    if (!vertShader.IsValid () || !fragShader.IsValid ())
        {
        LogError ( "Failed to load shaders for triangle pipeline" );
        return VK_NULL_HANDLE;
        }

   
    FGraphicsPipelineConfig config;
    config.VertexInput = GetTriangleVertexInput ();
    config.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config.PolygonMode = VK_POLYGON_MODE_FILL;
    config.CullMode = VK_CULL_MODE_BACK_BIT;
    config.FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    config.DepthTestEnable = VK_TRUE;
    config.DepthWriteEnable = VK_TRUE;
    config.DepthCompareOp = VK_COMPARE_OP_LESS;
    config.BlendEnable = VK_FALSE;
    config.DynamicStates = GetDefaultDynamicStates ();

    std::vector<FShaderModule> shaders = { vertShader, fragShader };
 
    return CreateGraphicsPipeline ( "TrianglePipeline", shaders, layout, RenderPass, config );
    }

//=============================================================================
// Vertex Input Helpers
//=============================================================================


VkPipeline CPipelineManager::CreateMeshPipeLine ( VkRenderPass RenderPass )
    {
    std::string PipelineName = "StaticMesh";
    if (HasPipeline ( PipelineName ))
        {
        return GetPipeline ( PipelineName );
        }

        // Получаем дескрипторные layout'ы от DescriptorManager
    VkDescriptorSetLayout globalLayout = m_DescMgr->GetGlobalLayout ();
    VkDescriptorSetLayout perObjectLayout = m_DescMgr->GetPerObjectLayout ();

    if (globalLayout == VK_NULL_HANDLE || perObjectLayout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to get descriptor set layouts" );
        return VK_NULL_HANDLE;
        }

    std::vector<VkDescriptorSetLayout> descriptorLayouts = { globalLayout, perObjectLayout };

    // Push constants только для model matrix (так как view/projection в дескрипторах)
    std::vector<VkPushConstantRange> pushConstants ( 1 );
    pushConstants[ 0 ].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstants[ 0 ].offset = 0;
    pushConstants[ 0 ].size = sizeof ( glm::mat4x4 ); // Только model matrix

    // Create pipeline layout with descriptor sets AND push constants
    VkPipelineLayout layout = CreatePipelineLayout (
        PipelineName + "Layout",
        descriptorLayouts,  // Теперь передаём дескрипторные layout'ы
        pushConstants );     // И push constants

    if (layout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create StaticMesh pipeline layout" );
        return VK_NULL_HANDLE;
        }

        // Load shaders
    FShaderModule vertShader = LoadShaderModule ( "Assets/Shaders/Mesh.vert", VK_SHADER_STAGE_VERTEX_BIT );
    FShaderModule fragShader = LoadShaderModule ( "Assets/Shaders/Mesh.frag", VK_SHADER_STAGE_FRAGMENT_BIT );

    if (vertShader.Module == VK_NULL_HANDLE || fragShader.Module == VK_NULL_HANDLE)
        {
        LogError ( "Shader modules are invalid!" );
        return VK_NULL_HANDLE;
        }

    FVertexInputDescription vertexInput;
    vertexInput.Bindings.push_back ( FMeshVertex::GetBindingDescription () );
    vertexInput.Attributes = FMeshVertex::GetAttributeDescriptions ();

    FGraphicsPipelineConfig config;
    config.VertexInput = vertexInput;
    config.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config.PolygonMode = VK_POLYGON_MODE_FILL;
    config.CullMode = VK_CULL_MODE_NONE;
    config.FrontFace = VK_FRONT_FACE_CLOCKWISE;
    config.DepthTestEnable = VK_TRUE;
    config.DepthWriteEnable = VK_TRUE;
    config.DepthCompareOp = VK_COMPARE_OP_LESS;
    config.BlendEnable = VK_TRUE;
    config.DynamicStates = GetDefaultDynamicStates ();

    std::vector<FShaderModule> shaders = { vertShader, fragShader };

    return CreateGraphicsPipeline ( PipelineName, shaders, layout, RenderPass, config );
    }



VkPipeline CPipelineManager::CreateTerrainPipeline ( VkRenderPass RenderPass )
    {
    if (HasPipeline ( "TerrainPipeline" ))
        {
        return GetPipeline ( "TerrainPipeline" );
        }

        // Получаем глобальный layout дескрипторов
    VkDescriptorSetLayout globalLayout = m_DescMgr->GetGlobalLayout ();
    if (globalLayout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to get global descriptor set layout" );
        return VK_NULL_HANDLE;
        }

    std::vector<VkDescriptorSetLayout> descriptorLayouts = { globalLayout };

    // Push constants только для model matrix и параметров террейна
    std::vector<VkPushConstantRange> pushConstants ( 1 );
    pushConstants[ 0 ].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstants[ 0 ].offset = 0;
    pushConstants[ 0 ].size = sizeof ( glm::mat4x4 ) + sizeof ( glm::vec4 ); // model + terrainParams

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

    FGraphicsPipelineConfig config;
    config.VertexInput = terrainVertexInput;
    config.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config.PolygonMode = VK_POLYGON_MODE_FILL;
    config.CullMode = VK_CULL_MODE_BACK_BIT;
    config.FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    config.DepthTestEnable = VK_TRUE;
    config.DepthWriteEnable = VK_TRUE;
    config.DepthCompareOp = VK_COMPARE_OP_LESS;
    config.BlendEnable = VK_FALSE;
    config.DynamicStates = GetDefaultDynamicStates ();

    std::vector<FShaderModule> shaders = { vertShader, fragShader };

    return CreateGraphicsPipeline ( "TerrainPipeline", shaders, layout, RenderPass, config );
    }



VkPipeline CPipelineManager::CreateWireframePipeline ( VkRenderPass RenderPass )
    {
    std::string PipelineName = "WireframePipeline";
    if (HasPipeline ( PipelineName ))
        {
        return GetPipeline ( PipelineName );
        }

        // Получаем глобальный layout дескрипторов
    VkDescriptorSetLayout globalLayout = m_DescMgr->GetGlobalLayout ();
    if (globalLayout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to get global descriptor set layout" );
        return VK_NULL_HANDLE;
        }

    std::vector<VkDescriptorSetLayout> descriptorLayouts = { globalLayout };

    // Push constants только для model matrix
    std::vector<VkPushConstantRange> pushConstants ( 1 );
    pushConstants[ 0 ].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstants[ 0 ].offset = 0;
    pushConstants[ 0 ].size = sizeof ( glm::mat4x4 ); // Только model

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

    FGraphicsPipelineConfig config;
    config.VertexInput = vertexInput;
    config.Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    config.PolygonMode = VK_POLYGON_MODE_FILL;
    config.CullMode = VK_CULL_MODE_NONE;
    config.FrontFace = VK_FRONT_FACE_CLOCKWISE;
    config.DepthTestEnable = VK_TRUE;
    config.DepthWriteEnable = VK_FALSE;
    config.DepthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    config.BlendEnable = VK_FALSE;
    config.LineWidth = 2.0f;
    config.DynamicStates = GetDefaultDynamicStates ();

    std::vector<FShaderModule> shaders = { vertShader, fragShader };

    return CreateGraphicsPipeline ( PipelineName, shaders, layout, RenderPass, config );
    }

FVertexInputDescription CPipelineManager::GetTriangleVertexInput () const
    {
    FVertexInputDescription desc;

    // Binding description
    VkVertexInputBindingDescription binding {};
    binding.binding = 0;
    binding.stride = 6 * sizeof ( float );  // position (3) + color (3)
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    desc.Bindings.push_back ( binding );

    // Position attribute (location 0)
    VkVertexInputAttributeDescription positionAttr {};
    positionAttr.binding = 0;
    positionAttr.location = 0;
    positionAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttr.offset = 0;
    desc.Attributes.push_back ( positionAttr );

    // Color attribute (location 1)
    VkVertexInputAttributeDescription colorAttr {};
    colorAttr.binding = 0;
    colorAttr.location = 1;
    colorAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
    colorAttr.offset = 3 * sizeof ( float );
    desc.Attributes.push_back ( colorAttr );

    return desc;
    }

FVertexInputDescription CPipelineManager::GetUIVertexInput () const
    {
    FVertexInputDescription desc;

    // For UI elements (2D position + UV)
    VkVertexInputBindingDescription binding {};
    binding.binding = 0;
    binding.stride = 4 * sizeof ( float );  // position (2) + uv (2)
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    desc.Bindings.push_back ( binding );

    VkVertexInputAttributeDescription positionAttr {};
    positionAttr.binding = 0;
    positionAttr.location = 0;
    positionAttr.format = VK_FORMAT_R32G32_SFLOAT;
    positionAttr.offset = 0;
    desc.Attributes.push_back ( positionAttr );

    VkVertexInputAttributeDescription uvAttr {};
    uvAttr.binding = 0;
    uvAttr.location = 1;
    uvAttr.format = VK_FORMAT_R32G32_SFLOAT;
    uvAttr.offset = 2 * sizeof ( float );
    desc.Attributes.push_back ( uvAttr );

    return desc;
    }

FVertexInputDescription CPipelineManager::GetSkyboxVertexInput () const
    {
    FVertexInputDescription desc;

    // Skybox uses only position (3D)
    VkVertexInputBindingDescription binding {};
    binding.binding = 0;
    binding.stride = 3 * sizeof ( float );
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    desc.Bindings.push_back ( binding );

    VkVertexInputAttributeDescription positionAttr {};
    positionAttr.binding = 0;
    positionAttr.location = 0;
    positionAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttr.offset = 0;
    desc.Attributes.push_back ( positionAttr );

    return desc;
    }

//=============================================================================
// State Creation Helpers
//=============================================================================

std::vector<VkDynamicState> CPipelineManager::GetDefaultDynamicStates () const
    {
    return { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    }

VkPipelineShaderStageCreateInfo CPipelineManager::GetShaderStageInfo ( const FShaderModule & Module ) const
    {
    VkPipelineShaderStageCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.stage = Module.Stage;
    info.module = Module.Module;
    info.pName = Module.EntryPoint.c_str ();
    return info;
    }

VkPipelineVertexInputStateCreateInfo CPipelineManager::GetVertexInputState ( const FVertexInputDescription & Desc ) const
    {
    VkPipelineVertexInputStateCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.vertexBindingDescriptionCount = static_cast< uint32_t >( Desc.Bindings.size () );
    info.pVertexBindingDescriptions = Desc.Bindings.data ();
    info.vertexAttributeDescriptionCount = static_cast< uint32_t >( Desc.Attributes.size () );
    info.pVertexAttributeDescriptions = Desc.Attributes.data ();
    return info;
    }

VkPipelineInputAssemblyStateCreateInfo CPipelineManager::GetInputAssemblyState ( const FGraphicsPipelineConfig & Config ) const
    {
    VkPipelineInputAssemblyStateCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.topology = Config.Topology;
    info.primitiveRestartEnable = Config.PrimitiveRestartEnable;
    return info;
    }

VkPipelineRasterizationStateCreateInfo CPipelineManager::GetRasterizationState ( const FGraphicsPipelineConfig & Config ) const
    {
    VkPipelineRasterizationStateCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.depthClampEnable = VK_FALSE;
    info.rasterizerDiscardEnable = VK_FALSE;
    info.polygonMode = Config.PolygonMode;
    info.lineWidth = Config.LineWidth;
    info.cullMode = Config.CullMode;
    info.frontFace = Config.FrontFace;
    info.depthBiasEnable = Config.DepthBiasEnable;
    info.depthBiasConstantFactor = Config.DepthBiasConstantFactor;
    info.depthBiasClamp = Config.DepthBiasClamp;
    info.depthBiasSlopeFactor = Config.DepthBiasSlopeFactor;
    return info;
    }

VkPipelineMultisampleStateCreateInfo CPipelineManager::GetMultisampleState ( const FGraphicsPipelineConfig & Config ) const
    {
    VkPipelineMultisampleStateCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pNext = NULL;
    info.sampleShadingEnable = Config.SampleShadingEnable;
    info.rasterizationSamples = Config.Samples;
    info.minSampleShading = Config.MinSampleShading;
    info.pSampleMask = nullptr;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;
    return info;
    }

VkPipelineDepthStencilStateCreateInfo CPipelineManager::GetDepthStencilState ( const FGraphicsPipelineConfig & Config ) const
    {
    VkPipelineDepthStencilStateCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.depthTestEnable = Config.DepthTestEnable;
    info.depthWriteEnable = Config.DepthWriteEnable;
    info.depthCompareOp = Config.DepthCompareOp;
    info.depthBoundsTestEnable = Config.DepthBoundsTestEnable;
    info.stencilTestEnable = Config.StencilTestEnable;
 
    VkStencilOpState stencilOp {};
    stencilOp.failOp = VK_STENCIL_OP_KEEP;
    stencilOp.passOp = VK_STENCIL_OP_KEEP;
    stencilOp.depthFailOp = VK_STENCIL_OP_KEEP;
    stencilOp.compareOp = VK_COMPARE_OP_ALWAYS;
    stencilOp.compareMask = 0;
    stencilOp.writeMask = 0;
    stencilOp.reference = 0;

    info.front = stencilOp;
    info.back = stencilOp;

    info.minDepthBounds = 0.0f;
    info.maxDepthBounds = 1.0f;
    return info;
    }

VkPipelineColorBlendAttachmentState CPipelineManager::GetColorBlendAttachment ( const FGraphicsPipelineConfig & Config ) const
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

VkPipelineColorBlendStateCreateInfo CPipelineManager::GetColorBlendState ( const FGraphicsPipelineConfig & Config ) const
    {
    VkPipelineColorBlendAttachmentState attachment = GetColorBlendAttachment ( Config );

    VkPipelineColorBlendStateCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.logicOpEnable = VK_FALSE;
    info.logicOp = VK_LOGIC_OP_COPY;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
     
    info.blendConstants[ 0 ] = 0.0f;
    info.blendConstants[ 1 ] = 0.0f;
    info.blendConstants[ 2 ] = 0.0f;
    info.blendConstants[ 3 ] = 0.0f;

    return info;
    }

VkPipelineDynamicStateCreateInfo CPipelineManager::GetDynamicState ( const FGraphicsPipelineConfig & Config ) const
    {
    VkPipelineDynamicStateCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    info.pNext = nullptr;

    // ВАЖНО: создаём локальный массив, который будет жить достаточно долго
    static VkDynamicState defaultStates [] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    if (Config.DynamicStates.empty ())
        {
        info.dynamicStateCount = 2;
        info.pDynamicStates = defaultStates;
        }
    else
        {
        info.dynamicStateCount = static_cast< uint32_t >( Config.DynamicStates.size () );
        info.pDynamicStates = Config.DynamicStates.data ();
        }

    return info;
    }