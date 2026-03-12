#ifndef CEMATH_CONSTANTS_H
#define CEMATH_CONSTANTS_H

namespace CEMath
{
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWO_PI = 6.28318530717958647692f;
    constexpr float HALF_PI = 1.57079632679489661923f;
    constexpr float QUARTER_PI = 0.785398163397448309616f;

    constexpr float DEG_TO_RAD = PI / 180.0f;
    constexpr float RAD_TO_DEG = 180.0f / PI;

    constexpr float EPSILON = 1e-6f;
    constexpr float EPSILON_SQUARED = 1e-12f;

    constexpr float FLOAT_MAX = 3.402823466e+38f;
    constexpr float FLOAT_MIN = 1.175494351e-38f;
}

#endif // CEMATH_CONSTANTS_H