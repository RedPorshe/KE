#include "Render/Vulkan/VertexStructs/WireframeVertex.h"

VkVertexInputBindingDescription FWireframeVertex::GetBindingDescription ()
    {
    VkVertexInputBindingDescription binding {};
    binding.binding = 0;
    binding.stride = sizeof ( FWireframeVertex );
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding;
    }

std::vector<VkVertexInputAttributeDescription> FWireframeVertex::GetAttributeDescriptions ()
    {
    std::vector<VkVertexInputAttributeDescription> attributes ( 2 );

    // Position
    attributes[ 0 ].binding = 0;
    attributes[ 0 ].location = 0;
    attributes[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[ 0 ].offset = offsetof ( FWireframeVertex, Position );

    // Color
    attributes[ 1 ].binding = 0;
    attributes[ 1 ].location = 1;
    attributes[ 1 ].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[ 1 ].offset = offsetof ( FWireframeVertex, Color );

    return attributes;
    }
