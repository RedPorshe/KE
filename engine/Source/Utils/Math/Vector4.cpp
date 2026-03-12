#include "Utils/Math/Vector4.h"
#include <iostream>
#include <iomanip>

namespace CEMath
	{
		// Конструкторы
	Vector4D::Vector4D () : x ( 0.f ), y ( 0.f ), z ( 0.f ), w ( 0.f ) {}

	Vector4D::Vector4D ( float scalar ) : x ( scalar ), y ( scalar ), z ( scalar ), w ( scalar ) {}

	Vector4D::Vector4D ( float inX, float inY, float inZ, float inW ) :x ( inX ), y ( inY ), z ( inZ ), w ( inW )
		{}

	Vector4D::Vector4D ( int inX, int inY, int inZ, int inW ) :
		x ( static_cast< float >( inX ) ), y ( static_cast< float >( inY ) ),
		z ( static_cast< float >( inZ ) ), w ( static_cast< float >( inW ) ) {}



	Vector4D::Vector4D ( const Vector2D & vec2, float z, float w ) :
		x ( vec2.x ), y ( vec2.y ), z ( z ), w ( w ) {}

	Vector4D::Vector4D ( const Vector3D & vec3, float w ) :
		x ( vec3.x ), y ( vec3.y ), z ( vec3.z ), w ( w ) {}

	Vector4D::Vector4D ( const Vector4D & other ) :
		x ( other.x ), y ( other.y ), z ( other.z ), w ( other.w ) {}

	// Операторы присваивания
	Vector4D & Vector4D::operator=( const Vector4D & other )
		{
		if (this != &other)
			{
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
			}
		return *this;
		}

		// Операторы сравнения
	bool Vector4D::operator==( const Vector4D & other ) const
		{
		return IsEqual ( x, other.x ) && IsEqual ( y, other.y ) &&
			IsEqual ( z, other.z ) && IsEqual ( w, other.w );
		}

	bool Vector4D::operator!=( const Vector4D & other ) const
		{
		return !( *this == other );
		}

		// Арифметические операторы
	Vector4D Vector4D::operator+( const Vector4D & other ) const
		{
		return Vector4D ( x + other.x, y + other.y, z + other.z, w + other.w );
		}

	Vector4D Vector4D::operator-( const Vector4D & other ) const
		{
		return Vector4D ( x - other.x, y - other.y, z - other.z, w - other.w );
		}

	Vector4D Vector4D::operator*( const Vector4D & other ) const
		{
		return Vector4D ( x * other.x, y * other.y, z * other.z, w * other.w );
		}

	Vector4D Vector4D::operator/( const Vector4D & other ) const
		{
		if (CEMath::IsZero ( other.x ) || CEMath::IsZero ( other.y ) ||
			 CEMath::IsZero ( other.z ) || CEMath::IsZero ( other.w ))
			{
			throw std::runtime_error ( "Vector4: Division by zero" );
			}
		return Vector4D ( x / other.x, y / other.y, z / other.z, w / other.w );
		}

	Vector4D Vector4D::operator+( float scalar ) const
		{
		return Vector4D ( x + scalar, y + scalar, z + scalar, w + scalar );
		}

	Vector4D Vector4D::operator-( float scalar ) const
		{
		return Vector4D ( x - scalar, y - scalar, z - scalar, w - scalar );
		}

	Vector4D Vector4D::operator*( float scalar ) const
		{
		return Vector4D ( x * scalar, y * scalar, z * scalar, w * scalar );
		}

	Vector4D Vector4D::operator/( float scalar ) const
		{
		if (CEMath::IsZero ( scalar ))
			{
			throw std::runtime_error ( "Vector4: Division by zero" );
			}
		float invScalar = 1.0f / scalar;
		return Vector4D ( x * invScalar, y * invScalar, z * invScalar, w * invScalar );
		}

		// Составные операторы присваивания
	Vector4D & Vector4D::operator+=( const Vector4D & other )
		{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
		}

	Vector4D & Vector4D::operator-=( const Vector4D & other )
		{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
		}

	Vector4D & Vector4D::operator*=( const Vector4D & other )
		{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;
		return *this;
		}

	Vector4D & Vector4D::operator/=( const Vector4D & other )
		{
		if (CEMath::IsZero ( other.x ) || CEMath::IsZero ( other.y ) ||
			 CEMath::IsZero ( other.z ) || CEMath::IsZero ( other.w ))
			{
			throw std::runtime_error ( "Vector4: Division by zero" );
			}
		x /= other.x;
		y /= other.y;
		z /= other.z;
		w /= other.w;
		return *this;
		}

	Vector4D & Vector4D::operator+=( float scalar )
		{
		x += scalar;
		y += scalar;
		z += scalar;
		w += scalar;
		return *this;
		}

	Vector4D & Vector4D::operator-=( float scalar )
		{
		x -= scalar;
		y -= scalar;
		z -= scalar;
		w -= scalar;
		return *this;
		}

	Vector4D & Vector4D::operator*=( float scalar )
		{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;
		return *this;
		}

	Vector4D & Vector4D::operator/=( float scalar )
		{
		if (CEMath::IsZero ( scalar ))
			{
			throw std::runtime_error ( "Vector4: Division by zero" );
			}
		float invScalar = 1.0f / scalar;
		x *= invScalar;
		y *= invScalar;
		z *= invScalar;
		w *= invScalar;
		return *this;
		}

		// Унарные операторы
	Vector4D Vector4D::operator-() const
		{
		return Vector4D ( -x, -y, -z, -w );
		}

		// Базовые математические функции
	float Vector4D::Length () const
		{
		return Sqrt ( x * x + y * y + z * z + w * w );
		}

	float Vector4D::LengthSquared () const
		{
		return x * x + y * y + z * z + w * w;
		}

	float Vector4D::Distance ( const Vector4D & other ) const
		{
		float dx = x - other.x;
		float dy = y - other.y;
		float dz = z - other.z;
		float dw = w - other.w;
		return Sqrt ( dx * dx + dy * dy + dz * dz + dw * dw );
		}

	float Vector4D::DistanceSquared ( const Vector4D & other ) const
		{
		float dx = x - other.x;
		float dy = y - other.y;
		float dz = z - other.z;
		float dw = w - other.w;
		return dx * dx + dy * dy + dz * dz + dw * dw;
		}

	float Vector4D::Dot ( const Vector4D & other ) const
		{
		return x * other.x + y * other.y + z * other.z + w * other.w;
		}

	Vector4D & Vector4D::Normalize ()
		{
		float len = Length ();
		if (CEMath::IsZero ( len ))
			{
			throw std::runtime_error ( "Vector4: Cannot normalize zero vector" );
			}
		float invLen = 1.0f / len;
		x *= invLen;
		y *= invLen;
		z *= invLen;
		w *= invLen;
		return *this;
		}

	Vector4D Vector4D::Normalized () const
		{
		Vector4D result = *this;
		result.Normalize ();
		return result;
		}

	bool Vector4D::IsNormalized () const
		{
		return IsEqual ( LengthSquared (), 1.0f );
		}

		// Геометрические операции
	Vector4D & Vector4D::Scale ( const Vector4D & other )
		{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;
		return *this;
		}

	Vector4D Vector4D::Scaled ( const Vector4D & other ) const
		{
		Vector4D result = *this;
		result.Scale ( other );
		return result;
		}

		// Линейная интерполяция
	Vector4D Vector4D::Lerp ( const Vector4D & a, const Vector4D & b, float t )
		{
		float clampedT = CEMath::Clamp ( t, 0.0f, 1.0f );
		return Vector4D (
			a.x + ( b.x - a.x ) * clampedT,
			a.y + ( b.y - a.y ) * clampedT,
			a.z + ( b.z - a.z ) * clampedT,
			a.w + ( b.w - a.w ) * clampedT
		);
		}

	Vector4D Vector4D::Slerp ( const Vector4D & a, const Vector4D & b, float t )
		{
		float clampedT = CEMath::Clamp ( t, 0.0f, 1.0f );
		float dot = CEMath::Clamp ( a.Dot ( b ), -1.0f, 1.0f );
		float theta = Acos ( dot ) * clampedT;

		Vector4D relativeVec = b - a * dot;
		if (relativeVec.LengthSquared () > EPSILON_SQUARED)
			relativeVec.Normalize ();
		else
			relativeVec = Vector4D ( -a.y, a.x, -a.w, a.z ); // Перпендикуляр

		return a * Cos ( theta ) + relativeVec * Sin ( theta );
		}

		// Однородные координаты
	Vector3D Vector4D::ToCartesian () const
		{
		if (CEMath::IsZero ( w ))
			return Vector3D ( x, y, z );

		float invW = 1.0f / w;
		return Vector3D ( x * invW, y * invW, z * invW );
		}

	Vector4D Vector4D::FromCartesian ( const Vector3D & cartesian, float w )
		{
		return Vector4D ( cartesian.x * w, cartesian.y * w, cartesian.z * w, w );
		}

		// Утилиты
	bool Vector4D::IsZero () const
		{
		return LengthSquared () < EPSILON_SQUARED;
		}

	bool Vector4D::IsFinite () const
		{
		return std::isfinite ( x ) && std::isfinite ( y ) &&
			std::isfinite ( z ) && std::isfinite ( w );
		}

		// Статические константы
	Vector4D Vector4D::Zero () { return Vector4D ( 0.0f, 0.0f, 0.0f, 0.0f ); }
	Vector4D Vector4D::One () { return Vector4D ( 1.0f, 1.0f, 1.0f, 1.0f ); }
	Vector4D Vector4D::UnitX () { return Vector4D ( 1.0f, 0.0f, 0.0f, 0.0f ); }
	Vector4D Vector4D::UnitY () { return Vector4D ( 0.0f, 1.0f, 0.0f, 0.0f ); }
	Vector4D Vector4D::UnitZ () { return Vector4D ( 0.0f, 0.0f, 1.0f, 0.0f ); }
	Vector4D Vector4D::UnitW () { return Vector4D ( 0.0f, 0.0f, 0.0f, 1.0f ); }

	// Цветовые константы
	Vector4D Vector4D::Red () { return Vector4D ( 1.0f, 0.0f, 0.0f, 1.0f ); }
	Vector4D Vector4D::Green () { return Vector4D ( 0.0f, 1.0f, 0.0f, 1.0f ); }
	Vector4D Vector4D::Blue () { return Vector4D ( 0.0f, 0.0f, 1.0f, 1.0f ); }
	Vector4D Vector4D::Yellow () { return Vector4D ( 1.0f, 1.0f, 0.0f, 1.0f ); }
	Vector4D Vector4D::Cyan () { return Vector4D ( 0.0f, 1.0f, 1.0f, 1.0f ); }
	Vector4D Vector4D::Magenta () { return Vector4D ( 1.0f, 0.0f, 1.0f, 1.0f ); }
	Vector4D Vector4D::White () { return Vector4D ( 1.0f, 1.0f, 1.0f, 1.0f ); }
	Vector4D Vector4D::Black () { return Vector4D ( 0.0f, 0.0f, 0.0f, 1.0f ); }
	Vector4D Vector4D::Gray () { return Vector4D ( 0.5f, 0.5f, 0.5f, 1.0f ); }
	Vector4D Vector4D::Clear () { return Vector4D ( 0.0f, 0.0f, 0.0f, 0.0f ); }

	// Минимум/максимум
	Vector4D Vector4D::Min ( const Vector4D & a, const Vector4D & b )
		{
		return Vector4D (
			std::min ( a.x, b.x ),
			std::min ( a.y, b.y ),
			std::min ( a.z, b.z ),
			std::min ( a.w, b.w )
		);
		}

	Vector4D Vector4D::Max ( const Vector4D & a, const Vector4D & b )
		{
		return Vector4D (
			std::max ( a.x, b.x ),
			std::max ( a.y, b.y ),
			std::max ( a.z, b.z ),
			std::max ( a.w, b.w )
		);
		}

		// Ограничение значений
	Vector4D & Vector4D::Clamp ( const Vector4D & min, const Vector4D & max )
		{
		x = CEMath::Clamp ( x, min.x, max.x );
		y = CEMath::Clamp ( y, min.y, max.y );
		z = CEMath::Clamp ( z, min.z, max.z );
		w = CEMath::Clamp ( w, min.w, max.w );
		return *this;
		}

	Vector4D & Vector4D::Clamp01 ()
		{
		x = CEMath::Clamp ( x, 0.0f, 1.0f );
		y = CEMath::Clamp ( y, 0.0f, 1.0f );
		z = CEMath::Clamp ( z, 0.0f, 1.0f );
		w = CEMath::Clamp ( w, 0.0f, 1.0f );
		return *this;
		}

	Vector4D & Vector4D::ClampMagnitude ( float maxLength )
		{
		float lenSq = LengthSquared ();
		if (lenSq > maxLength * maxLength)
			{
			float len = Sqrt ( lenSq );
			float scale = maxLength / len;
			x *= scale;
			y *= scale;
			z *= scale;
			w *= scale;
			}
		return *this;
		}

	Vector4D Vector4D::Clamp ( const Vector4D & value, const Vector4D & min, const Vector4D & max )
		{
		Vector4D result = value;
		result.Clamp ( min, max );
		return result;
		}

	Vector4D Vector4D::Clamp01 ( const Vector4D & value )
		{
		Vector4D result = value;
		result.Clamp01 ();
		return result;
		}

	Vector4D Vector4D::ClampMagnitude ( const Vector4D & vector, float maxLength )
		{
		Vector4D result = vector;
		result.ClampMagnitude ( maxLength );
		return result;
		}

		// Абсолютные значения и знаки
	Vector4D & Vector4D::Abs ()
		{
		x = CEMath::Abs ( x );
		y = CEMath::Abs ( y );
		z = CEMath::Abs ( z );
		w = CEMath::Abs ( w );
		return *this;
		}

	Vector4D Vector4D::Abs () const
		{
		Vector4D result = *this;
		result.Abs ();
		return result;
		}

	Vector4D & Vector4D::Sign ()
		{
		x = static_cast< float >( CEMath::Sign ( x ) );
		y = static_cast< float >( CEMath::Sign ( y ) );
		z = static_cast< float >( CEMath::Sign ( z ) );
		w = static_cast< float >( CEMath::Sign ( w ) );
		return *this;
		}

	Vector4D Vector4D::Sign () const
		{
		Vector4D result = *this;
		result.Sign ();
		return result;
		}

		// Преобразования
	Vector2D Vector4D::ToVector2D () const
		{
		return Vector2D ( x, y );
		}

	Vector3D Vector4D::ToVector3D () const
		{
		return Vector3D ( x, y, z );
		}

	Vector3D Vector4D::ToVector3DCartesian () const
		{
		return ToCartesian ();
		}

		// Цветовые операции
	Vector4D & Vector4D::Saturate ()
		{
		return Clamp01 ();
		}

	Vector4D Vector4D::Saturated () const
		{
		Vector4D result = *this;
		return result.Saturate ();
		}

	float Vector4D::GetBrightness () const
		{
			// Яркость как среднее RGB компонент
		return ( x + y + z ) / 3.0f;
		}

	Vector4D & Vector4D::Invert ()
		{
		x = 1.0f - x;
		y = 1.0f - y;
		z = 1.0f - z;
		// Альфа обычно не инвертируется
		return *this;
		}

	Vector4D Vector4D::Inverted () const
		{
		Vector4D result = *this;
		result.Invert ();
		return result;
		}

		// Компонентные операции
	float Vector4D::MinComponent () const
		{
		return std::min ( std::min ( x, y ), std::min ( z, w ) );
		}

	float Vector4D::MaxComponent () const
		{
		return std::max ( std::max ( x, y ), std::max ( z, w ) );
		}

	float Vector4D::Sum () const { return x + y + z + w; }

	float Vector4D::Average () const { return ( x + y + z + w ) / 4.0f; }

	std::ostream & operator<<( std::ostream & os, const Vector4D & vec )
		{
		os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
		return os;
		}

	std::istream & operator>>( std::istream & is, Vector4D & vec )
		{
		char open, comma1, comma2, comma3, close;
		is >> open >> vec.x >> comma1 >> vec.y >> comma2 >> vec.z >> comma3 >> vec.w >> close;

		// Если формат не "(x, y, z, w)", попробуем прочитать просто четыре числа
		if (!is || ( open != '(' && open != '[' ) || comma1 != ',' || comma2 != ',' ||
			 comma3 != ',' || ( close != ')' && close != ']' ))
			{
			is.clear ();
			is.seekg ( -static_cast< int >( is.tellg () ), std::ios_base::cur );
			is >> vec.x >> vec.y >> vec.z >> vec.w;
			}

		return is;
		}
	}