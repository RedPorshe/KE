// MathGLMConverters.h
#pragma once
#include "Utils/Math/MathTypes.h"

// Подключаем все необходимые заголовки GLM
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>  // для value_ptr
#include <glm/gtx/matrix_decompose.hpp>  // для decompose (исправлено!)
#include <glm/gtx/transform.hpp>  // для translate, scale, rotate

struct  FTransform;

namespace CEMath
    {
        // Конвертация из CEMath в GLM
    inline glm::vec2 ToGLM ( const Vector2D & v ) { return glm::vec2 ( v.x, v.y ); }
    inline glm::vec3 ToGLM ( const Vector3D & v ) { return glm::vec3 ( v.x, v.y, v.z ); }
    inline glm::vec4 ToGLM ( const Vector4D & v ) { return glm::vec4 ( v.x, v.y, v.z, v.w ); }

    inline glm::quat ToGLM ( const Quaternion & q ) { return glm::quat ( q.w, q.x, q.y, q.z ); }

    inline glm::mat4 ToGLM ( const Matrix4x4 & m )
        {
            // Создаем матрицу из массива (GLM ожидает column-major порядок)
        return glm::make_mat4 ( m.m );
        }

        // Конвертация из GLM в CEMath
    inline Vector2D FromGLM ( const glm::vec2 & v ) { return Vector2D ( v.x, v.y ); }
    inline Vector3D FromGLM ( const glm::vec3 & v ) { return Vector3D ( v.x, v.y, v.z ); }
    inline Vector4D FromGLM ( const glm::vec4 & v ) { return Vector4D ( v.x, v.y, v.z, v.w ); }

    inline Quaternion FromGLM ( const glm::quat & q ) { return Quaternion ( q.x, q.y, q.z, q.w ); }

    inline Matrix4x4 FromGLM ( const glm::mat4 & m )
        {
            // Получаем указатель на данные GLM матрицы
        const float * glmData = glm::value_ptr ( m );
        return Matrix4x4 ( glmData );
        }

    }