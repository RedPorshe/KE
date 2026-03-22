#pragma once
#include "CoreMinimal.h"
#include <vulkan/vulkan.h>

struct KE_API FWireframeVertex
    {
    FVector Position;
    FVector Color;

    static VkVertexInputBindingDescription GetBindingDescription ();
        

    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions ();
       
    };