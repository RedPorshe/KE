#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragWorldPos;
layout(location = 4) in vec3 fragViewDir;

layout(location = 0) out vec4 outColor;

// Освещение
const vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float lightIntensity = 1.2;

const vec3 ambientColor = vec3(0.2, 0.2, 0.3);
const float ambientIntensity = 0.3;

const float specularStrength = 0.5;
const float shininess = 32.0;

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(fragViewDir);
    
    // Ambient
    vec3 ambient = ambientColor * ambientIntensity;
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * lightIntensity;
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * specularStrength * lightColor;
    
    // Финальный цвет
    vec3 finalColor = fragColor * (ambient + diffuse) + specular;
    finalColor = pow(finalColor, vec3(1.0/2.2));
    
    outColor = vec4(finalColor, 1.0);
}