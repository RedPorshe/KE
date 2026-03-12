#include "Render/Vulkan/Managers/BufferManager.h"
#include "Core/EngineInfo.h"
#include "Render/Vulkan/Managers/DeviceManager.h"
#include "Render/Vulkan/Managers/CommandManager.h"
#include <functional>

CBufferManager::CBufferManager ( FEngineInfo & Info )
    : IVulkanManager ( Info )
    {}

CBufferManager::~CBufferManager ()
    {
    Shutdown ();
    LogDebug ( GetManagerName (), " destroyed" );
    }

bool CBufferManager::Initialize ()
    {
    LogDebug ( "Initializing BufferManager..." );

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    if (!deviceMgr || !deviceMgr->IsInitialized ())
        {
        LogError ( "DeviceManager not initialized" );
        return false;
        }

    auto * cmdMgr = static_cast< CCommandManager * >( m_Info.Vulkan.CommandManager.get () );
    if (!cmdMgr || !cmdMgr->IsInitialized ())
        {
        LogError ( "CommandManager not initialized" );
        return false;
        }

    LogDebug ( "BufferManager initialized successfully" );
    m_bInitialized = true;
    return true;
    }

void CBufferManager::Shutdown ()
    {
    if (!m_bInitialized) return;

    LogDebug ( "Shutting down BufferManager..." );

    // Destroy triangle vertex buffer
    if (m_TriangleVertexBuffer.IsValid ())
        {
        DestroyBuffer ( m_TriangleVertexBuffer );
        }

    m_bInitialized = false;
    LogDebug ( "BufferManager shutdown complete" );
    }

const char * CBufferManager::GetManagerName () const
    {
    return "BufferManager";
    }

FBuffer CBufferManager::CreateBuffer ( VkDeviceSize Size,
                                       VkBufferUsageFlags Usage,
                                       VkMemoryPropertyFlags Properties )
    {
    FBuffer buffer;
    buffer.Size = Size;
    buffer.Usage = Usage;

    // Create buffer
    buffer.Buffer = CreateBufferHandle ( Size, Usage );
    if (buffer.Buffer == VK_NULL_HANDLE)
        {
        return buffer;
        }

    // Allocate memory
    buffer.Memory = AllocateBufferMemory ( buffer.Buffer, Properties );
    if (buffer.Memory == VK_NULL_HANDLE)
        {
        auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
        VkDevice device = deviceMgr->GetDevice ();
        vkDestroyBuffer ( device, buffer.Buffer, nullptr );
        buffer.Buffer = VK_NULL_HANDLE;
        return buffer;
        }

    // Bind memory
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkResult result = vkBindBufferMemory ( device, buffer.Buffer, buffer.Memory, 0 );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to bind buffer memory: ", static_cast< int >( result ) );
        vkDestroyBuffer ( device, buffer.Buffer, nullptr );
        vkFreeMemory ( device, buffer.Memory, nullptr );
        buffer.Invalidate ();
        }

    return buffer;
    }

FBuffer CBufferManager::CreateVertexBuffer ( VkDeviceSize Size, const void * Data )
    {
   // LogDebug ( "Creating vertex buffer - Size: ", Size, ", Data: ", ( void * ) Data );

    FBuffer buffer;

    if (Size == 0)
        {
        LogError ( "Cannot create vertex buffer with size 0" );
        return buffer;
        }

    if (Data)
        {
       // LogDebug ( "Creating staging buffer for vertex data" );
        FBuffer stagingBuffer = CreateStagingBuffer ( Size, Data );
        if (!stagingBuffer.IsValid ())
            {
            LogError ( "Failed to create staging buffer" );
            return buffer;
            }

       // LogDebug ( "Creating device local vertex buffer" );
        buffer = CreateBuffer ( Size,
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

        if (buffer.IsValid ())
            {
          //  LogDebug ( "Copying data from staging to device buffer" );
            CopyBufferToBuffer ( stagingBuffer, buffer, Size );
            }

        DestroyBuffer ( stagingBuffer );
        }
    else
        {
       // LogDebug ( "Creating host visible vertex buffer" );
        buffer = CreateBuffer ( Size,
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
        }

    if (buffer.IsValid ())
        {
       // LogDebug ( "Vertex buffer created successfully" );
        }
    else
        {
        LogError ( "Failed to create vertex buffer" );
        }

    return buffer;
    }

FBuffer CBufferManager::CreateIndexBuffer ( VkDeviceSize Size, const void * Data )
    {
    FBuffer buffer;

    if (Data)
        {
        // Create staging buffer
        FBuffer stagingBuffer = CreateStagingBuffer ( Size, Data );
        if (!stagingBuffer.IsValid ())
            {
            LogError ( "Failed to create staging buffer" );
            return buffer;
            }

        // Create device local buffer
        buffer = CreateBuffer ( Size,
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

        if (buffer.IsValid ())
            {
            // Copy from staging to device local buffer
            CopyBufferToBuffer ( stagingBuffer, buffer, Size );
            }

        // Cleanup staging buffer
        DestroyBuffer ( stagingBuffer );
        }
    else
        {
        // Create host visible buffer for dynamic data
        buffer = CreateBuffer ( Size,
                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
        }

    return buffer;
    }

FBuffer CBufferManager::CreateUniformBuffer ( VkDeviceSize Size )
    {
    return CreateBuffer ( Size,
                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
    }

FBuffer CBufferManager::CreateStagingBuffer ( VkDeviceSize Size, const void * Data )
    {
    FBuffer buffer = CreateBuffer ( Size,
                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

    if (buffer.IsValid () && Data)
        {
        CopyDataToBuffer ( buffer, Data, Size );
        }

    return buffer;
    }

FBuffer CBufferManager::CreateTriangleVertexBuffer ()
    {
    LogDebug ( "Creating triangle vertex buffer..." );

    // Проверяем, что данные не пустые
    if (m_TriangleVertices.empty ())
        {
        LogError ( "Triangle vertices are empty" );
        return FBuffer ();
        }

    LogDebug ( "Triangle vertices count: ", m_TriangleVertices.size () );
    LogDebug ( "Vertex size: ", sizeof ( FTriangleVertex ), " bytes" );
    LogDebug ( "Total buffer size: ", sizeof ( FTriangleVertex ) * m_TriangleVertices.size (), " bytes" );

    m_TriangleVertexBuffer = CreateVertexBuffer ( m_TriangleVertices );

    if (!m_TriangleVertexBuffer.IsValid ())
        {
        LogError ( "Failed to create triangle vertex buffer" );
        return m_TriangleVertexBuffer;
        }

    LogDebug ( "Triangle vertex buffer created successfully" );
    LogDebug ( "Buffer handle: ", ( void * ) m_TriangleVertexBuffer.Buffer );
    LogDebug ( "Buffer size: ", m_TriangleVertexBuffer.Size );

    return m_TriangleVertexBuffer;
    }

void CBufferManager::CopyDataToBuffer ( const FBuffer & DstBuffer, const void * Data, VkDeviceSize Size )
    {
    if (!DstBuffer.IsValid () || !Data || Size == 0)
        {
        LogError ( "Invalid parameters for CopyDataToBuffer" );
        return;
        }

    if (Size > DstBuffer.Size)
        {
        LogError ( "Data size exceeds buffer size" );
        return;
        }

    void * mappedData = MapBuffer ( const_cast< FBuffer & >( DstBuffer ) );
    if (mappedData)
        {
        memcpy ( mappedData, Data, static_cast< size_t >( Size ) );
        UnmapBuffer ( const_cast< FBuffer & >( DstBuffer ) );
        }
    }

void CBufferManager::CopyBufferToBuffer ( const FBuffer & SrcBuffer, const FBuffer & DstBuffer, VkDeviceSize Size )
    {
    if (!SrcBuffer.IsValid () || !DstBuffer.IsValid () || Size == 0)
        {
        LogError ( "Invalid parameters for CopyBufferToBuffer" );
        return;
        }

    if (Size > SrcBuffer.Size || Size > DstBuffer.Size)
        {
        LogError ( "Copy size exceeds buffer size" );
        return;
        }

    ExecuteSingleTimeCommand ( [ & ] ( VkCommandBuffer cmdBuffer )
                               {
                               VkBufferCopy copyRegion {};
                               copyRegion.srcOffset = 0;
                               copyRegion.dstOffset = 0;
                               copyRegion.size = Size;
                               vkCmdCopyBuffer ( cmdBuffer, SrcBuffer.Buffer, DstBuffer.Buffer, 1, &copyRegion );
                               } );
    }

void * CBufferManager::MapBuffer ( FBuffer & Buffer )
    {
    if (!Buffer.IsValid ())
        {
        LogError ( "Cannot map invalid buffer" );
        return nullptr;
        }

    if (Buffer.MappedData)
        {
        return Buffer.MappedData;
        }

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkResult result = vkMapMemory ( device, Buffer.Memory, 0, Buffer.Size, 0, &Buffer.MappedData );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to map buffer memory: ", static_cast< int >( result ) );
        return nullptr;
        }

    return Buffer.MappedData;
    }

void CBufferManager::UnmapBuffer ( FBuffer & Buffer )
    {
    if (!Buffer.IsValid () || !Buffer.MappedData)
        {
        return;
        }

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkUnmapMemory ( device, Buffer.Memory );
    Buffer.MappedData = nullptr;
    }

void CBufferManager::UpdateUniformBuffer ( FBuffer & Buffer, const void * Data, VkDeviceSize Size )
    {
    CopyDataToBuffer ( Buffer, Data, Size );
    }

void CBufferManager::DestroyBuffer ( FBuffer & Buffer )
    {
    if (!Buffer.IsValid ()) return;

    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    if (Buffer.MappedData)
        {
        vkUnmapMemory ( device, Buffer.Memory );
        Buffer.MappedData = nullptr;
        }

    if (Buffer.Buffer != VK_NULL_HANDLE)
        {
        vkDestroyBuffer ( device, Buffer.Buffer, nullptr );
        }

    if (Buffer.Memory != VK_NULL_HANDLE)
        {
        vkFreeMemory ( device, Buffer.Memory, nullptr );
        }

    Buffer.Invalidate ();
    }

VkBuffer CBufferManager::CreateBufferHandle ( VkDeviceSize Size, VkBufferUsageFlags Usage )
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = Size;
    bufferInfo.usage = Usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer buffer;
    VkResult result = vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to create buffer: ", static_cast< int >( result ) );
        return VK_NULL_HANDLE;
        }

    return buffer;
    }

VkDeviceMemory CBufferManager::AllocateBufferMemory ( VkBuffer Buffer, VkMemoryPropertyFlags Properties )
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements ( device, Buffer, &memRequirements );

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType ( memRequirements.memoryTypeBits, Properties );

    if (allocInfo.memoryTypeIndex == UINT32_MAX)
        {
        LogError ( "Failed to find suitable memory type" );
        return VK_NULL_HANDLE;
        }

    VkDeviceMemory memory;
    VkResult result = vkAllocateMemory ( device, &allocInfo, nullptr, &memory );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to allocate buffer memory: ", static_cast< int >( result ) );
        return VK_NULL_HANDLE;
        }

    return memory;
    }

uint32_t CBufferManager::FindMemoryType ( uint32_t TypeFilter, VkMemoryPropertyFlags Properties ) const
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    return deviceMgr->FindMemoryType ( TypeFilter, Properties );
    }

void CBufferManager::CopyDataToDeviceMemory ( VkDeviceMemory Memory, const void * Data, VkDeviceSize Size )
    {
    auto * deviceMgr = static_cast< CDeviceManager * >( m_Info.Vulkan.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    void * mappedData;
    vkMapMemory ( device, Memory, 0, Size, 0, &mappedData );
    memcpy ( mappedData, Data, static_cast< size_t >( Size ) );
    vkUnmapMemory ( device, Memory );
    }

void CBufferManager::ExecuteSingleTimeCommand ( std::function<void ( VkCommandBuffer )> && Function )
    {
    auto * cmdMgr = static_cast< CCommandManager * >( m_Info.Vulkan.CommandManager.get () );

    VkCommandBuffer cmdBuffer = cmdMgr->BeginSingleTimeCommands ();
    if (cmdBuffer != VK_NULL_HANDLE)
        {
        Function ( cmdBuffer );
        cmdMgr->EndSingleTimeCommands ( cmdBuffer );
        }
    }