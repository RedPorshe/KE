#include "Utils/Math/Quaternion.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace CEMath
{
    // Конструкторы
    Quaternion::Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}

    Quaternion::Quaternion(float x, float y, float z, float w)
        : x(x), y(y), z(z), w(w) {
    }

    Quaternion::Quaternion(const Vector3D& axis, float radians)
    {
        Vector3D normalizedAxis = axis.Normalized();
        float halfAngle = radians * 0.5f;
        float sinHalf = Sin(halfAngle);

        x = normalizedAxis.x * sinHalf;
        y = normalizedAxis.y * sinHalf;
        z = normalizedAxis.z * sinHalf;
        w = Cos(halfAngle);
    }

    bool Quaternion::IsEqual(const Quaternion& other, float epsilon) const
    {
        // Для кватернионов важно сравнивать с учетом того,
        // что q и -q представляют одно и то же вращение

        // Сначала сравниваем как есть
        bool directEqual =
            std::fabs(x - other.x) < epsilon &&
            std::fabs(y - other.y) < epsilon &&
            std::fabs(z - other.z) < epsilon &&
            std::fabs(w - other.w) < epsilon;

        if (directEqual)
            return true;

        // Если не равны напрямую, проверяем противоположные
        // (q и -q представляют одно вращение)
        bool oppositeEqual =
            std::fabs(x + other.x) < epsilon &&
            std::fabs(y + other.y) < epsilon &&
            std::fabs(z + other.z) < epsilon &&
            std::fabs(w + other.w) < epsilon;

        return oppositeEqual;
    }

    Quaternion::Quaternion(float pitch, float yaw, float roll)
    {
        // Правильный порядок: Z * Y * X (Roll * Yaw * Pitch)
        float halfPitch = pitch * 0.5f;
        float halfYaw = yaw * 0.5f;
        float halfRoll = roll * 0.5f;

        float sinPitch = Sin(halfPitch);
        float cosPitch = Cos(halfPitch);
        float sinYaw = Sin(halfYaw);
        float cosYaw = Cos(halfYaw);
        float sinRoll = Sin(halfRoll);
        float cosRoll = Cos(halfRoll);

        // Порядок умножения: Z * Y * X
        x = sinPitch * cosYaw * cosRoll - cosPitch * sinYaw * sinRoll;
        y = cosPitch * sinYaw * cosRoll + sinPitch * cosYaw * sinRoll;
        z = cosPitch * cosYaw * sinRoll - sinPitch * sinYaw * cosRoll;
        w = cosPitch * cosYaw * cosRoll + sinPitch * sinYaw * sinRoll;
    }

    Quaternion::Quaternion(const Quaternion& other)
        : x(other.x), y(other.y), z(other.z), w(other.w) {
    }

    Quaternion::Quaternion(const Matrix4x4& rotationMatrix)
    {
        *this = FromRotationMatrix(rotationMatrix);
    }

    // Операторы присваивания
    Quaternion& Quaternion::operator=(const Quaternion& other)
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
    bool Quaternion::operator==(const Quaternion& other) const
    {
        return CEMath::IsEqual(x, other.x) && CEMath::IsEqual(y, other.y) &&
            CEMath::IsEqual(z, other.z) && CEMath::IsEqual(w, other.w);
    }

    bool Quaternion::operator!=(const Quaternion& other) const
    {
        return !(*this == other);
    }

    // Арифметические операторы
    Quaternion Quaternion::operator+(const Quaternion& other) const
    {
        return Quaternion(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    Quaternion Quaternion::operator-(const Quaternion& other) const
    {
        return Quaternion(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    Quaternion Quaternion::operator*(const Quaternion& other) const
    {
        // q1 * q2 = (w1*w2 - v1·v2, w1*v2 + w2*v1 + v1×v2)
        return Quaternion(
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w,
            w * other.w - x * other.x - y * other.y - z * other.z
        );
    }

    Quaternion Quaternion::operator*(float scalar) const
    {
        return Quaternion(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    Quaternion Quaternion::operator/(float scalar) const
    {
        if (CEMath::IsZero(scalar))
            throw std::runtime_error("Quaternion: Division by zero");

        float invScalar = 1.0f / scalar;
        return *this * invScalar;
    }

    // Умножение кватерниона на вектор (вращение вектора)
    Vector3D Quaternion::operator*(const Vector3D& vec) const
    {
        // v' = q * v * q⁻¹
        // Для единичного кватерниона q⁻¹ = сопряженный(q)
        Quaternion vecQuat(vec.x, vec.y, vec.z, 0.0f);
        Quaternion result = (*this) * vecQuat * Conjugated();

        return Vector3D(result.x, result.y, result.z);
    }

    // Составные операторы присваивания
    Quaternion& Quaternion::operator+=(const Quaternion& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    Quaternion& Quaternion::operator-=(const Quaternion& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    Quaternion& Quaternion::operator*=(const Quaternion& other)
    {
        *this = *this * other;
        return *this;
    }

    Quaternion& Quaternion::operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    Quaternion& Quaternion::operator/=(float scalar)
    {
        if (CEMath::IsZero(scalar))
            throw std::runtime_error("Quaternion: Division by zero");

        float invScalar = 1.0f / scalar;
        return *this *= invScalar;
    }

    // Унарные операторы
    Quaternion Quaternion::operator-() const
    {
        return Quaternion(-x, -y, -z, -w);
    }

    // Базовые математические функции
    float Quaternion::Length() const
    {
        return Sqrt(x * x + y * y + z * z + w * w);
    }

    float Quaternion::LengthSquared() const
    {
        return x * x + y * y + z * z + w * w;
    }

    float Quaternion::Dot(const Quaternion& other) const
    {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    Quaternion& Quaternion::Normalize()
    {
        float len = Length();
        if (!CEMath::IsZero(len))
        {
            float invLen = 1.0f / len;
            x *= invLen;
            y *= invLen;
            z *= invLen;
            w *= invLen;
        }
        return *this;
    }

    Quaternion Quaternion::Normalized() const
    {
        Quaternion result = *this;
        return result.Normalize();
    }

    Quaternion& Quaternion::Conjugate()
    {
        x = -x;
        y = -y;
        z = -z;
        // w остается без изменений
        return *this;
    }

    Quaternion Quaternion::Conjugated() const
    {
        Quaternion result = *this;
        return result.Conjugate();
    }

    Quaternion& Quaternion::Inverse()
    {
        // Для единичного кватерниона обратный = сопряженный
        float lenSq = LengthSquared();
        if (CEMath::IsZero(lenSq))
            throw std::runtime_error("Quaternion: Cannot invert zero quaternion");

        float invLenSq = 1.0f / lenSq;
        x = -x * invLenSq;
        y = -y * invLenSq;
        z = -z * invLenSq;
        w = w * invLenSq;

        return *this;
    }

    Quaternion Quaternion::Inversed() const
    {
        Quaternion result = *this;
        return result.Inverse();
    }

    // Проверки
    bool Quaternion::IsUnit() const
    {
        return CEMath::IsEqual(LengthSquared(), 1.0f);
    }

    bool Quaternion::IsIdentity() const
    {
        return CEMath::IsEqual(x, CEMath::EPSILON) && CEMath::IsEqual(y, CEMath::EPSILON) &&
            CEMath::IsEqual(z, CEMath::EPSILON) && CEMath::IsEqual(w, CEMath::EPSILON);
    }

    bool Quaternion::IsZero() const
    {
        return CEMath::IsEqual(x, CEMath::EPSILON) && CEMath::IsEqual(y, CEMath::EPSILON) &&
            CEMath::IsEqual(z, CEMath::EPSILON) && CEMath::IsEqual(w, CEMath::EPSILON);
    }

    // Получение информации
    float Quaternion::GetAngle() const
    {
        if (!IsUnit())
        {
            Quaternion normalized = Normalized();
            return 2.0f * Acos(Clamp(normalized.w, -1.0f, 1.0f));
        }
        return 2.0f * Acos(Clamp(w, -1.0f, 1.0f));
    }

    Vector3D Quaternion::GetAxis() const
    {
        Quaternion normalized = Normalized();
        float sinHalfAngle = Sqrt(1.0f - normalized.w * normalized.w);

        if (CEMath::IsZero(sinHalfAngle))
            return Vector3D(1.0f, 0.0f, 0.0f); // Ось по умолчанию

        float invSinHalf = 1.0f / sinHalfAngle;
        return Vector3D(
            normalized.x * invSinHalf,
            normalized.y * invSinHalf,
            normalized.z * invSinHalf
        );
    }

    Vector3D Quaternion::GetEulerAngles() const
    {
        Quaternion q = Normalized();

        // Преобразование кватерниона в углы Эйлера (ZYX порядок - Yaw, Pitch, Roll)
        float sinPitch = 2.0f * (q.w * q.x + q.y * q.z);
        float cosPitch = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
        float pitch = Atan2(sinPitch, cosPitch);

        float sinYaw = 2.0f * (q.w * q.y - q.z * q.x);
        float yaw = 0.0f;
        if (Abs(sinYaw) >= 1.0f)
            yaw = CopySign(HALF_PI, sinYaw);
        else
            yaw = Asin(sinYaw);

        float sinRoll = 2.0f * (q.w * q.z + q.x * q.y);
        float cosRoll = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
        float roll = Atan2(sinRoll, cosRoll);

        return Vector3D(pitch, yaw, roll);
    }

    // Преобразования
    Matrix4x4 Quaternion::ToMatrix() const
    {
        Quaternion q = Normalized();

        float xx = q.x * q.x;
        float yy = q.y * q.y;
        float zz = q.z * q.z;
        float xy = q.x * q.y;
        float xz = q.x * q.z;
        float yz = q.y * q.z;
        float wx = q.w * q.x;
        float wy = q.w * q.y;
        float wz = q.w * q.z;

        return Matrix4x4(
            1.0f - 2.0f * (yy + zz), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f,
            2.0f * (xy - wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz + wx), 0.0f,
            2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (xx + yy), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    Vector4D Quaternion::ToVector4() const
    {
        return Vector4D(x, y, z, w);
    }

    // Операции вращения
    Quaternion& Quaternion::Rotate(const Quaternion& rotation)
    {
        *this = rotation * *this;
        return *this;
    }

    Quaternion& Quaternion::Rotate(const Vector3D& axis, float radians)
    {
        Quaternion rotation(axis, radians);
        *this = rotation * *this;
        return *this;
    }

    Quaternion& Quaternion::RotateX(float radians)
    {
        return Rotate(Vector3D::UnitX(), radians);
    }

    Quaternion& Quaternion::RotateY(float radians)
    {
        return Rotate(Vector3D::UnitY(), radians);
    }

    Quaternion& Quaternion::RotateZ(float radians)
    {
        return Rotate(Vector3D::UnitZ(), radians);
    }
    // Линейная интерполяция
    Quaternion Quaternion::Lerp(const Quaternion& a, const Quaternion& b, float t)
    {
        float clampedT = Clamp(t, 0.0f, 1.0f);
        float dot = a.Dot(b);

        // Убедимся, что интерполируем по кратчайшему пути
        Quaternion bAdjusted = b;
        if (dot < 0.0f)
        {
            bAdjusted = -b;
            dot = -dot;
        }

        float invT = 1.0f - clampedT;
        return Quaternion(
            a.x * invT + bAdjusted.x * clampedT,
            a.y * invT + bAdjusted.y * clampedT,
            a.z * invT + bAdjusted.z * clampedT,
            a.w * invT + bAdjusted.w * clampedT
        ).Normalized();
    }

    // Нормализованная линейная интерполяция (быстрее, чем Slerp)
    Quaternion Quaternion::Nlerp(const Quaternion& a, const Quaternion& b, float t)
    {
        return Lerp(a, b, t).Normalized();
    }

    // Сферическая линейная интерполяция (правильная)
    Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float t)
    {
        float clampedT = Clamp(t, 0.0f, 1.0f);
        float dot = a.Dot(b);

        // Убедимся, что интерполируем по кратчайшему пути
        Quaternion bAdjusted = b;
        if (dot < 0.0f)
        {
            bAdjusted = -b;
            dot = -dot;
        }

        // Если кватернионы очень близки, используем Lerp для избежания деления на ноль
        if (dot > 0.9995f)
            return Nlerp(a, bAdjusted, clampedT);

        float theta = Acos(dot);
        float sinTheta = Sin(theta);

        float factorA = Sin((1.0f - clampedT) * theta) / sinTheta;
        float factorB = Sin(clampedT * theta) / sinTheta;

        return Quaternion(
            a.x * factorA + bAdjusted.x * factorB,
            a.y * factorA + bAdjusted.y * factorB,
            a.z * factorA + bAdjusted.z * factorB,
            a.w * factorA + bAdjusted.w * factorB
        );
    }

    Quaternion Quaternion::SlerpShortestPath(const Quaternion& a, const Quaternion& b, float t)
    {
        float clampedT = Clamp(t, 0.0f, 1.0f);
        float dot = a.Dot(b);

        Quaternion bAdjusted = b;
        if (dot < 0.0f)
        {
            bAdjusted = -b;
            dot = -dot;
        }

        return Slerp(a, bAdjusted, clampedT);
    }

    // Вращение к цели
    Quaternion Quaternion::RotateTowards(const Quaternion& from, const Quaternion& to, float maxRadiansDelta)
    {
        float dot = from.Dot(to);

        // Если кватернионы совпадают или противоположны
        if (dot > 0.9999f)
            return to;

        // Угол между кватернионами
        float theta = Acos(Clamp(dot, -1.0f, 1.0f));

        // Если угол очень мал, сразу возвращаем целевой кватернион
        if (theta < maxRadiansDelta)
            return to;

        // Интерполируем на максимально допустимый угол
        float t = maxRadiansDelta / theta;
        return Slerp(from, to, t);
    }

    // Утилиты
    Quaternion Quaternion::Identity()
    {
        return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    }

    Quaternion Quaternion::Zero()
    {
        return Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
    }

    // Создание из углов Эйлера
    Quaternion Quaternion::FromEulerAngles(float pitch, float yaw, float roll)
    {
        return Quaternion(pitch, yaw, roll);
    }

    Quaternion Quaternion::FromEulerAngles(const Vector3D& eulerAngles)
    {
        return Quaternion(eulerAngles.x, eulerAngles.y, eulerAngles.z);
    }

    // Создание из осей
    Quaternion Quaternion::FromAxes(const Vector3D& right, const Vector3D& up, const Vector3D& forward)
    {
        Matrix4x4 rotationMatrix(
            right.x, up.x, forward.x, 0.0f,
            right.y, up.y, forward.y, 0.0f,
            right.z, up.z, forward.z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );

        return FromRotationMatrix(rotationMatrix);
    }

    // Создание из матрицы поворота
    Quaternion Quaternion::FromRotationMatrix(const Matrix4x4& rotationMatrix)
    {
        float trace = rotationMatrix(0, 0) + rotationMatrix(1, 1) + rotationMatrix(2, 2);

        if (trace > 0.0f)
        {
            float s = 0.5f / Sqrt(trace + 1.0f);
            return Quaternion(
                (rotationMatrix(2, 1) - rotationMatrix(1, 2)) * s,
                (rotationMatrix(0, 2) - rotationMatrix(2, 0)) * s,
                (rotationMatrix(1, 0) - rotationMatrix(0, 1)) * s,
                0.25f / s
            );
        }
        else
        {
            if (rotationMatrix(0, 0) > rotationMatrix(1, 1) && rotationMatrix(0, 0) > rotationMatrix(2, 2))
            {
                float s = 2.0f * Sqrt(1.0f + rotationMatrix(0, 0) - rotationMatrix(1, 1) - rotationMatrix(2, 2));
                return Quaternion(
                    0.25f * s,
                    (rotationMatrix(0, 1) + rotationMatrix(1, 0)) / s,
                    (rotationMatrix(0, 2) + rotationMatrix(2, 0)) / s,
                    (rotationMatrix(2, 1) - rotationMatrix(1, 2)) / s
                );
            }
            else if (rotationMatrix(1, 1) > rotationMatrix(2, 2))
            {
                float s = 2.0f * Sqrt(1.0f + rotationMatrix(1, 1) - rotationMatrix(0, 0) - rotationMatrix(2, 2));
                return Quaternion(
                    (rotationMatrix(0, 1) + rotationMatrix(1, 0)) / s,
                    0.25f * s,
                    (rotationMatrix(1, 2) + rotationMatrix(2, 1)) / s,
                    (rotationMatrix(0, 2) - rotationMatrix(2, 0)) / s
                );
            }
            else
            {
                float s = 2.0f * Sqrt(1.0f + rotationMatrix(2, 2) - rotationMatrix(0, 0) - rotationMatrix(1, 1));
                return Quaternion(
                    (rotationMatrix(0, 2) + rotationMatrix(2, 0)) / s,
                    (rotationMatrix(1, 2) + rotationMatrix(2, 1)) / s,
                    0.25f * s,
                    (rotationMatrix(1, 0) - rotationMatrix(0, 1)) / s
                );
            }
        }
    }

    // Создание из направления взгляда
    Quaternion Quaternion::LookRotation(const Vector3D& forward, const Vector3D& up)
    {
        Vector3D f = forward.Normalized();
        Vector3D u = up.Normalized();
        Vector3D r = u.Cross(f).Normalized();
        u = f.Cross(r).Normalized();

        return FromAxes(r, u, f);
    }

    // Внешние операторы
    Quaternion operator*(float scalar, const Quaternion& q)
    {
        return q * scalar;
    }

    Vector3D operator*(const Vector3D& vec, const Quaternion& q)
    {
        return q * vec;
    }

    std::ostream& operator<<(std::ostream& os, const Quaternion& q)
    {
        os << std::fixed << std::setprecision(6);

        // Выводим в формате: (x, y, z, w) или (angle: axis)
        if (q.IsUnit())
        {
            float angle = 2.0f * std::acos(std::max(-1.0f, std::min(1.0f, q.w)));
            Vector3D axis;

            if (std::fabs(angle) < EPSILON)
            {
                axis = Vector3D::UnitX();
            }
            else
            {
                float s = std::sqrt(1.0f - q.w * q.w);
                if (s > EPSILON)
                {
                    axis = Vector3D(q.x / s, q.y / s, q.z / s);
                }
                else
                {
                    axis = Vector3D::UnitX();
                }
            }

            os << "Quaternion [angle: " << RadiansToDegrees(angle) << "°, axis: " << axis << "]";
        }
        else
        {
            os << "Quaternion (" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")";
        }

        return os;
    }

    std::istream& operator>>(std::istream& is, Quaternion& q)
    {
        std::string token;
        is >> token;

        // Если первый токен "Quaternion", пропускаем его
        if (token == "Quaternion" || token == "quaternion")
        {
            char c;
            is >> c; // Пропускаем '['
            is >> token; // Читаем следующее слово
        }

        // Пробуем разные форматы
        if (token == "angle:")
        {
            // Формат: angle: X°, axis: (x, y, z)
            float angleDegrees;
            std::string degreesSymbol;
            is >> angleDegrees >> degreesSymbol;

            std::string axisLabel;
            is >> axisLabel; // "axis:"

            Vector3D axis;
            is >> axis;

            q = Quaternion(axis, DegreesToRadians(angleDegrees));
        }
        else if (token[0] == '(' || token[0] == '[')
        {
            // Формат: (x, y, z, w) или [x, y, z, w]
            // Извлекаем x из token (может содержать '(' или '[')
            std::string xStr = token.substr(1);
            std::stringstream ss(xStr);
            ss >> q.x;

            char comma;
            is >> q.y >> comma >> q.z >> comma >> q.w;

            // Пропускаем закрывающую скобку
            char close;
            is >> close;
        }
        else
        {
            // Просто 4 числа
            std::stringstream ss(token);
            ss >> q.x;
            is >> q.y >> q.z >> q.w;
        }

        return is;
    }
}