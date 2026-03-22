#include "KE/Vulkan/Managers/BufferManager.h"
#include "KE/Vulkan/Managers/DeviceManager.h"
#include "KE/Vulkan/Managers/CommandManager.h"
#include "KE/Vulkan/VKinfo.h"

BufferManager::BufferManager () :IVKManager ()
	{}

BufferManager::~BufferManager ()
	{
	Shutdown ();
	}

bool BufferManager::Init ()
	{
    LogDebug ( "Initializing BufferManager..." );

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    if (!deviceMgr || !deviceMgr->IsInitialized ())
        {
        LogError ( "DeviceManager not initialized" );
        return false;
        }

    auto * cmdMgr = static_cast< CommandManager * >( m_info->Managers.CommandManager.get () );
    if (!cmdMgr || !cmdMgr->IsInitialized ())
        {
        LogError ( "CommandManager not initialized" );
        return false;
        }

    LogDebug ( "BufferManager initialized successfully" );
    bIsInitialized = true;
    return true;

	}

void BufferManager::Shutdown ()
	{}

const std::string & BufferManager::GetManagerName () const
	{
	static const std::string name = "Buffer Manager";
	return name;
	}

FBuffer BufferManager::CreateBuffer ( VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties )
	{
	FBuffer buffer;
	buffer.Size = Size;
	buffer.Usage = Usage;
	VkDevice device = VK_NULL_HANDLE;
	if (DeviceManager * deviceManager = dynamic_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () ))
		{
		device = deviceManager->GetDevice ();
		}
	
	buffer.Buffer = CreateBufferHandle ( Size, Usage );
	if (buffer.Buffer == VK_NULL_HANDLE)
		{
		return buffer;
		}


	buffer.Memory = AllocateBufferMemory ( buffer.Buffer, Properties );
	if (buffer.Memory == VK_NULL_HANDLE)
		{		
		vkDestroyBuffer ( device, buffer.Buffer, nullptr );
		buffer.Buffer = VK_NULL_HANDLE;
		return buffer;
		}

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


FBuffer BufferManager::CreateVertexBuffer ( VkDeviceSize Size, const void * Data )
    {
    LogDebug ( "Creating vertex buffer - Size: ", Size, ", Data: ", ( void * ) Data );
    FBuffer buffer;

    if (Size == 0)
        {
        LogError ( "Cannot create vertex buffer with size 0" );
        return buffer;
        }

    if (Data)
        {
        LogDebug ( "Creating staging buffer for vertex data" );
        FBuffer stagingBuffer = CreateStagingBuffer ( Size, Data );
        if (!stagingBuffer.IsValid ())
            {
            LogError ( "Failed to create staging buffer" );
            return buffer;
            }

        LogDebug ( "Creating device local vertex buffer" );
        buffer = CreateBuffer ( Size,
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

        if (buffer.IsValid ())
            {
            LogDebug ( "Copying data from staging to device buffer" );
            CopyBufferToBuffer ( stagingBuffer, buffer, Size );
            }

        DestroyBuffer ( stagingBuffer );
        }
    else
        {
        LogDebug ( "Creating host visible vertex buffer" );
        buffer = CreateBuffer ( Size,
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
        }

    if (buffer.IsValid ())
        {
        LogDebug ( "Vertex buffer created successfully" );
        }
    else
        {
        LogError ( "Failed to create vertex buffer" );
        }

    return buffer;
    }


FBuffer BufferManager::CreateIndexBuffer ( VkDeviceSize Size, const void * Data )
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

FBuffer BufferManager::CreateUniformBuffer ( VkDeviceSize Size )
    {
    return CreateBuffer ( Size,
                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

    }

FBuffer BufferManager::CreateStagingBuffer ( VkDeviceSize Size, const void * Data )
    {
        // Staging buffer должен иметь флаг TRANSFER_SRC_BIT, а не TRANSFER_DST_BIT
    FBuffer buffer = CreateBuffer ( Size,
                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,  // ← Исправлено!
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

    if (buffer.IsValid () && Data)
        {
        CopyDataToBuffer ( buffer, Data, Size );
        }

    return buffer;
    }

void BufferManager::CopyDataToBuffer ( const FBuffer & DstBuffer, const void * Data, VkDeviceSize Size )
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

void BufferManager::CopyBufferToBuffer ( const FBuffer & SrcBuffer, const FBuffer & DstBuffer, VkDeviceSize Size )
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

void * BufferManager::MapBuffer ( FBuffer & Buffer )
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

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    VkResult result = vkMapMemory ( device, Buffer.Memory, 0, Buffer.Size, 0, &Buffer.MappedData );
    if (result != VK_SUCCESS)
        {
        LogError ( "Failed to map buffer memory: ", static_cast< int >( result ) );
        return nullptr;
        }

    return Buffer.MappedData;

    }

void BufferManager::UnmapBuffer ( FBuffer & Buffer )
    {
    if (!Buffer.IsValid () || !Buffer.MappedData)
        {
        return;
        }

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    vkUnmapMemory ( device, Buffer.Memory );
    Buffer.MappedData = nullptr;
    }

void BufferManager::UpdateUniformBuffer ( FBuffer & Buffer, const void * Data, VkDeviceSize Size )
    {
    CopyDataToBuffer ( Buffer, Data, Size );
    }

void BufferManager::DestroyBuffer ( FBuffer & Buffer )
    {
    if (!Buffer.IsValid ()) return;

    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
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

VkBuffer BufferManager::CreateBufferHandle ( VkDeviceSize Size, VkBufferUsageFlags Usage )
    {
    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
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

VkDeviceMemory BufferManager::AllocateBufferMemory ( VkBuffer Buffer, VkMemoryPropertyFlags Properties )
    {
    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
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

uint32_t BufferManager::FindMemoryType ( uint32_t TypeFilter, VkMemoryPropertyFlags Properties ) const
    {
    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    return deviceMgr->FindMemoryType ( TypeFilter, Properties );

    }

void BufferManager::CopyDataToDeviceMemory ( VkDeviceMemory Memory, const void * Data, VkDeviceSize Size )
    {
    auto * deviceMgr = static_cast< DeviceManager * >( m_info->Managers.DeviceManager.get () );
    VkDevice device = deviceMgr->GetDevice ();

    void * mappedData;
    vkMapMemory ( device, Memory, 0, Size, 0, &mappedData );
    memcpy ( mappedData, Data, static_cast< size_t >( Size ) );
    vkUnmapMemory ( device, Memory );
    }

void BufferManager::ExecuteSingleTimeCommand ( std::function<void ( VkCommandBuffer )> && Function )
    {
    auto * cmdMgr = static_cast< CommandManager * >( m_info->Managers.CommandManager.get () );

    VkCommandBuffer cmdBuffer = cmdMgr->BeginSingleTimeCommands ();
    if (cmdBuffer != VK_NULL_HANDLE)
        {
        Function ( cmdBuffer );
        cmdMgr->EndSingleTimeCommands ( cmdBuffer );
        }
    }
