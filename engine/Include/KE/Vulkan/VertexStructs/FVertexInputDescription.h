#pragma once
#include "Core/KEExport.h"
#include <vulkan/vulkan.h>
#include <vector>

struct KE_API FVertexInputDescription
    {
    std::vector<VkVertexInputBindingDescription> Bindings;
    std::vector<VkVertexInputAttributeDescription> Attributes;
    };