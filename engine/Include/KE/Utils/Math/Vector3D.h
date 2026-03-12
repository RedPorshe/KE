#pragma once
#include "Core/KEExport.h"
#include <cmath>
#include <stdexcept>
#include <iostream>
#include "Utils/Math/CE_MathConstants.h"
#include "Utils/Math/CE_MathHelpers.h"
#include "Vector2D.h"

namespace CEMath
    {
    struct KE_API Vector3D
        {
        float x {}, y {}, z {};

        // ============================================================================
        // Constructors
        // ============================================================================

        Vector3D ();
        explicit Vector3D ( float scalar );
        Vector3D ( float inX, float inY, float inZ );
        Vector3D ( const Vector2D & vec2, float z = 0.0f );
        Vector3D ( const Vector3D & other );

        // ============================================================================
        // Assignment Operators
        // ============================================================================

        Vector3D & operator=( const Vector3D & other );

        // ============================================================================
        // Comparison Operators
        // ============================================================================

        bool operator==( const Vector3D & other ) const;
        bool operator!=( const Vector3D & other ) const;

        // ============================================================================
        // Arithmetic Operators (Vector-Vector)
        // ============================================================================

        Vector3D operator+( const Vector3D & other ) const;
        Vector3D operator-( const Vector3D & other ) const;
        Vector3D operator*( const Vector3D & other ) const;
        Vector3D operator/( const Vector3D & other ) const;

        // ============================================================================
        // Arithmetic Operators (Vector-Scalar)
        // ============================================================================

        Vector3D operator+( float scalar ) const;
        Vector3D operator-( float scalar ) const;
        Vector3D operator*( float scalar ) const;
        Vector3D operator/( float scalar ) const;

        // ============================================================================
        // Compound Assignment Operators (Vector-Vector)
        // ============================================================================

        Vector3D & operator+=( const Vector3D & other );
        Vector3D & operator-=( const Vector3D & other );
        Vector3D & operator*=( const Vector3D & other );
        Vector3D & operator/=( const Vector3D & other );

        // ============================================================================
        // Compound Assignment Operators (Vector-Scalar)
        // ============================================================================

        Vector3D & operator+=( float scalar );
        Vector3D & operator-=( float scalar );
        Vector3D & operator*=( float scalar );
        Vector3D & operator/=( float scalar );

        // ============================================================================
        // Index Operators
        // ============================================================================

        float operator[]( int index ) const;           // const версия для чтения
        float & operator[]( int index );                 // не-const версия для записи

        // ============================================================================
        // Unary Operators
        // ============================================================================

        Vector3D operator-() const;
        Vector3D operator+() const { return *this; }

        // ============================================================================
        // Basic Properties
        // ============================================================================

        float Length () const;
        float LengthSquared () const;
        float Distance ( const Vector3D & other ) const;
        float DistanceSquared ( const Vector3D & other ) const;
        float Dot ( const Vector3D & other ) const;
        Vector3D Cross ( const Vector3D & other ) const;

        // ============================================================================
        // Normalization
        // ============================================================================

        Vector3D & Normalize ();
        Vector3D Normalized () const;
        bool IsNormalized () const;

        // ============================================================================
        // Scaling
        // ============================================================================

        Vector3D & Scale ( const Vector3D & other );
        Vector3D Scaled ( const Vector3D & other ) const;

        // ============================================================================
        // Projections and Reflections
        // ============================================================================

        Vector3D Project ( const Vector3D & normal ) const;
        Vector3D ProjectOnPlane ( const Vector3D & planeNormal ) const;
        Vector3D Reflect ( const Vector3D & normal ) const;

        // ============================================================================
        // Interpolation
        // ============================================================================

        static Vector3D Lerp ( const Vector3D & a, const Vector3D & b, float t );
        static Vector3D Slerp ( const Vector3D & a, const Vector3D & b, float t );

        // ============================================================================
        // Angles and Rotation
        // ============================================================================

        static float Angle ( const Vector3D & a, const Vector3D & b );
        static Vector3D RotateTowards ( const Vector3D & current, const Vector3D & target, float maxRadiansDelta );

        // ============================================================================
        // Utility Methods
        // ============================================================================

        bool IsZero () const;
        bool IsFinite () const;
        bool IsEqual ( const Vector3D & other, float epsilon = EPSILON ) const;

        // ============================================================================
        // Static Constants
        // ============================================================================

        static Vector3D Zero ();
        static Vector3D One ();
        static Vector3D UnitX ();
        static Vector3D UnitY ();
        static Vector3D UnitZ ();

        // ============================================================================
        // Direction Vectors
        // ============================================================================

        static Vector3D Left ();
        static Vector3D Right ();
        static Vector3D Up ();
        static Vector3D Down ();
        static Vector3D Forward ();
        static Vector3D Back ();

        // ============================================================================
        // Min/Max Operations
        // ============================================================================

        static Vector3D Min ( const Vector3D & a, const Vector3D & b );
        static Vector3D Max ( const Vector3D & a, const Vector3D & b );

        // ============================================================================
        // Clamping
        // ============================================================================

        Vector3D & Clamp ( const Vector3D & min, const Vector3D & max );
        Vector3D & ClampMagnitude ( float maxLength );
        static Vector3D Clamp ( const Vector3D & value, const Vector3D & min, const Vector3D & max );
        static Vector3D ClampMagnitude ( const Vector3D & vector, float maxLength );

        // ============================================================================
        // Absolute and Sign
        // ============================================================================

        Vector3D & Abs ();
        Vector3D Abs () const;
        Vector3D & Sign ();
        Vector3D Sign () const;

        // ============================================================================
        // Conversion to 2D
        // ============================================================================

        Vector2D ToVector2D () const;
        Vector2D ToVector2DXY () const;
        Vector2D ToVector2DXZ () const;
        Vector2D ToVector2DYZ () const;

        // ============================================================================
        // Component Statistics
        // ============================================================================

        float MinComponent () const;
        float MaxComponent () const;
        float Sum () const;
        float Average () const;

        // ============================================================================
        // Geometric Distance Calculations
        // ============================================================================

        float DistanceToLine ( const Vector3D & linePoint, const Vector3D & lineDirection ) const;
        float DistanceToPlane ( const Vector3D & planePoint, const Vector3D & planeNormal ) const;

        // ============================================================================
        // Alternative Accessors (for different interpretations)
        // ============================================================================

        // As dimensions
        float & Width () { return x; }
        float & Height () { return y; }
        float & Depth () { return z; }
        const float & Width () const { return x; }
        const float & Height () const { return y; }
        const float & Depth () const { return z; }

        // As RGB color
        float & R () { return x; }
        float & G () { return y; }
        float & B () { return z; }
        const float & R () const { return x; }
        const float & G () const { return y; }
        const float & B () const { return z; }

        // As UVW texture coordinates
        float & U () { return x; }
        float & V () { return y; }
        float & W () { return z; }
        const float & U () const { return x; }
        const float & V () const { return y; }
        const float & W () const { return z; }
        };

        // ============================================================================
        // Stream Operators
        // ============================================================================

        KE_API  std::ostream & operator<<( std::ostream & os, const Vector3D & vec );
        KE_API   std::istream & operator>>( std::istream & is, Vector3D & vec );

    // ============================================================================
    // Global Utility Functions
    // ============================================================================

    inline bool IsEqual ( const Vector3D & a, const Vector3D & b, float epsilon = EPSILON )
        {
        return a.IsEqual ( b, epsilon );
        }

        // Vector3D-dependent helper function from CE_MathHelpers
    inline Vector3D NormalizeAnglesDegrees ( const Vector3D & angles )
        {
        auto NormalizeDeg = [] ( float degrees ) -> float
            {
            degrees = std::fmod ( degrees, 360.0f );
            if (degrees < 0.0f)
                degrees += 360.0f;

            // Snap to exact values if very close (within epsilon)
            // Using larger epsilon (0.1°) to catch floating-point rounding errors
            const float EPSILON = 0.1f;
            if (std::fabs ( degrees - 90.0f ) < EPSILON) degrees = 90.0f;
            if (std::fabs ( degrees - 180.0f ) < EPSILON) degrees = 180.0f;
            if (std::fabs ( degrees - 270.0f ) < EPSILON) degrees = 270.0f;
            if (std::fabs ( degrees - 360.0f ) < EPSILON) degrees = 0.0f;
            if (std::fabs ( degrees ) < EPSILON) degrees = 0.0f;

            return degrees;
            };

        return Vector3D (
            NormalizeDeg ( angles.x ),
            NormalizeDeg ( angles.y ),
            NormalizeDeg ( angles.z )
        );
        }
    }