#pragma once

#include <cmath>
#include <stdexcept>
#include "Utils/Math/CE_MathConstants.h"
#include "Utils/Math/CE_MathHelpers.h"
#include "Vector2D.h"
#include "Vector3D.h"

namespace CEMath
{
    struct KE_API Vector4D
    {
        float x{}, y{}, z{}, w{};

         
        Vector4D();
        Vector4D(float scalar);
        Vector4D(float inX, float inY, float inZ, float inW);
        Vector4D(int inX, int inY, int inZ, int inW);
        Vector4D(const Vector2D& vec2, float z = 0.0f, float w = 0.0f);
        Vector4D(const Vector3D& vec3, float w = 0.0f);
        Vector4D(const Vector4D& other);

         
        Vector4D& operator=(const Vector4D& other);

      
        bool operator==(const Vector4D& other) const;
        bool operator!=(const Vector4D& other) const;

         
        Vector4D operator+(const Vector4D& other) const;
        Vector4D operator-(const Vector4D& other) const;
        Vector4D operator*(const Vector4D& other) const;
        Vector4D operator/(const Vector4D& other) const;

        Vector4D operator+(float scalar) const;
        Vector4D operator-(float scalar) const;
        Vector4D operator*(float scalar) const;
        Vector4D operator/(float scalar) const;

        
        Vector4D& operator+=(const Vector4D& other);
        Vector4D& operator-=(const Vector4D& other);
        Vector4D& operator*=(const Vector4D& other);
        Vector4D& operator/=(const Vector4D& other);

        Vector4D& operator+=(float scalar);
        Vector4D& operator-=(float scalar);
        Vector4D& operator*=(float scalar);
        Vector4D& operator/=(float scalar);

         
        Vector4D operator-() const;
        Vector4D operator+() const { return *this; }

        
        float Length() const;
        float LengthSquared() const;
        float Distance(const Vector4D& other) const;
        float DistanceSquared(const Vector4D& other) const;
        float Dot(const Vector4D& other) const;

        Vector4D& Normalize();
        Vector4D Normalized() const;
        bool IsNormalized() const;

        
        Vector4D& Scale(const Vector4D& other);
        Vector4D Scaled(const Vector4D& other) const;

         
        static Vector4D Lerp(const Vector4D& a, const Vector4D& b, float t);
        static Vector4D Slerp(const Vector4D& a, const Vector4D& b, float t);

         
        Vector3D ToCartesian() const;
        static Vector4D FromCartesian(const Vector3D& cartesian, float w = 1.0f);

        
        bool IsZero() const;
        bool IsFinite() const;

         
        static Vector4D Zero();
        static Vector4D One();
        static Vector4D UnitX();
        static Vector4D UnitY();
        static Vector4D UnitZ();
        static Vector4D UnitW();

         
        static Vector4D Red();
        static Vector4D Green();
        static Vector4D Blue();
        static Vector4D Yellow();
        static Vector4D Cyan();
        static Vector4D Magenta();
        static Vector4D White();
        static Vector4D Black();
        static Vector4D Gray();
        static Vector4D Clear();  

         
        static Vector4D Min(const Vector4D& a, const Vector4D& b);
        static Vector4D Max(const Vector4D& a, const Vector4D& b);

         
        Vector4D& Clamp(const Vector4D& min, const Vector4D& max);
        Vector4D& Clamp01();  
        Vector4D& ClampMagnitude(float maxLength);
        static Vector4D Clamp(const Vector4D& value, const Vector4D& min, const Vector4D& max);
        static Vector4D Clamp01(const Vector4D& value);
        static Vector4D ClampMagnitude(const Vector4D& vector, float maxLength);

         
        Vector4D& Abs();
        Vector4D Abs() const;
        Vector4D& Sign();
        Vector4D Sign() const;

         
        Vector2D ToVector2D() const;
        Vector3D ToVector3D() const;
        Vector3D ToVector3DCartesian() const;

         
        Vector4D& Saturate();   
        Vector4D Saturated() const;
        float GetBrightness() const;  
        Vector4D& Invert();  
        Vector4D Inverted() const;

         
        float MinComponent() const;
        float MaxComponent() const;
        float Sum() const;
        float Average() const;

         
        float& Width() { return x; }
        float& Height() { return y; }
        float& Depth() { return z; }
        float& Time() { return w; }
        const float& Width() const { return x; }
        const float& Height() const { return y; }
        const float& Depth() const { return z; }
        const float& Time() const { return w; }

         
        float& R() { return x; }
        float& G() { return y; }
        float& B() { return z; }
        float& A() { return w; }
        const float& R() const { return x; }
        const float& G() const { return y; }
        const float& B() const { return z; }
        const float& A() const { return w; }

         
        float& U() { return x; }
        float& V() { return y; }
        float& S() { return z; }
        float& T() { return w; }
        const float& U() const { return x; }
        const float& V() const { return y; }
        const float& S() const { return z; }
        const float& T() const { return w; }
    };
    KE_API   std::ostream& operator<<(std::ostream& os, const Vector4D& vec);
    KE_API   std::istream& operator>>(std::istream& is, Vector4D& vec);
}