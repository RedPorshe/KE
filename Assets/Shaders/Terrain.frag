#version 450

// Входные данные от вершинного шейдера
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragWorldPos;
layout(location = 4) in float fragHeight;

// Дескрипторный набор 0 для глобальных uniform-ов (для обратной совместимости)
layout(set = 0, binding = 0) uniform GlobalUniform {
    mat4 view;
    mat4 projection;
} global;

// Push constants для model matrix и параметров террейна
layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 terrainParams; // x = tiling factor, y = height scale, z = fog density, w = use texture flag
} push;

// Выходной цвет
layout(location = 0) out vec4 outColor;

// Освещение
const vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5));
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 ambientColor = vec3(0.2, 0.2, 0.3);

// Функция для получения цвета на основе высоты (градиент)
vec3 getHeightColor(float height) {
    // Определяем диапазоны высот
    float low = -50.0;
    float midLow = 0.0;
    float midHigh = 50.0;
    float high = 100.0;
    
    if (height < low) {
        // Глубокая вода
        return vec3(0.0, 0.2, 0.5);
    } else if (height < midLow) {
        // Мелкая вода / песок
        float t = (height - low) / (midLow - low);
        return mix(vec3(0.0, 0.2, 0.5), vec3(0.8, 0.7, 0.5), t);
    } else if (height < midHigh) {
        // Трава / земля
        float t = (height - midLow) / (midHigh - midLow);
        return mix(vec3(0.2, 0.6, 0.2), vec3(0.4, 0.3, 0.2), t);
    } else {
        // Скалы / снег
        float t = min(1.0, (height - midHigh) / (high - midHigh));
        return mix(vec3(0.4, 0.3, 0.2), vec3(1.0, 1.0, 1.0), t);
    }
}

// Функция для простого тумана
vec3 applyFog(vec3 color, float distance) {
    float fogFactor = exp(-distance * push.terrainParams.z);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    return mix(vec3(0.5, 0.6, 0.7), color, fogFactor);
}

void main() {
    vec3 finalColor;
    
    // Проверяем, использовать ли текстуры или цвет по высоте
    if (push.terrainParams.w > 0.5) {
        // Здесь будет текстурный сэмплинг, когда добавите текстуры
        // finalColor = texture(terrainTexture, fragUV).rgb;
        finalColor = fragColor; // Пока используем входящий цвет
    } else {
        // Используем градиент по высоте
        finalColor = getHeightColor(fragHeight);
    }
    
    // Простое освещение
    vec3 normal = normalize(fragNormal);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Комбинируем
    finalColor = finalColor * (ambientColor + diffuse);
    
    // Добавляем туман (используем view матрицу из дескрипторов для позиции камеры)
    vec3 cameraPos = - (global.view[3].xyz * mat3(global.view));
    float distance = length(fragWorldPos - cameraPos);
    finalColor = applyFog(finalColor, distance);
    
    outColor = vec4(finalColor, 1.0);
}