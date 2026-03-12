#include "Utils/Math/Color.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace CEMath
    {
        // ============================================================================
        // Constructors
        // ============================================================================

    Color::Color () : r ( 0.0f ), g ( 0.0f ), b ( 0.0f ), a ( 1.0f ) {}

    Color::Color ( float grayscale )
        : r ( grayscale ), g ( grayscale ), b ( grayscale ), a ( 1.0f ) {}

    Color::Color ( float inR, float inG, float inB, float inA )
        : r ( inR ), g ( inG ), b ( inB ), a ( inA ) {}

    Color::Color ( const Vector3D & rgb, float inA )
        : r ( rgb.x ), g ( rgb.y ), b ( rgb.z ), a ( inA ) {}

    Color::Color ( const Vector4D & rgba )
        : r ( rgba.x ), g ( rgba.y ), b ( rgba.z ), a ( rgba.w ) {}

    Color::Color ( const Color & other )
        : r ( other.r ), g ( other.g ), b ( other.b ), a ( other.a ) {}

    // ============================================================================
    // Assignment Operators
    // ============================================================================

    Color & Color::operator=( const Color & other )
        {
        if (this != &other)
            {
            r = other.r;
            g = other.g;
            b = other.b;
            a = other.a;
            }
        return *this;
        }

        // ============================================================================
        // Comparison Operators
        // ============================================================================

    bool Color::operator==( const Color & other ) const
        {
        return IsEqual ( r, other.r ) && IsEqual ( g, other.g ) &&
            IsEqual ( b, other.b ) && IsEqual ( a, other.a );
        }

    bool Color::operator!=( const Color & other ) const
        {
        return !( *this == other );
        }

        // ============================================================================
        // Arithmetic Operators
        // ============================================================================

    Color Color::operator+( const Color & other ) const
        {
        return Color ( r + other.r, g + other.g, b + other.b, a + other.a );
        }

    Color Color::operator-( const Color & other ) const
        {
        return Color ( r - other.r, g - other.g, b - other.b, a - other.a );
        }

    Color Color::operator*( const Color & other ) const
        {
        return Color ( r * other.r, g * other.g, b * other.b, a * other.a );
        }

    Color Color::operator/( const Color & other ) const
        {
        if (CEMath::IsZero ( other.r ) || CEMath::IsZero ( other.g ) || CEMath::IsZero ( other.b ) || CEMath::IsZero ( other.a ))
            {
            throw std::runtime_error ( "Color division by zero" );
            }
        return Color ( r / other.r, g / other.g, b / other.b, a / other.a );
        }

    Color Color::operator+( float scalar ) const
        {
        return Color ( r + scalar, g + scalar, b + scalar, a + scalar );
        }

    Color Color::operator-( float scalar ) const
        {
        return Color ( r - scalar, g - scalar, b - scalar, a - scalar );
        }

    Color Color::operator*( float scalar ) const
        {
        return Color ( r * scalar, g * scalar, b * scalar, a * scalar );
        }

    Color Color::operator/( float scalar ) const
        {
        if (CEMath::IsZero ( scalar ))
            {
            throw std::runtime_error ( "Color division by zero" );
            }
        float invScalar = 1.0f / scalar;
        return Color ( r * invScalar, g * invScalar, b * invScalar, a * invScalar );
        }

        // ============================================================================
        // Compound Assignment Operators
        // ============================================================================

    Color & Color::operator+=( const Color & other )
        {
        r += other.r;
        g += other.g;
        b += other.b;
        a += other.a;
        return *this;
        }

    Color & Color::operator-=( const Color & other )
        {
        r -= other.r;
        g -= other.g;
        b -= other.b;
        a -= other.a;
        return *this;
        }

    Color & Color::operator*=( const Color & other )
        {
        r *= other.r;
        g *= other.g;
        b *= other.b;
        a *= other.a;
        return *this;
        }

    Color & Color::operator/=( const Color & other )
        {
        if (CEMath::IsZero ( other.r ) || CEMath::IsZero ( other.g ) || CEMath::IsZero ( other.b ) || CEMath::IsZero ( other.a ))
            {
            throw std::runtime_error ( "Color division by zero" );
            }
        r /= other.r;
        g /= other.g;
        b /= other.b;
        a /= other.a;
        return *this;
        }

    Color & Color::operator+=( float scalar )
        {
        r += scalar;
        g += scalar;
        b += scalar;
        a += scalar;
        return *this;
        }

    Color & Color::operator-=( float scalar )
        {
        r -= scalar;
        g -= scalar;
        b -= scalar;
        a -= scalar;
        return *this;
        }

    Color & Color::operator*=( float scalar )
        {
        r *= scalar;
        g *= scalar;
        b *= scalar;
        a *= scalar;
        return *this;
        }

    Color & Color::operator/=( float scalar )
        {
        if (CEMath::IsZero ( scalar ))
            {
            throw std::runtime_error ( "Color division by zero" );
            }
        float invScalar = 1.0f / scalar;
        r *= invScalar;
        g *= invScalar;
        b *= invScalar;
        a *= invScalar;
        return *this;
        }

        // ============================================================================
        // Unary Operators
        // ============================================================================

    Color Color::operator-() const
        {
        return Color ( -r, -g, -b, -a );
        }

        // ============================================================================
        // Color Operations
        // ============================================================================

    float Color::GetBrightness () const
        {
            // Perceived brightness (standard formula)
        return 0.299f * r + 0.587f * g + 0.114f * b;
        }

    float Color::GetLuminance () const
        {
            // Standard luminance (Rec. 709)
        return 0.2126f * r + 0.7152f * g + 0.0722f * b;
        }

    Color & Color::Saturate ()
        {
        r = Clamp ( r, 0.0f, 1.0f );
        g = Clamp ( g, 0.0f, 1.0f );
        b = Clamp ( b, 0.0f, 1.0f );
        a = Clamp ( a, 0.0f, 1.0f );
        return *this;
        }

    Color Color::Saturated () const
        {
        Color result = *this;
        result.Saturate ();
        return result;
        }

    Color & Color::Invert ()
        {
        r = 1.0f - r;
        g = 1.0f - g;
        b = 1.0f - b;
        // Alpha remains unchanged
        return *this;
        }

    Color Color::Inverted () const
        {
        Color result = *this;
        result.Invert ();
        return result;
        }

    Color & Color::Grayscale ()
        {
        float gray = GetBrightness ();
        r = gray;
        g = gray;
        b = gray;
        return *this;
        }

    Color Color::Grayscale () const
        {
        Color result = *this;
        result.Grayscale ();
        return result;
        }

    Color & Color::Lerp ( const Color & other, float t )
        {
        float clampedT = Clamp ( t, 0.0f, 1.0f );
        r = r + ( other.r - r ) * clampedT;
        g = g + ( other.g - g ) * clampedT;
        b = b + ( other.b - b ) * clampedT;
        a = a + ( other.a - a ) * clampedT;
        return *this;
        }

    Color Color::Lerp ( const Color & a, const Color & b, float t )
        {
        Color result = a;
        result.Lerp ( b, t );
        return result;
        }

        // ============================================================================
        // Conversion Operators
        // ============================================================================

    Vector3D Color::ToVector3D () const
        {
        return Vector3D ( r, g, b );
        }

    Vector4D Color::ToVector4D () const
        {
        return Vector4D ( r, g, b, a );
        }

    uint32 Color::ToHex () const
        {
        uint32 ri = static_cast< uint32 >( Clamp ( r, 0.0f, 1.0f ) * 255.0f );
        uint32 gi = static_cast< uint32 >( Clamp ( g, 0.0f, 1.0f ) * 255.0f );
        uint32 bi = static_cast< uint32 >( Clamp ( b, 0.0f, 1.0f ) * 255.0f );
        uint32 ai = static_cast< uint32 >( Clamp ( a, 0.0f, 1.0f ) * 255.0f );

        return ( ri << 24 ) | ( gi << 16 ) | ( bi << 8 ) | ai;
        }

    Color Color::FromHex ( uint32 hex )
        {
        float r = ( ( hex >> 24 ) & 0xFF ) / 255.0f;
        float g = ( ( hex >> 16 ) & 0xFF ) / 255.0f;
        float b = ( ( hex >> 8 ) & 0xFF ) / 255.0f;
        float a = ( hex & 0xFF ) / 255.0f;
        return Color ( r, g, b, a );
        }

        // ============================================================================
        // Validation
        // ============================================================================

    bool Color::IsZero () const
        {
        return CEMath::IsZero ( r ) && CEMath::IsZero ( g ) && CEMath::IsZero ( b ) && CEMath::IsZero ( a );
        }

    bool Color::IsOne () const
        {
        return IsEqual ( r, 1.0f ) && IsEqual ( g, 1.0f ) &&
            IsEqual ( b, 1.0f ) && IsEqual ( a, 1.0f );
        }

    bool Color::IsValid () const
        {
        return r >= 0.0f && r <= 1.0f &&
            g >= 0.0f && g <= 1.0f &&
            b >= 0.0f && b <= 1.0f &&
            a >= 0.0f && a <= 1.0f;
        }

        // ============================================================================
        // Static Constants
        // ============================================================================

    Color Color::White () { return Color ( 1.00f, 1.00f, 1.00f, 1.0f ); }
    Color Color::Black () { return Color ( 0.00f, 0.00f, 0.00f, 1.0f ); }
    Color Color::Red () { return Color ( 1.00f, 0.00f, 0.00f, 1.0f ); }
    Color Color::Green () { return Color ( 0.00f, 1.00f, 0.00f, 1.0f ); }
    Color Color::Blue () { return Color ( 0.00f, 0.00f, 1.00f, 1.0f ); }
    Color Color::Yellow () { return Color ( 1.00f, 1.00f, 0.00f, 1.0f ); }
    Color Color::Cyan () { return Color ( 0.00f, 1.00f, 1.00f, 1.0f ); }
    Color Color::Magenta () { return Color ( 1.00f, 0.00f, 1.00f, 1.0f ); }
    Color Color::Orange () { return Color ( 1.00f, 0.65f, 0.00f, 1.0f ); }
    Color Color::Purple () { return Color ( 0.50f, 0.00f, 0.50f, 1.0f ); }
    Color Color::Pink () { return Color ( 1.00f, 0.75f, 0.80f, 1.0f ); }
    Color Color::Brown () { return Color ( 0.65f, 0.16f, 0.16f, 1.0f ); }
    Color Color::Gray () { return Color ( 0.50f, 0.50f, 0.50f, 1.0f ); }
    Color Color::LightGray () { return Color ( 0.83f, 0.83f, 0.83f, 1.0f ); }
    Color Color::DarkGray () { return Color ( 0.33f, 0.33f, 0.33f, 1.0f ); }
    Color Color::Transparent () { return Color ( 0.00f, 0.00f, 0.00f, 0.0f ); }

    // ============================================================================
    // Stream Operators
    // ============================================================================

    std::ostream & operator<<( std::ostream & os, const Color & color )
        {
        os << "RGBA(" << color.r << ", " << color.g << ", "
            << color.b << ", " << color.a << ")";
        return os;
        }

    std::istream & operator>>( std::istream & is, Color & color )
        {
        char open, comma1, comma2, comma3, close;
        std::string prefix;

        // Try to read with prefix
        if (is >> prefix && prefix == "RGBA(")
            {
            is >> color.r >> comma1 >> color.g >> comma2
                >> color.b >> comma3 >> color.a >> close;
            }
        else
            {
                // Fallback to simple format
            is.clear ();
            is.seekg ( -static_cast< int >( prefix.length () ), std::ios_base::cur );
            is >> color.r >> color.g >> color.b >> color.a;
            }

        return is;
        }

        // ============================================================================
        // Global Operators
        // ============================================================================

    Color operator*( float scalar, const Color & color )
        {
        return color * scalar;
        }
    }