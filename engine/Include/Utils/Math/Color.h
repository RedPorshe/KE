#pragma once
#include "Core/KEExport.h"
#include "Utils/Math/Vector3D.h"
#include "Utils/Math/Vector4.h"
#include "Utils/Math/MathTypes.h"



namespace CEMath
    {
    struct KE_API Color
        {
        float r {}, g {}, b {}, a { 1.0f };  // a - альфа/интенсивность (0-1)

        // ============================================================================
        // Constructors
        // ============================================================================

        Color ();
        explicit Color ( float grayscale );
        Color ( float inR, float inG, float inB, float inA = 1.0f );
        Color ( const Vector3D & rgb, float inA = 1.0f );
        Color ( const Vector4D & rgba );
        Color ( const Color & other );

        // ============================================================================
        // Assignment Operators
        // ============================================================================

        Color & operator=( const Color & other );

        // ============================================================================
        // Comparison Operators
        // ============================================================================

        bool operator==( const Color & other ) const;
        bool operator!=( const Color & other ) const;

        // ============================================================================
        // Arithmetic Operators
        // ============================================================================

        Color operator+( const Color & other ) const;
        Color operator-( const Color & other ) const;
        Color operator*( const Color & other ) const;  // component-wise multiplication
        Color operator/( const Color & other ) const;

        Color operator+( float scalar ) const;
        Color operator-( float scalar ) const;
        Color operator*( float scalar ) const;
        Color operator/( float scalar ) const;

        Color & operator+=( const Color & other );
        Color & operator-=( const Color & other );
        Color & operator*=( const Color & other );
        Color & operator/=( const Color & other );

        Color & operator+=( float scalar );
        Color & operator-=( float scalar );
        Color & operator*=( float scalar );
        Color & operator/=( float scalar );

        // ============================================================================
        // Unary Operators
        // ============================================================================

        Color operator-() const;
        Color operator+() const { return *this; }

        // ============================================================================
        // Color Operations
        // ============================================================================

        float GetBrightness () const;          // Perceived brightness (0-1)
        float GetLuminance () const;            // Standard luminance: 0.2126*R + 0.7152*G + 0.0722*B
        Color & Saturate ();                     // Clamp between 0 and 1
        Color Saturated () const;
        Color & Invert ();                        // RGB inversion (1 - r, 1 - g, 1 - b)
        Color Inverted () const;
        Color & Grayscale ();                     // Convert to grayscale
        Color Grayscale () const;
        Color & Lerp ( const Color & other, float t );
        static Color Lerp ( const Color & a, const Color & b, float t );

        // ============================================================================
        // Conversion Operators
        // ============================================================================

        Vector3D ToVector3D () const;            // RGB only
        Vector4D ToVector4D () const;            // RGBA
        uint32_t ToHex () const;                  // 0xRRGGBBAA (используем uint32_t)
        static Color FromHex ( uint32_t hex );

        // ============================================================================
        // Validation
        // ============================================================================

        bool IsZero () const;
        bool IsOne () const;
        bool IsValid () const;                    // All components between 0 and 1

        // ============================================================================
        // Static Constants - УБИРАЕМ ДУБЛИКАТЫ!
        // ============================================================================

        static Color White ();
        static Color Black ();
        static Color Red ();
        static Color Green ();
        static Color Blue ();
        static Color Yellow ();
        static Color Cyan ();
        static Color Magenta ();
        static Color Orange ();
        static Color Purple ();
        static Color Pink ();
        static Color Brown ();
        static Color Gray ();
        static Color LightGray ();
        static Color DarkGray ();
        static Color Transparent ();
        static Color Clear () { return Transparent (); }

        // Дополнительные цвета (не дублируем основные)
        static Color AliceBlue ();
        static Color AntiqueWhite ();
        static Color Aqua ();
        static Color Aquamarine ();
        static Color Azure ();
        static Color Beige ();
        static Color Bisque ();
        static Color BlanchedAlmond ();
        static Color BlueViolet ();
        static Color BurlyWood ();
        static Color CadetBlue ();
        static Color Chartreuse ();
        static Color Chocolate ();
        static Color Coral ();
        static Color CornflowerBlue ();
        static Color Cornsilk ();
        static Color Crimson ();
        static Color DarkBlue ();
        static Color DarkCyan ();
        static Color DarkGoldenRod ();
        static Color DarkGreen ();
        static Color DarkKhaki ();
        static Color DarkMagenta ();
        static Color DarkOliveGreen ();
        static Color DarkOrchid ();
        static Color DarkRed ();
        static Color DarkSalmon ();
        static Color DarkSeaGreen ();
        static Color DarkSlateBlue ();
        static Color DarkSlateGray ();
        static Color DarkTurquoise ();
        static Color DarkViolet ();
        static Color DeepPink ();
        static Color DeepSkyBlue ();
        static Color DimGray ();
        static Color DodgerBlue ();
        static Color FireBrick ();
        static Color FloralWhite ();
        static Color ForestGreen ();
        static Color Fuchsia ();
        static Color Gainsboro ();
        static Color GhostWhite ();
        static Color Gold ();
        static Color GoldenRod ();
        static Color GreenYellow ();
        static Color HoneyDew ();
        static Color HotPink ();
        static Color IndianRed ();
        static Color Indigo ();
        static Color Ivory ();
        static Color Khaki ();
        static Color Lavender ();
        static Color LavenderBlush ();
        static Color LawnGreen ();
        static Color LemonChiffon ();
        static Color LightBlue ();
        static Color LightCoral ();
        static Color LightCyan ();
        static Color LightGoldenRodYellow ();
        static Color LightGreen ();
        static Color LightPink ();
        static Color LightSalmon ();
        static Color LightSeaGreen ();
        static Color LightSkyBlue ();
        static Color LightSlateGray ();
        static Color LightSteelBlue ();
        static Color LightYellow ();
        static Color Lime ();
        static Color LimeGreen ();
        static Color Linen ();
        static Color Maroon ();
        static Color MediumAquaMarine ();
        static Color MediumBlue ();
        static Color MediumOrchid ();
        static Color MediumPurple ();
        static Color MediumSeaGreen ();
        static Color MediumSlateBlue ();
        static Color MediumSpringGreen ();
        static Color MediumTurquoise ();
        static Color MediumVioletRed ();
        static Color MidnightBlue ();
        static Color MintCream ();
        static Color MistyRose ();
        static Color Moccasin ();
        static Color NavajoWhite ();
        static Color Navy ();
        static Color OldLace ();
        static Color Olive ();
        static Color OliveDrab ();
        static Color OrangeRed ();
        static Color Orchid ();
        static Color PaleGoldenRod ();
        static Color PaleGreen ();
        static Color PaleTurquoise ();
        static Color PaleVioletRed ();
        static Color PapayaWhip ();
        static Color PeachPuff ();
        static Color Peru ();
        static Color Plum ();
        static Color PowderBlue ();
        static Color RebeccaPurple ();
        static Color RosyBrown ();
        static Color RoyalBlue ();
        static Color SaddleBrown ();
        static Color Salmon ();
        static Color SandyBrown ();
        static Color SeaGreen ();
        static Color SeaShell ();
        static Color Sienna ();
        static Color Silver ();
        static Color SkyBlue ();
        static Color SlateBlue ();
        static Color SlateGray ();
        static Color Snow ();
        static Color SpringGreen ();
        static Color SteelBlue ();
        static Color Tan ();
        static Color Teal ();
        static Color Thistle ();
        static Color Tomato ();
        static Color Turquoise ();
        static Color Violet ();
        static Color Wheat ();
        static Color WhiteSmoke ();
        static Color YellowGreen ();

        // Accessors as RGB
        float & R () { return r; }
        float & G () { return g; }
        float & B () { return b; }
        float & A () { return a; }
        const float & R () const { return r; }
        const float & G () const { return g; }
        const float & B () const { return b; }
        const float & A () const { return a; }
        };

        // ============================================================================
        // Stream Operators
        // ============================================================================

    KE_API std::ostream & operator<<( std::ostream & os, const Color & color );
    KE_API std::istream & operator>>( std::istream & is, Color & color );

    // ============================================================================
    // Global Operators
    // ============================================================================

    KE_API Color operator*( float scalar, const Color & color );
    }