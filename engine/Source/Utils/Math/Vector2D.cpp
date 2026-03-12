#include "Utils/Math/Vector2D.h"
#include <iostream>
#include <iomanip>

namespace CEMath
{
    Vector2D::Vector2D() : x(0.f), y(0.f) {}

    Vector2D::Vector2D(float scalar) : x(scalar), y(scalar) {}

    Vector2D::Vector2D(float inX, float inY) : x(inX), y(inY) {}

    Vector2D::Vector2D(int inX, int inY) :
        x(static_cast<float>(inX)), y(static_cast<float>(inY)) {
    }

    Vector2D::Vector2D(const Vector2D& other) : x(other.x), y(other.y) {}

    
    Vector2D& Vector2D::operator=(const Vector2D& other)
    {
        if (this != &other)
        {
            x = other.x;
            y = other.y;
        }
        return *this;
    }

    
    bool Vector2D::operator==(const Vector2D& other) const
    {
        return IsEqual(x, other.x) && IsEqual(y, other.y);
    }

    bool Vector2D::operator!=(const Vector2D& other) const
    {
        return !(*this == other);
    }

    
    Vector2D Vector2D::operator+(const Vector2D& other) const
    {
        return Vector2D(x + other.x, y + other.y);
    }

    Vector2D Vector2D::operator-(const Vector2D& other) const
    {
        return Vector2D(x - other.x, y - other.y);
    }

    Vector2D Vector2D::operator*(const Vector2D& other) const
    {
        return Vector2D(x * other.x, y * other.y);
    }

    Vector2D Vector2D::operator/(const Vector2D& other) const
    {
        if (CEMath::IsZero(other.x) || CEMath::IsZero(other.y))
        {
            throw std::runtime_error("Vector2D: Division by zero");
        }
        return Vector2D(x / other.x, y / other.y);
    }

    Vector2D Vector2D::operator+(float scalar) const
    {
        return Vector2D(x + scalar, y + scalar);
    }

    Vector2D Vector2D::operator-(float scalar) const
    {
        return Vector2D(x - scalar, y - scalar);
    }

    Vector2D Vector2D::operator*(float scalar) const
    {
        return Vector2D(x * scalar, y * scalar);
    }

    Vector2D Vector2D::operator/(float scalar) const
    {
        if (CEMath::IsZero(scalar))
        {
            throw std::runtime_error("Vector2D: Division by zero");
        }
        float invScalar = 1.0f / scalar;
        return Vector2D(x * invScalar, y * invScalar);
    }

   
    Vector2D& Vector2D::operator+=(const Vector2D& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2D& Vector2D::operator-=(const Vector2D& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vector2D& Vector2D::operator*=(const Vector2D& other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    Vector2D& Vector2D::operator/=(const Vector2D& other)
    {
        if (CEMath::IsZero(other.x) || CEMath::IsZero(other.y))
        {
            throw std::runtime_error("Vector2D: Division by zero");
        }
        x /= other.x;
        y /= other.y;
        return *this;
    }

    Vector2D& Vector2D::operator+=(float scalar)
    {
        x += scalar;
        y += scalar;
        return *this;
    }

    Vector2D& Vector2D::operator-=(float scalar)
    {
        x -= scalar;
        y -= scalar;
        return *this;
    }

    Vector2D& Vector2D::operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2D& Vector2D::operator/=(float scalar)
    {
        if (CEMath::IsZero(scalar))
        {
            throw std::runtime_error("Vector2D: Division by zero");
        }
        float invScalar = 1.0f / scalar;
        x *= invScalar;
        y *= invScalar;
        return *this;
    }

   
    Vector2D Vector2D::operator-() const
    {
        return Vector2D(-x, -y);
    }

  
    Vector2D& Vector2D::operator++()
    {
        x += 1.0f;
        y += 1.0f;
        return *this;
    }

    Vector2D Vector2D::operator++(int)
    {
        Vector2D temp = *this;
        ++(*this);
        return temp;
    }

    Vector2D& Vector2D::operator--()
    {
        x -= 1.0f;
        y -= 1.0f;
        return *this;
    }

    Vector2D Vector2D::operator--(int)
    {
        Vector2D temp = *this;
        --(*this);
        return temp;
    }

   
    float Vector2D::Length() const
    {
        return Sqrt(x * x + y * y);
    }

    float Vector2D::LengthSquared() const
    {
        return x * x + y * y;
    }

    float Vector2D::Dot(const Vector2D& other) const
    {
        return x * other.x + y * other.y;
    }

    float Vector2D::Cross(const Vector2D& other) const
    {
        return x * other.y - y * other.x;
    }

    Vector2D& Vector2D::Normalize()
    {
        float len = Length();
        if (CEMath::IsZero(len))
        {
            throw std::runtime_error("Vector2D: Cannot normalize zero vector");
        }
        float invLen = 1.0f / len;
        x *= invLen;
        y *= invLen;
        return *this;
    }

    Vector2D Vector2D::Normalized() const
    {
        Vector2D result = *this;
        return result.Normalize();
    }

    
    bool Vector2D::IsZero() const
    {
        return LengthSquared() < EPSILON_SQUARED;
    }

    Vector2D Vector2D::Zero()
    {
        return Vector2D(0.0f, 0.0f);
    }

    Vector2D Vector2D::One()
    {
        return Vector2D(1.0f, 1.0f);
    }

    Vector2D Vector2D::UnitX()
    {
        return Vector2D(1.0f, 0.0f);
    }

    Vector2D Vector2D::UnitY()
    {
        return Vector2D(0.0f, 1.0f);
    }
    // Расстояние между векторами
    float Vector2D::Distance(const Vector2D& other) const
    {
        float dx = x - other.x;
        float dy = y - other.y;
        return Sqrt(dx * dx + dy * dy);
    }

    float Vector2D::DistanceSquared(const Vector2D& other) const
    {
        float dx = x - other.x;
        float dy = y - other.y;
        return dx * dx + dy * dy;
    }

    // Проверка нормализован ли вектор
    bool Vector2D::IsNormalized() const
    {
        return IsEqual(LengthSquared(), 1.0f);
    }

    // Вращение
    Vector2D& Vector2D::Rotate(float radians)
    {
        float cosAngle = Cos(radians);
        float sinAngle = Sin(radians);
        float newX = x * cosAngle - y * sinAngle;
        float newY = x * sinAngle + y * cosAngle;
        x = newX;
        y = newY;
        return *this;
    }

    Vector2D Vector2D::Rotated(float radians) const
    {
        Vector2D result = *this;
        return result.Rotate(radians);
    }

    // Угол вектора
    float Vector2D::Angle() const
    {
        return Atan2(y, x);
    }

    float Vector2D::Angle(const Vector2D& from, const Vector2D& to)
    {
        float dot = CEMath::Clamp(from.Dot(to), -1.0f, 1.0f);
        return Acos(dot);
    }

    Vector2D& Vector2D::RotateTowards(const Vector2D& target, float maxRadiansDelta)
    {
        float angle = Angle(*this, target);
        if (angle < maxRadiansDelta)
        {
            *this = target;
        }
        else
        {
            float t = maxRadiansDelta / angle;
            *this = Lerp(*this, target, t);
        }
        return *this;
    }

    // Отражение (физика)
    Vector2D Vector2D::Reflect(const Vector2D& normal) const
    {
        float dot = Dot(normal);
        return *this - normal * (2.0f * dot);
    }

    // Проекция
    Vector2D Vector2D::Project(const Vector2D& normal) const
    {
        float dot = Dot(normal);
        float lenSq = normal.LengthSquared();
        if (CEMath::IsZero(lenSq))
            return Zero();
        return normal * (dot / lenSq);
    }

    // Линейная интерполяция
    Vector2D Vector2D::Lerp(const Vector2D& a, const Vector2D& b, float t)
    {
        float clampedT = CEMath::Clamp(t, 0.0f, 1.0f);
        return Vector2D(
            a.x + (b.x - a.x) * clampedT,
            a.y + (b.y - a.y) * clampedT
        );
    }

    Vector2D Vector2D::LerpUnclamped(const Vector2D& a, const Vector2D& b, float t)
    {
        return Vector2D(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t
        );
    }

    Vector2D Vector2D::Slerp(const Vector2D& a, const Vector2D& b, float t)
    {
        float clampedT = CEMath::Clamp(t, 0.0f, 1.0f);

        float dot = CEMath::Clamp(a.Dot(b), -1.0f, 1.0f);
        float theta = Acos(dot) * clampedT;

        Vector2D relativeVec = b - a * dot;
        if (relativeVec.LengthSquared() > EPSILON_SQUARED)
            relativeVec.Normalize();
        else
            relativeVec = a.PerpendicularCounterClockwise();

        return a * Cos(theta) + relativeVec * Sin(theta);
    }

    // Ограничение
    Vector2D& Vector2D::Clamp(const Vector2D& min, const Vector2D& max)
    {
        x = CEMath::Clamp(x, min.x, max.x);
        y = CEMath::Clamp(y, min.y, max.y);
        return *this;
    }

    Vector2D& Vector2D::ClampMagnitude(float maxLength)
    {
        float lenSq = LengthSquared();
        if (lenSq > maxLength * maxLength)
        {
            float len = Sqrt(lenSq);
            float scale = maxLength / len;
            x *= scale;
            y *= scale;
        }
        return *this;
    }

    Vector2D Vector2D::Clamp(const Vector2D& value, const Vector2D& min, const Vector2D& max)
    {
        Vector2D result = value;
        result.Clamp(min, max);
        return result;
    }

    Vector2D Vector2D::ClampMagnitude(const Vector2D& vector, float maxLength)
    {
        Vector2D result = vector;
        return result.ClampMagnitude(maxLength);
    }

    // Минимум/максимум
    Vector2D Vector2D::Min(const Vector2D& a, const Vector2D& b)
    {
        return Vector2D(
            std::min(a.x, b.x),
            std::min(a.y, b.y)
        );
    }

    Vector2D Vector2D::Max(const Vector2D& a, const Vector2D& b)
    {
        return Vector2D(
            std::max(a.x, b.x),
            std::max(a.y, b.y)
        );
    }

    Vector2D& Vector2D::Abs()
    {
        x = CEMath::Abs(x);
        y = CEMath::Abs(y);
        return *this;
    }

    Vector2D Vector2D::Abs() const
    {
        Vector2D result = *this;
        return result.Abs();
    }

    // Масштабирование
    Vector2D& Vector2D::Scale(const Vector2D& other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    Vector2D& Vector2D::Scale(float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2D Vector2D::Scaled(const Vector2D& other) const
    {
        Vector2D result = *this;
        result.Scale(other);
        return result;
    }

    Vector2D Vector2D::Scaled(float scalar) const
    {
        Vector2D result = *this;
        result.Scale(scalar);
        return result;
    }

    // Проверка на корректность
    bool Vector2D::IsFinite() const
    {
        return std::isfinite(x) && std::isfinite(y);
    }

    // Направленные векторы
    Vector2D Vector2D::Left() { return Vector2D(-1.0f, 0.0f); }
    Vector2D Vector2D::Right() { return Vector2D(1.0f, 0.0f); }
    Vector2D Vector2D::Up() { return Vector2D(0.0f, 1.0f); }
    Vector2D Vector2D::Down() { return Vector2D(0.0f, -1.0f); }
    Vector2D Vector2D::UpLeft() { return Vector2D(-0.707106781f, 0.707106781f).Normalized(); }
    Vector2D Vector2D::UpRight() { return Vector2D(0.707106781f, 0.707106781f).Normalized(); }
    Vector2D Vector2D::DownLeft() { return Vector2D(-0.707106781f, -0.707106781f).Normalized(); }
    Vector2D Vector2D::DownRight() { return Vector2D(0.707106781f, -0.707106781f).Normalized(); }

    // Вспомогательные функции
    float Vector2D::MinComponent() const { return std::min(x, y); }
    float Vector2D::MaxComponent() const { return std::max(x, y); }
    float Vector2D::Sum() const { return x + y; }
    float Vector2D::Average() const { return (x + y) * 0.5f; }

    // Перпендикулярные векторы
    Vector2D Vector2D::PerpendicularClockwise() const { return Vector2D(y, -x); }
    Vector2D Vector2D::PerpendicularCounterClockwise() const { return Vector2D(-y, x); }

    // Расстояние до линии
    float Vector2D::DistanceToLine(const Vector2D& lineStart, const Vector2D& lineEnd) const
    {
        Vector2D line = lineEnd - lineStart;
        Vector2D pointToStart = *this - lineStart;

        float lineLengthSq = line.LengthSquared();
        if (CEMath::IsZero(lineLengthSq))
            return Distance(lineStart);

        float t = CEMath::Clamp(pointToStart.Dot(line) / lineLengthSq, 0.0f, 1.0f);
        Vector2D projection = lineStart + line * t;

        return Distance(projection);
    }

    float Vector2D::DistanceToSegment(const Vector2D& segmentStart, const Vector2D& segmentEnd) const
    {
        return DistanceToLine(segmentStart, segmentEnd);
    }

    // Преобразования
    Vector2D& Vector2D::Floor()
    {
        x = CEMath::Floor(x);
        y = CEMath::Floor(y);
        return *this;
    }

    Vector2D Vector2D::Floor() const
    {
        Vector2D result = *this;
        return result.Floor();
    }

    Vector2D& Vector2D::Ceil()
    {
        x = CEMath::Ceil(x);
        y = CEMath::Ceil(y);
        return *this;
    }

    Vector2D Vector2D::Ceil() const
    {
        Vector2D result = *this;
        return result.Ceil();
    }

    Vector2D& Vector2D::Round()
    {
        x = CEMath::Round(x);
        y = CEMath::Round(y);
        return *this;
    }

    Vector2D Vector2D::Round() const
    {
        Vector2D result = *this;
        return result.Round();
    }

    Vector2D& Vector2D::Sign()
    {
        x = static_cast< float >( CEMath::Sign ( x ) );
        y = static_cast< float >( CEMath::Sign ( y ) );
        return *this;
    }

    Vector2D Vector2D::Sign() const
    {
        Vector2D result = *this;
        return result.Sign();
    }

    std::ostream& operator<<(std::ostream& os, const Vector2D& vec)
    {
        os << "(" << vec.x << ", " << vec.y << ")";
        return os;
    }

    std::istream& operator>>(std::istream& is, Vector2D& vec)
    {
        char open, comma, close;
        is >> open >> vec.x >> comma >> vec.y >> close;

        // Если формат не "(x, y)", попробуем прочитать просто два числа
        if (!is || (open != '(' && open != '[') || comma != ',' ||
            (close != ')' && close != ']'))
        {
            is.clear();
            is.seekg(-static_cast<int>(is.tellg()), std::ios_base::cur);
            is >> vec.x >> vec.y;
        }

        return is;
    }
}