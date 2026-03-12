#ifndef CEMATH_HELPERS_H
#define CEMATH_HELPERS_H

#include "Utils/Math/CE_MathConstants.h"
#include <cmath>
#include <algorithm>

namespace CEMath
{
    struct Vector3D;  // Forward declaration
    // math helpers
    inline bool IsZero(float value, float epsilon = EPSILON)
    {
        return std::fabs(value) < epsilon;
    }

    inline bool IsEqual(float a, float b, float epsilon = EPSILON)
    {
        return std::fabs(a - b) < epsilon;
    }

    inline float Clamp(float value, float min, float max)
    {
        return std::max(min, std::min(value, max));
    }

    inline float Lerp(float a, float b, float t)
    {
        return a + (b - a) * Clamp(t, 0.0f, 1.0f);
    }

    inline float InverseLerp(float a, float b, float value)
    {
        if (IsEqual(a, b))
            return 0.0f;

        return Clamp((value - a) / (b - a), 0.0f, 1.0f);
    }

    inline float SmoothStep(float edge0, float edge1, float x)
    {
        float t = Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    inline float Square(float value)
    {
        return value * value;
    }

    inline float SafeDivide(float numerator, float denominator, float fallback = 0.0f)
    {
        if (IsZero(denominator))
            return fallback;

        return numerator / denominator;
    }

    inline float DegreesToRadians(float degrees)
    {
        return degrees * DEG_TO_RAD;
    }

    inline float RadiansToDegrees(float radians)
    {
        return radians * RAD_TO_DEG;
    }

    inline float WrapAngle(float angle)
    {
        angle = std::fmod(angle + PI, TWO_PI);
        if (angle < 0.0f)
            angle += TWO_PI;
        return angle - PI;
    }

    inline float WrapAngleDegrees(float degrees)
    {
        degrees = std::fmod(degrees + 180.0f, 360.0f);
        if (degrees < 0.0f)
            degrees += 360.0f;
        return degrees - 180.0f;
    }

    inline float Sin(float radians)
    {
        return std::sin(radians);
    }

    inline float Cos(float radians)
    {
        return std::cos(radians);
    }

    inline float Tan(float radians)
    {
        return std::tan(radians);
    }

    inline float Asin(float value)
    {
        return std::asin(Clamp(value, -1.0f, 1.0f));
    }

    inline float Acos(float value)
    {
        return std::acos(Clamp(value, -1.0f, 1.0f));
    }

    inline float Atan(float value)
    {
        return std::atan(value);
    }

    inline float Atan2(float y, float x)
    {
        if (IsZero(x) && IsZero(y))
            return 0.0f;

        return std::atan2(y, x);
    }

    inline float Sqrt(float value)
    {
        if (value < 0.0f)
            return 0.0f;

        return std::sqrt(value);
    }

    inline float InverseSqrt(float value)
    {
        if (value <= 0.0f)
            return FLOAT_MAX;

        return 1.0f / std::sqrt(value);
    }

    inline float Abs(float value)
    {
        return std::fabs(value);
    }

    inline float Floor(float value)
    {
        return std::floor(value);
    }

    inline float Ceil(float value)
    {
        return std::ceil(value);
    }

    inline float Round(float value)
    {
        return std::round(value);
    }

    inline float Mod(float x, float y)
    {
        if (IsZero(y))
            return 0.0f;

        return std::fmod(x, y);
    }

    inline float Pow(float base, float exponent)
    {
        return std::pow(base, exponent);
    }

    inline float Exp(float value)
    {
        return std::exp(value);
    }

    inline float Log(float value)
    {
        if (value <= 0.0f)
            return -FLOAT_MAX;

        return std::log(value);
    }

    inline float Log10(float value)
    {
        if (value <= 0.0f)
            return -FLOAT_MAX;

        return std::log10(value);
    }

    inline float Min(float a, float b)
    {
        return std::min(a, b);
    }

    inline float Max(float a, float b)
    {
        return std::max(a, b);
    }

    inline int Sign(float value)
    {
        if (IsZero(value))
            return 0;

        return (value > 0.0f) ? 1 : -1;
    }

    inline bool IsPowerOfTwo(int value)
    {
        return value > 0 && (value & (value - 1)) == 0;
    }

    inline float CopySign(float magnitude, float sign)
    {
        return std::copysign(magnitude, sign); // C++11 and later
    }

    inline float NormalizeAngleDegrees(float degrees)
    {
        degrees = std::fmod(degrees, 360.0f);
        if (degrees < 0.0f)
            degrees += 360.0f;
        return degrees;
    }

    // Vector3D-dependent functions are moved to the end after Vector3D inclusion

    inline int NextPowerOfTwo(int value)
    {
        if (value <= 0)
            return 1;

        value--;
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        value++;

        return value;
    }
}

#endif // CEMATH_HELPERS_H