#pragma once
#include "Render/Vulkan/VulkanInterface.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <functional>
#include <unordered_map>

struct FEngineInfo;

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

// Vertex structure for triangle (position + color)
struct KE_API FTriangleVertex
	{
	FVector position;  // x, y
	FVector color; // r, g, b

	static VkVertexInputBindingDescription GetBindingDescription ()
		{
		VkVertexInputBindingDescription binding {};
		binding.binding = 0;
		binding.stride = sizeof ( FTriangleVertex );  // 24 bytes
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding;
		}

	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions ()
		{
		std::vector<VkVertexInputAttributeDescription> attributes ( 2 );

	// Position (vec3)
		attributes[ 0 ].binding = 0;
		attributes[ 0 ].location = 0;
		attributes[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[ 0 ].offset = offsetof ( FTriangleVertex, position );

		// Color (vec3)
		attributes[ 1 ].binding = 0;
		attributes[ 1 ].location = 1;
		attributes[ 1 ].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[ 1 ].offset = offsetof ( FTriangleVertex, color );

		return attributes;
		}
	};

class KE_API CBufferManager final : public IVulkanManager
	{
	public:
		CBufferManager ( FEngineInfo & Info );
		virtual ~CBufferManager ();

		// IVulkanManager
		bool Initialize () override;
		void Shutdown () override;
		const char * GetManagerName () const override;

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

		// Triangle buffer helpers
		FBuffer CreateTriangleVertexBuffer ();
		FBuffer GetTriangleVertexBuffer () const { return m_TriangleVertexBuffer; }

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

	private:
		// Triangle vertex buffer (for testing)
		FBuffer m_TriangleVertexBuffer;

		// Default triangle vertices
		const std::vector<FTriangleVertex> m_TriangleVertices = {
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Левый верх (красный)
		{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},   // Правый верх (зеленый)
		{{0.0f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}   // Центр низ (синий)
			};
	};