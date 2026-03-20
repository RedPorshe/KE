#pragma once
#include "KE/Vulkan/VKManager.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <functional>
#include <unordered_map>



struct KE_API FBuffer
	{
	VkBuffer Buffer = VK_NULL_HANDLE;
	VkDeviceMemory Memory = VK_NULL_HANDLE;
	VkDeviceSize Size = 0;
	VkBufferUsageFlags Usage = 0;
	void * MappedData = nullptr;

	bool IsValid () const { return Buffer != VK_NULL_HANDLE && Memory != VK_NULL_HANDLE; }
	void Invalidate ()
		{
		Buffer = VK_NULL_HANDLE;
		Memory = VK_NULL_HANDLE;
		Size = 0;
		Usage = 0;
		MappedData = nullptr;
		}
	};


class KE_API BufferManager final : public IVKManager
	{
	public:
		BufferManager ();
		virtual ~BufferManager () override;

		bool Init () override;

		void Shutdown () override;

		const std::string & GetManagerName () const override;

		// Buffer creation
		FBuffer CreateBuffer ( VkDeviceSize Size,
							   VkBufferUsageFlags Usage,
							   VkMemoryPropertyFlags Properties );

		   // Specific buffer types
		FBuffer CreateVertexBuffer ( VkDeviceSize Size, const void * Data = nullptr );
		FBuffer CreateIndexBuffer ( VkDeviceSize Size, const void * Data = nullptr );
		FBuffer CreateUniformBuffer ( VkDeviceSize Size );
		FBuffer CreateStagingBuffer ( VkDeviceSize Size, const void * Data = nullptr );

		// Template versions for convenience
		template<typename T>
		FBuffer CreateVertexBuffer ( const std::vector<T> & Vertices )
			{
			return CreateVertexBuffer ( sizeof ( T ) * Vertices.size (), Vertices.data () );
			}

		template<typename T>
		FBuffer CreateIndexBuffer ( const std::vector<T> & Indices )
			{
			return CreateIndexBuffer ( sizeof ( T ) * Indices.size (), Indices.data () );
			}

		template<typename T>
		FBuffer CreateUniformBuffer ()
			{
			return CreateUniformBuffer ( sizeof ( T ) );
			}


		// Data operations
		void CopyDataToBuffer ( const FBuffer & DstBuffer, const void * Data, VkDeviceSize Size );
		void CopyBufferToBuffer ( const FBuffer & SrcBuffer, const FBuffer & DstBuffer, VkDeviceSize Size );

		// Mapping
		void * MapBuffer ( FBuffer & Buffer );
		void UnmapBuffer ( FBuffer & Buffer );
		void UpdateUniformBuffer ( FBuffer & Buffer, const void * Data, VkDeviceSize Size );

		// Cleanup
		void DestroyBuffer ( FBuffer & Buffer );

	private:
		VkBuffer CreateBufferHandle ( VkDeviceSize Size, VkBufferUsageFlags Usage );
		VkDeviceMemory AllocateBufferMemory ( VkBuffer Buffer, VkMemoryPropertyFlags Properties );
		uint32_t FindMemoryType ( uint32_t TypeFilter, VkMemoryPropertyFlags Properties ) const;

		// Staging helpers
		void CopyDataToDeviceMemory ( VkDeviceMemory Memory, const void * Data, VkDeviceSize Size );
		void ExecuteSingleTimeCommand ( std::function<void ( VkCommandBuffer )> && Function );

	

		
	

	};