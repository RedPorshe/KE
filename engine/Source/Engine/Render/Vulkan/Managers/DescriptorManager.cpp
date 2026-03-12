#include "Render/Vulkan/Managers/DescriptorManager.h"
#include "Core/EngineInfo.h"
#include "Render/Vulkan/Managers/DeviceManager.h"

CDescriptorManager::CDescriptorManager ( FEngineInfo & Info )
    : IVulkanManager ( Info )
    {}

CDescriptorManager::~CDescriptorManager ()
    {
    Shutdown ();
    LogDebug ( GetManagerName (), " destroyed" );
    }

bool CDescriptorManager::Initialize ()
    {
    LogDebug ( "Initializing DescriptorManager..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr || !deviceMgr->IsInitialized ())
        {
        LogError ( "DeviceManager not initialized" );
        return false;
        }

        // Создаём стандартные layout'ы
    if (!CreateDefaultLayouts ())
        {
        LogError ( "Failed to create default descriptor set layouts" );
        return false;
        }

        // Создаём стандартные пулы
    if (!CreateDefaultPools ())
        {
        LogError ( "Failed to create default descriptor pools" );
        return false;
        }

    LogDebug ( "DescriptorManager initialized successfully" );
    m_bInitialized = true;
    return true;
    }

void CDescriptorManager::Shutdown ()
    {
    if (!m_bInitialized) return;

    LogDebug ( "Shutting down DescriptorManager..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr) return;

    VkDevice device = deviceMgr->GetDevice ();
    if (device == VK_NULL_HANDLE) return;

    // Уничтожаем все пулы
    for (auto & [name, pool] : m_PoolCache)
        {
        if (pool != VK_NULL_HANDLE)
            {
            LogDebug ( "  Destroying pool: ", name.c_str () );
            vkDestroyDescriptorPool ( device, pool, nullptr );
            }
        }
    m_PoolCache.clear ();
    m_PoolInfoCache.clear ();

    // Уничтожаем все layout'ы
    for (auto & [name, layout] : m_LayoutCache)
        {
        if (layout != VK_NULL_HANDLE)
            {
            LogDebug ( "  Destroying layout: ", name.c_str () );
            vkDestroyDescriptorSetLayout ( device, layout, nullptr );
            }
        }
    m_LayoutCache.clear ();

    m_GlobalLayout = VK_NULL_HANDLE;
    m_PerObjectLayout = VK_NULL_HANDLE;
    m_TextureLayout = VK_NULL_HANDLE;
    m_GlobalPool = VK_NULL_HANDLE;
    m_PerFramePool = VK_NULL_HANDLE;

    m_bInitialized = false;
    LogDebug ( "DescriptorManager shutdown complete" );
    }

const char * CDescriptorManager::GetManagerName () const
    {
    return "DescriptorManager";
    }

    //=============================================================================
    // Создание layout дескрипторов
    //=============================================================================

VkDescriptorSetLayout CDescriptorManager::CreateDescriptorSetLayout (
    const std::string & LayoutName,
    const std::vector<VkDescriptorSetLayoutBinding> & Bindings )
    {
        // Проверяем, есть ли уже такой layout
    auto it = m_LayoutCache.find ( LayoutName );
    if (it != m_LayoutCache.end ())
        {
        LogDebug ( "  Using existing descriptor set layout: ", LayoutName );
        return it->second;
        }

    LogDebug ( "Creating descriptor set layout: ", LayoutName, " with ", Bindings.size (), " bindings" );

    VkDescriptorSetLayout layout = CreateDescriptorSetLayoutInternal ( Bindings );
    if (layout != VK_NULL_HANDLE)
        {
        m_LayoutCache[ LayoutName ] = layout;
        LogDebug ( "  Layout created and cached: ", ( void * ) layout );
        }

    return layout;
    }

VkDescriptorSetLayout CDescriptorManager::CreateDescriptorSetLayout (
    const std::string & LayoutName,
    const FDescriptorSetLayoutInfo & Info )
    {
    return CreateDescriptorSetLayout ( LayoutName, Info.Bindings );
    }

VkDescriptorSetLayout CDescriptorManager::GetDescriptorSetLayout ( const std::string & LayoutName ) const
    {
    auto it = m_LayoutCache.find ( LayoutName );
    if (it != m_LayoutCache.end ())
        {
        return it->second;
        }
    LogError ( "Descriptor set layout not found: ", LayoutName );
    return VK_NULL_HANDLE;
    }

bool CDescriptorManager::HasDescriptorSetLayout ( const std::string & LayoutName ) const
    {
    return m_LayoutCache.find ( LayoutName ) != m_LayoutCache.end ();
    }

    //=============================================================================
    // Создание пулов дескрипторов
    //=============================================================================

VkDescriptorPool CDescriptorManager::CreateDescriptorPool (
    const std::string & PoolName,
    uint32_t MaxSets,
    const std::vector<VkDescriptorPoolSize> & PoolSizes,
    VkDescriptorPoolCreateFlags Flags )
    {
        // Проверяем, есть ли уже такой пул
    auto it = m_PoolCache.find ( PoolName );
    if (it != m_PoolCache.end ())
        {
        LogDebug ( "  Using existing descriptor pool: ", PoolName );
        return it->second;
        }

    LogDebug ( "Creating descriptor pool: ", PoolName, " with max sets: ", MaxSets );

    VkDescriptorPool pool = CreateDescriptorPoolInternal ( MaxSets, PoolSizes, Flags );
    if (pool != VK_NULL_HANDLE)
        {
        m_PoolCache[ PoolName ] = pool;

        FDescriptorPoolInfo poolInfo;
        poolInfo.Pool = pool;
        poolInfo.Name = PoolName;
        poolInfo.MaxSets = MaxSets;
        m_PoolInfoCache[ PoolName ] = poolInfo;

        LogDebug ( "  Pool created and cached: ", ( void * ) pool );
        }

    return pool;
    }

VkDescriptorPool CDescriptorManager::GetDescriptorPool ( const std::string & PoolName ) const
    {
    auto it = m_PoolCache.find ( PoolName );
    if (it != m_PoolCache.end ())
        {
        return it->second;
        }
    LogError ( "Descriptor pool not found: ", PoolName );
    return VK_NULL_HANDLE;
    }

bool CDescriptorManager::HasDescriptorPool ( const std::string & PoolName ) const
    {
    return m_PoolCache.find ( PoolName ) != m_PoolCache.end ();
    }

    //=============================================================================
    // Выделение и освобождение наборов дескрипторов
    //=============================================================================

std::vector<VkDescriptorSet> CDescriptorManager::AllocateDescriptorSets (
    const std::string & PoolName,
    const std::vector<VkDescriptorSetLayout> & Layouts )
    {
    std::vector<VkDescriptorSet> descriptorSets;

    auto poolIt = m_PoolCache.find ( PoolName );
    if (poolIt == m_PoolCache.end ())
        {
        LogError ( "Cannot allocate descriptor sets - pool not found: ", PoolName );
        return descriptorSets;
        }

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = poolIt->second;
    allocInfo.descriptorSetCount = static_cast< uint32_t >( Layouts.size () );
    allocInfo.pSetLayouts = Layouts.data ();

    descriptorSets.resize ( Layouts.size () );
    VkResult result = vkAllocateDescriptorSets ( device, &allocInfo, descriptorSets.data () );

    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to allocate descriptor sets: ", static_cast< int >( result ) );
        descriptorSets.clear ();
        return descriptorSets;
        }

        // Обновляем информацию о пуле
    auto infoIt = m_PoolInfoCache.find ( PoolName );
    if (infoIt != m_PoolInfoCache.end ())
        {
        for (auto set : descriptorSets)
            {
            infoIt->second.DescriptorSets.push_back ( set );
            }
        }

    LogDebug ( "Allocated ", Layouts.size (), " descriptor sets from pool: ", PoolName );
    return descriptorSets;
    }

VkDescriptorSet CDescriptorManager::AllocateDescriptorSet (
    const std::string & PoolName,
    VkDescriptorSetLayout Layout )
    {
    auto sets = AllocateDescriptorSets ( PoolName, { Layout } );
    if (sets.empty ())
        {
        return VK_NULL_HANDLE;
        }
    return sets[ 0 ];
    }

void CDescriptorManager::FreeDescriptorSet ( const std::string & PoolName, VkDescriptorSet DescriptorSet )
    {
    FreeDescriptorSets ( PoolName, { DescriptorSet } );
    }

void CDescriptorManager::FreeDescriptorSets ( const std::string & PoolName, const std::vector<VkDescriptorSet> & DescriptorSets )
    {
    if (DescriptorSets.empty ()) return;

    auto poolIt = m_PoolCache.find ( PoolName );
    if (poolIt == m_PoolCache.end ())
        {
        LogError ( "Cannot free descriptor sets - pool not found: ", PoolName );
        return;
        }

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkFreeDescriptorSets (
        device,
        poolIt->second,
        static_cast< uint32_t >( DescriptorSets.size () ),
        DescriptorSets.data () );

    // Удаляем из списка в информации о пуле
    auto infoIt = m_PoolInfoCache.find ( PoolName );
    if (infoIt != m_PoolInfoCache.end ())
        {
        for (auto set : DescriptorSets)
            {
            auto & sets = infoIt->second.DescriptorSets;
            sets.erase ( std::remove ( sets.begin (), sets.end (), set ), sets.end () );
            }
        }

    LogDebug ( "Freed ", DescriptorSets.size (), " descriptor sets from pool: ", PoolName );
    }

    //=============================================================================
    // Обновление дескрипторов
    //=============================================================================

void CDescriptorManager::UpdateDescriptorSet (
    VkDescriptorSet DescriptorSet,
    uint32_t Binding,
    VkDescriptorType Type,
    const FDescriptorResourceInfo & ResourceInfo )
    {
    VkWriteDescriptorSet write {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = DescriptorSet;
    write.dstBinding = Binding;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = Type;

    switch (Type)
        {
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                {
                VkDescriptorBufferInfo bufferInfo {};
                bufferInfo.buffer = ResourceInfo.Buffer;
                bufferInfo.offset = ResourceInfo.Offset;
                bufferInfo.range = ResourceInfo.Range;
                write.pBufferInfo = &bufferInfo;
                break;
                }

            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                {
                VkDescriptorImageInfo imageInfo {};
                imageInfo.sampler = ResourceInfo.Sampler;
                imageInfo.imageView = ResourceInfo.ImageView;
                imageInfo.imageLayout = ResourceInfo.ImageLayout;
                write.pImageInfo = &imageInfo;
                break;
                }

            case VK_DESCRIPTOR_TYPE_SAMPLER:
                {
                VkDescriptorImageInfo imageInfo {};
                imageInfo.sampler = ResourceInfo.Sampler;
                write.pImageInfo = &imageInfo;
                break;
                }

            default:
                LogError ( "Unsupported descriptor type for update: ", static_cast< int >( Type ) );
                return;
        }

    UpdateDescriptorSets ( { write } );
    }

void CDescriptorManager::UpdateDescriptorSets ( const std::vector<VkWriteDescriptorSet> & Writes )
    {
    if (Writes.empty ()) return;

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkUpdateDescriptorSets (
        device,
        static_cast< uint32_t >( Writes.size () ),
        Writes.data (),
        0,
        nullptr );
    }

void CDescriptorManager::UpdateBufferDescriptor (
    VkDescriptorSet DescriptorSet,
    uint32_t Binding,
    VkBuffer Buffer,
    VkDeviceSize Offset,
    VkDeviceSize Range,
    VkDescriptorType Type )
    {
    FDescriptorResourceInfo info;
    info.Type = Type;
    info.Buffer = Buffer;
    info.Offset = Offset;
    info.Range = Range;
    UpdateDescriptorSet ( DescriptorSet, Binding, Type, info );
    }

void CDescriptorManager::UpdateImageDescriptor (
    VkDescriptorSet DescriptorSet,
    uint32_t Binding,
    VkImageView ImageView,
    VkSampler Sampler,
    VkImageLayout Layout,
    VkDescriptorType Type )
    {
    FDescriptorResourceInfo info;
    info.Type = Type;
    info.ImageView = ImageView;
    info.Sampler = Sampler;
    info.ImageLayout = Layout;
    UpdateDescriptorSet ( DescriptorSet, Binding, Type, info );
    }

    //=============================================================================
    // Создание стандартных layout'ов
    //=============================================================================

bool CDescriptorManager::CreateDefaultLayouts ()
    {
    LogDebug ( "Creating default descriptor set layouts..." );

    // 1. Глобальный layout (камера, время и т.д.)
    std::vector<VkDescriptorSetLayoutBinding> globalBindings ( 1 );
    globalBindings[ 0 ].binding = 0;
    globalBindings[ 0 ].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalBindings[ 0 ].descriptorCount = 1;
    globalBindings[ 0 ].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    globalBindings[ 0 ].pImmutableSamplers = nullptr;

    m_GlobalLayout = CreateDescriptorSetLayout ( "GlobalLayout", globalBindings );
    if (m_GlobalLayout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create global descriptor set layout" );
        return false;
        }

        // 2. Per-object layout (матрица модели)
    std::vector<VkDescriptorSetLayoutBinding> objectBindings ( 1 );
    objectBindings[ 0 ].binding = 0;
    objectBindings[ 0 ].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    objectBindings[ 0 ].descriptorCount = 1;
    objectBindings[ 0 ].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    objectBindings[ 0 ].pImmutableSamplers = nullptr;

    m_PerObjectLayout = CreateDescriptorSetLayout ( "PerObjectLayout", objectBindings );
    if (m_PerObjectLayout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create per-object descriptor set layout" );
        return false;
        }

        // 3. Текстурный layout
    std::vector<VkDescriptorSetLayoutBinding> textureBindings ( 2 );
    textureBindings[ 0 ].binding = 0;
    textureBindings[ 0 ].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBindings[ 0 ].descriptorCount = 1;
    textureBindings[ 0 ].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    textureBindings[ 0 ].pImmutableSamplers = nullptr;

    textureBindings[ 1 ].binding = 1;
    textureBindings[ 1 ].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBindings[ 1 ].descriptorCount = 1;
    textureBindings[ 1 ].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    textureBindings[ 1 ].pImmutableSamplers = nullptr;

    m_TextureLayout = CreateDescriptorSetLayout ( "TextureLayout", textureBindings );
    if (m_TextureLayout == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create texture descriptor set layout" );
        return false;
        }

    LogDebug ( "Default descriptor set layouts created successfully" );
    return true;
    }

    //=============================================================================
    // Создание стандартных пулов
    //=============================================================================

bool CDescriptorManager::CreateDefaultPools ()
    {
    LogDebug ( "Creating default descriptor pools..." );

    // 1. Глобальный пул (для ресурсов, живущих всё время)
    std::vector<VkDescriptorPoolSize> globalPoolSizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20 },
        { VK_DESCRIPTOR_TYPE_SAMPLER, 10 }
        };

    m_GlobalPool = CreateDescriptorPool (
        "GlobalPool",
        100,  // max sets
        globalPoolSizes,
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT );

    if (m_GlobalPool == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create global descriptor pool" );
        return false;
        }

        // 2. Per-frame пул (для ресурсов, обновляемых каждый кадр)
    std::vector<VkDescriptorPoolSize> perFramePoolSizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 50 }
        };

    m_PerFramePool = CreateDescriptorPool (
        "PerFramePool",
        200,  // max sets
        perFramePoolSizes,
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT );

    if (m_PerFramePool == VK_NULL_HANDLE)
        {
        LogError ( "Failed to create per-frame descriptor pool" );
        return false;
        }

    LogDebug ( "Default descriptor pools created successfully" );
    return true;
    }

    //=============================================================================
    // Сброс пулов
    //=============================================================================

bool CDescriptorManager::ResetDescriptorPool ( const std::string & PoolName )
    {
    auto poolIt = m_PoolCache.find ( PoolName );
    if (poolIt == m_PoolCache.end ())
        {
        LogError ( "Cannot reset pool - not found: ", PoolName );
        return false;
        }

    return ResetDescriptorPool ( poolIt->second );
    }

bool CDescriptorManager::ResetDescriptorPool ( VkDescriptorPool Pool )
    {
    if (Pool == VK_NULL_HANDLE)
        {
        LogError ( "Cannot reset null descriptor pool" );
        return false;
        }

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkResult result = vkResetDescriptorPool ( device, Pool, 0 );

    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to reset descriptor pool: ", static_cast< int >( result ) );
        return false;
        }

        // Очищаем информацию о наборах для этого пула
    for (auto & [name, info] : m_PoolInfoCache)
        {
        if (info.Pool == Pool)
            {
            info.DescriptorSets.clear ();
            break;
            }
        }

    return true;
    }

    //=============================================================================
    // Внутренние методы
    //=============================================================================

VkDescriptorSetLayout CDescriptorManager::CreateDescriptorSetLayoutInternal (
    const std::vector<VkDescriptorSetLayoutBinding> & Bindings )
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkDescriptorSetLayoutCreateInfo layoutInfo {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast< uint32_t >( Bindings.size () );
    layoutInfo.pBindings = Bindings.data ();

    VkDescriptorSetLayout layout;
    VkResult result = vkCreateDescriptorSetLayout ( device, &layoutInfo, nullptr, &layout );

    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create descriptor set layout: ", static_cast< int >( result ) );
        return VK_NULL_HANDLE;
        }

    return layout;
    }

VkDescriptorPool CDescriptorManager::CreateDescriptorPoolInternal (
    uint32_t MaxSets,
    const std::vector<VkDescriptorPoolSize> & PoolSizes,
    VkDescriptorPoolCreateFlags Flags )
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast< uint32_t >( PoolSizes.size () );
    poolInfo.pPoolSizes = PoolSizes.data ();
    poolInfo.maxSets = MaxSets;
    poolInfo.flags = Flags;

    VkDescriptorPool pool;
    VkResult result = vkCreateDescriptorPool ( device, &poolInfo, nullptr, &pool );

    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create descriptor pool: ", static_cast< int >( result ) );
        return VK_NULL_HANDLE;
        }

    return pool;
    }