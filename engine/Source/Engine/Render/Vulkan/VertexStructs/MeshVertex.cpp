#include "Render/Vulkan/VertexStructs/MeshVertex.h"

VkVertexInputBindingDescription FMeshVertex::GetBindingDescription ()
    {
    VkVertexInputBindingDescription binding {};
    binding.binding = 0;
    binding.stride = sizeof ( FMeshVertex );
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding;
    }

std::vector<VkVertexInputAttributeDescription> FMeshVertex::GetAttributeDescriptions ()
    {
    std::vector<VkVertexInputAttributeDescription> attributes ( 4 );

    // Position
    attributes[ 0 ].binding = 0;
    attributes[ 0 ].location = 0;
    attributes[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[ 0 ].offset = offsetof ( FMeshVertex, Position );

    // Normal
    attributes[ 1 ].binding = 0;
    attributes[ 1 ].location = 1;
    attributes[ 1 ].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[ 1 ].offset = offsetof ( FMeshVertex, Normal );

    // Color
    attributes[ 2 ].binding = 0;
    attributes[ 2 ].location = 2;
    attributes[ 2 ].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[ 2 ].offset = offsetof ( FMeshVertex, Color );

    // UV
    attributes[ 3 ].binding = 0;
    attributes[ 3 ].location = 3;
    attributes[ 3 ].format = VK_FORMAT_R32G32_SFLOAT;
    attributes[ 3 ].offset = offsetof ( FMeshVertex, UV );

    return attributes;
    }
