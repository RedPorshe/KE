#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inUV;

// Дескрипторный набор 0 для глобальных uniform-ов
layout(set = 0, binding = 0) uniform GlobalUniform {
    mat4 view;
    mat4 projection;
} global;

// Push constant только для model matrix
layout(push_constant) uniform ModelPush {
    mat4 model;
} push;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragWorldPos;
layout(location = 4) out vec3 fragViewDir;

void main() {
    vec4 worldPos = push.model * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;
    
    mat3 normalMatrix = transpose(inverse(mat3(push.model)));
    fragNormal = normalize(normalMatrix * inNormal);
    
    fragUV = inUV;
    fragColor = inColor;
    
    // Направление к камере
    vec3 cameraPos = - (global.view[3].xyz * mat3(global.view));
    fragViewDir = normalize(cameraPos - fragWorldPos);
    
    gl_Position = global.projection * global.view * worldPos;
}