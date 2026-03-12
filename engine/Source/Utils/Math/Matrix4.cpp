#include "Utils/Math/Matrix4.h"
#include "Utils/Math/Quaternion.h"

namespace CEMath
{
   
    Matrix4x4::Matrix4x4()
    {
        Identity();
    }

    Matrix4x4::Matrix4x4(float diagonal)
    {
        for (int i = 0; i < 16; ++i)
            m[i] = 0.0f;

        m[0] = m[5] = m[10] = m[15] = diagonal;
    }

    Matrix4x4::Matrix4x4(const float data[16])
    {
        for (int i = 0; i < 16; ++i)
            m[i] = data[i];
    }


    Matrix4x4::Matrix4x4 ( const Quaternion & rotation, const Vector3D & scale )
        {
        Quaternion rot = rotation.Normalized ();

        float xx = rot.x * rot.x;
        float yy = rot.y * rot.y;
        float zz = rot.z * rot.z;
        float xy = rot.x * rot.y;
        float xz = rot.x * rot.z;
        float yz = rot.y * rot.z;
        float wx = rot.w * rot.x;
        float wy = rot.w * rot.y;
        float wz = rot.w * rot.z;

        // Только поворотная часть с масштабом
        m[ 0 ] = ( 1.0f - 2.0f * ( yy + zz ) ) * scale.x;
        m[ 1 ] = ( 2.0f * ( xy + wz ) ) * scale.x;
        m[ 2 ] = ( 2.0f * ( xz - wy ) ) * scale.x;
        m[ 3 ] = 0.0f;

        m[ 4 ] = ( 2.0f * ( xy - wz ) ) * scale.y;
        m[ 5 ] = ( 1.0f - 2.0f * ( xx + zz ) ) * scale.y;
        m[ 6 ] = ( 2.0f * ( yz + wx ) ) * scale.y;
        m[ 7 ] = 0.0f;

        m[ 8 ] = ( 2.0f * ( xz + wy ) ) * scale.z;
        m[ 9 ] = ( 2.0f * ( yz - wx ) ) * scale.z;
        m[ 10 ] = ( 1.0f - 2.0f * ( xx + yy ) ) * scale.z;
        m[ 11 ] = 0.0f;

        m[ 12 ] = 0.0f;
        m[ 13 ] = 0.0f;
        m[ 14 ] = 0.0f;
        m[ 15 ] = 1.0f;
        }

    Matrix4x4::Matrix4x4 ( const Vector3D & xAxis, const Vector3D & yAxis, const Vector3D & zAxis, const Vector3D & position )
        {
            // Первая колонка (ось X)
        m[ 0 ] = xAxis.x;
        m[ 1 ] = xAxis.y;
        m[ 2 ] = xAxis.z;
        m[ 3 ] = 0.0f;

        // Вторая колонка (ось Y)
        m[ 4 ] = yAxis.x;
        m[ 5 ] = yAxis.y;
        m[ 6 ] = yAxis.z;
        m[ 7 ] = 0.0f;

        // Третья колонка (ось Z)
        m[ 8 ] = zAxis.x;
        m[ 9 ] = zAxis.y;
        m[ 10 ] = zAxis.z;
        m[ 11 ] = 0.0f;

        // Четвёртая колонка (позиция)
        m[ 12 ] = position.x;
        m[ 13 ] = position.y;
        m[ 14 ] = position.z;
        m[ 15 ] = 1.0f;
        }


    Matrix4x4::Matrix4x4(const Matrix4x4& other)
    {
        for (int i = 0; i < 16; ++i)
            m[i] = other.m[i];
    }

    Matrix4x4::Matrix4x4(float m00, float m10, float m20, float m30,
        float m01, float m11, float m21, float m31,
        float m02, float m12, float m22, float m32,
        float m03, float m13, float m23, float m33)
    {
        
        m[0] = m00; m[4] = m01; m[8] = m02;  m[12] = m03;
        m[1] = m10; m[5] = m11; m[9] = m12;  m[13] = m13;
        m[2] = m20; m[6] = m21; m[10] = m22; m[14] = m23;
        m[3] = m30; m[7] = m31; m[11] = m32; m[15] = m33;
    }

    
    Matrix4x4& Matrix4x4::operator=(const Matrix4x4& other)
    {
        if (this != &other)
        {
            for (int i = 0; i < 16; ++i)
                m[i] = other.m[i];
        }
        return *this;
    }

    
    float& Matrix4x4::operator()(int row, int col)
    {
        if (row < 0 || row > 3 || col < 0 || col > 3)
            throw std::out_of_range("Matrix4 index out of range");

        return m[col * 4 + row];
    }

    float Matrix4x4::operator()(int row, int col) const
    {
        if (row < 0 || row > 3 || col < 0 || col > 3)
            throw std::out_of_range("Matrix4 index out of range");

        return m[col * 4 + row];
    }

    float& Matrix4x4::operator[](int index)
    {
        if (index < 0 || index > 15)
            throw std::out_of_range("Matrix4 index out of range");

        return m[index];
    }

    float Matrix4x4::operator[](int index) const
    {
        if (index < 0 || index > 15)
            throw std::out_of_range("Matrix4 index out of range");

        return m[index];
    }

    
    bool Matrix4x4::operator==(const Matrix4x4& other) const
    {
        for (int i = 0; i < 16; ++i)
        {
            if (!IsEqual(m[i], other.m[i]))
                return false;
        }
        return true;
    }

    bool Matrix4x4::operator!=(const Matrix4x4& other) const
    {
        return !(*this == other);
    }

    
    Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const
    {
        Matrix4x4 result;
        for (int i = 0; i < 16; ++i)
            result.m[i] = m[i] + other.m[i];
        return result;
    }

    Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const
    {
        Matrix4x4 result;
        for (int i = 0; i < 16; ++i)
            result.m[i] = m[i] - other.m[i];
        return result;
    }

    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const
    {
        Matrix4x4 result;

        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k)
                {
                    sum += (*this)(i, k) * other(k, j);
                }
                result(i, j) = sum;
            }
        }

        return result;
    }

    Matrix4x4 Matrix4x4::operator*(float scalar) const
    {
        Matrix4x4 result;
        for (int i = 0; i < 16; ++i)
            result.m[i] = m[i] * scalar;
        return result;
    }

    Matrix4x4 Matrix4x4::operator/(float scalar) const
    {
        if (IsZero(scalar))
            throw std::runtime_error("Matrix4: Division by zero");

        float invScalar = 1.0f / scalar;
        return *this * invScalar;
    }

    
    Vector4D Matrix4x4::operator*(const Vector4D& vec) const
    {
        return Vector4D(
            m[0] * vec.x + m[4] * vec.y + m[8] * vec.z + m[12] * vec.w,
            m[1] * vec.x + m[5] * vec.y + m[9] * vec.z + m[13] * vec.w,
            m[2] * vec.x + m[6] * vec.y + m[10] * vec.z + m[14] * vec.w,
            m[3] * vec.x + m[7] * vec.y + m[11] * vec.z + m[15] * vec.w
        );
    }

    Vector3D Matrix4x4::operator*(const Vector3D& vec) const
    {
        float x = m[0] * vec.x + m[4] * vec.y + m[8] * vec.z + m[12];
        float y = m[1] * vec.x + m[5] * vec.y + m[9] * vec.z + m[13];
        float z = m[2] * vec.x + m[6] * vec.y + m[10] * vec.z + m[14];
        float w = m[3] * vec.x + m[7] * vec.y + m[11] * vec.z + m[15];

        if (!IsZero(w) && !IsEqual(w, 1.0f))
        {
            float invW = 1.0f / w;
            x *= invW;
            y *= invW;
            z *= invW;
        }

        return Vector3D(x, y, z);
    }
   
    Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& other)
    {
        for (int i = 0; i < 16; ++i)
            m[i] += other.m[i];
        return *this;
    }

    Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& other)
    {
        for (int i = 0; i < 16; ++i)
            m[i] -= other.m[i];
        return *this;
    }

    Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& other)
    {
        *this = *this * other;
        return *this;
    }

    Matrix4x4& Matrix4x4::operator*=(float scalar)
    {
        for (int i = 0; i < 16; ++i)
            m[i] *= scalar;
        return *this;
    }

    Matrix4x4& Matrix4x4::operator/=(float scalar)
    {
        if (IsZero(scalar))
            throw std::runtime_error("Matrix4: Division by zero");

        float invScalar = 1.0f / scalar;
        return *this *= invScalar;
    }

    
    Matrix4x4 Matrix4x4::operator-() const
    {
        Matrix4x4 result;
        for (int i = 0; i < 16; ++i)
            result.m[i] = -m[i];
        return result;
    }

    
    Matrix4x4 Matrix4x4::Transposed() const
    {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                result(i, j) = (*this)(j, i);
            }
        }
        return result;
    }

    Matrix4x4& Matrix4x4::Transpose()
    {
        *this = Transposed();
        return *this;
    }

    float Matrix4x4::Determinant() const
    {
        float det = 0.0f;

        det += m[0] * (
            m[5] * (m[10] * m[15] - m[11] * m[14]) -
            m[6] * (m[9] * m[15] - m[11] * m[13]) +
            m[7] * (m[9] * m[14] - m[10] * m[13])
            );

        det -= m[4] * (
            m[1] * (m[10] * m[15] - m[11] * m[14]) -
            m[2] * (m[9] * m[15] - m[11] * m[13]) +
            m[3] * (m[9] * m[14] - m[10] * m[13])
            );

        det += m[8] * (
            m[1] * (m[6] * m[15] - m[7] * m[14]) -
            m[2] * (m[5] * m[15] - m[7] * m[13]) +
            m[3] * (m[5] * m[14] - m[6] * m[13])
            );

        det -= m[12] * (
            m[1] * (m[6] * m[11] - m[7] * m[10]) -
            m[2] * (m[5] * m[11] - m[7] * m[9]) +
            m[3] * (m[5] * m[10] - m[6] * m[9])
            );

        return det;
    }

    Matrix4x4 Matrix4x4::Inversed() const
    {
        Matrix4x4 result = *this;
        result.Inverse();
        return result;
    }

    Matrix4x4& Matrix4x4::Inverse()
    {
        float det = Determinant();
        if (IsZero(det))
            throw std::runtime_error("Matrix4: Matrix is singular (non-invertible)");

        float inv[16];

        inv[0] = m[5] * m[10] * m[15] -
            m[5] * m[11] * m[14] -
            m[9] * m[6] * m[15] +
            m[9] * m[7] * m[14] +
            m[13] * m[6] * m[11] -
            m[13] * m[7] * m[10];

        inv[4] = -m[4] * m[10] * m[15] +
            m[4] * m[11] * m[14] +
            m[8] * m[6] * m[15] -
            m[8] * m[7] * m[14] -
            m[12] * m[6] * m[11] +
            m[12] * m[7] * m[10];

        inv[8] = m[4] * m[9] * m[15] -
            m[4] * m[11] * m[13] -
            m[8] * m[5] * m[15] +
            m[8] * m[7] * m[13] +
            m[12] * m[5] * m[11] -
            m[12] * m[7] * m[9];

        inv[12] = -m[4] * m[9] * m[14] +
            m[4] * m[10] * m[13] +
            m[8] * m[5] * m[14] -
            m[8] * m[6] * m[13] -
            m[12] * m[5] * m[10] +
            m[12] * m[6] * m[9];

        inv[1] = -m[1] * m[10] * m[15] +
            m[1] * m[11] * m[14] +
            m[9] * m[2] * m[15] -
            m[9] * m[3] * m[14] -
            m[13] * m[2] * m[11] +
            m[13] * m[3] * m[10];

        inv[5] = m[0] * m[10] * m[15] -
            m[0] * m[11] * m[14] -
            m[8] * m[2] * m[15] +
            m[8] * m[3] * m[14] +
            m[12] * m[2] * m[11] -
            m[12] * m[3] * m[10];

        inv[9] = -m[0] * m[9] * m[15] +
            m[0] * m[11] * m[13] +
            m[8] * m[1] * m[15] -
            m[8] * m[3] * m[13] -
            m[12] * m[1] * m[11] +
            m[12] * m[3] * m[9];

        inv[13] = m[0] * m[9] * m[14] -
            m[0] * m[10] * m[13] -
            m[8] * m[1] * m[14] +
            m[8] * m[2] * m[13] +
            m[12] * m[1] * m[10] -
            m[12] * m[2] * m[9];

        inv[2] = m[1] * m[6] * m[15] -
            m[1] * m[7] * m[14] -
            m[5] * m[2] * m[15] +
            m[5] * m[3] * m[14] +
            m[13] * m[2] * m[7] -
            m[13] * m[3] * m[6];

        inv[6] = -m[0] * m[6] * m[15] +
            m[0] * m[7] * m[14] +
            m[4] * m[2] * m[15] -
            m[4] * m[3] * m[14] -
            m[12] * m[2] * m[7] +
            m[12] * m[3] * m[6];

        inv[10] = m[0] * m[5] * m[15] -
            m[0] * m[7] * m[13] -
            m[4] * m[1] * m[15] +
            m[4] * m[3] * m[13] +
            m[12] * m[1] * m[7] -
            m[12] * m[3] * m[5];

        inv[14] = -m[0] * m[5] * m[14] +
            m[0] * m[6] * m[13] +
            m[4] * m[1] * m[14] -
            m[4] * m[2] * m[13] -
            m[12] * m[1] * m[6] +
            m[12] * m[2] * m[5];

        inv[3] = -m[1] * m[6] * m[11] +
            m[1] * m[7] * m[10] +
            m[5] * m[2] * m[11] -
            m[5] * m[3] * m[10] -
            m[9] * m[2] * m[7] +
            m[9] * m[3] * m[6];

        inv[7] = m[0] * m[6] * m[11] -
            m[0] * m[7] * m[10] -
            m[4] * m[2] * m[11] +
            m[4] * m[3] * m[10] +
            m[8] * m[2] * m[7] -
            m[8] * m[3] * m[6];

        inv[11] = -m[0] * m[5] * m[11] +
            m[0] * m[7] * m[9] +
            m[4] * m[1] * m[11] -
            m[4] * m[3] * m[9] -
            m[8] * m[1] * m[7] +
            m[8] * m[3] * m[5];

        inv[15] = m[0] * m[5] * m[10] -
            m[0] * m[6] * m[9] -
            m[4] * m[1] * m[10] +
            m[4] * m[2] * m[9] +
            m[8] * m[1] * m[6] -
            m[8] * m[2] * m[5];

        det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
        det = 1.0f / det;

        for (int i = 0; i < 16; ++i)
            m[i] = inv[i] * det;

        return *this;
    }


    Matrix4x4::Matrix4x4 ( const Vector3D & translation, const Quaternion & rotation, const Vector3D & scale )
        {
            // Нормализуем кватернион для безопасности
        Quaternion rot = rotation.Normalized ();

        // Получаем матрицу поворота из кватерниона
        float xx = rot.x * rot.x;
        float yy = rot.y * rot.y;
        float zz = rot.z * rot.z;
        float xy = rot.x * rot.y;
        float xz = rot.x * rot.z;
        float yz = rot.y * rot.z;
        float wx = rot.w * rot.x;
        float wy = rot.w * rot.y;
        float wz = rot.w * rot.z;

        // Матрица 3x3 поворота с применением масштаба
        // Первая колонка (ось X) с масштабом по X
        m[ 0 ] = ( 1.0f - 2.0f * ( yy + zz ) ) * scale.x;
        m[ 1 ] = ( 2.0f * ( xy + wz ) ) * scale.x;
        m[ 2 ] = ( 2.0f * ( xz - wy ) ) * scale.x;
        m[ 3 ] = 0.0f;

        // Вторая колонка (ось Y) с масштабом по Y
        m[ 4 ] = ( 2.0f * ( xy - wz ) ) * scale.y;
        m[ 5 ] = ( 1.0f - 2.0f * ( xx + zz ) ) * scale.y;
        m[ 6 ] = ( 2.0f * ( yz + wx ) ) * scale.y;
        m[ 7 ] = 0.0f;

        // Третья колонка (ось Z) с масштабом по Z
        m[ 8 ] = ( 2.0f * ( xz + wy ) ) * scale.z;
        m[ 9 ] = ( 2.0f * ( yz - wx ) ) * scale.z;
        m[ 10 ] = ( 1.0f - 2.0f * ( xx + yy ) ) * scale.z;
        m[ 11 ] = 0.0f;

        // Четвёртая колонка (позиция)
        m[ 12 ] = translation.x;
        m[ 13 ] = translation.y;
        m[ 14 ] = translation.z;
        m[ 15 ] = 1.0f;
        }

    Matrix4x4::Matrix4x4 ( const Vector3D & translation, const Quaternion & rotation )
        : Matrix4x4 ( translation, rotation, Vector3D ( 1.0f, 1.0f, 1.0f ) )
        {
            // Делегируем конструктору с единичным масштабом
        }



    Matrix4x4& Matrix4x4::Identity()
    {
        for (int i = 0; i < 16; ++i)
            m[i] = 0.0f;

        m[0] = m[5] = m[10] = m[15] = 1.0f;
        return *this;
    }

    bool Matrix4x4::IsIdentity() const
    {
        for (int i = 0; i < 16; ++i)
        {
            if (i == 0 || i == 5 || i == 10 || i == 15)
            {
                if (!IsEqual(m[i], 1.0f))
                    return false;
            }
            else
            {
                if (!IsZero(m[i]))
                    return false;
            }
        }
        return true;
    }
    
    bool Matrix4x4::IsInvertible() const
    {
        return !IsZero(Determinant());
    }

    bool Matrix4x4::IsOrthogonal() const
    {
        Matrix4x4 transposed = Transposed();
        Matrix4x4 product = *this * transposed;
        return product.IsIdentity();
    }

    bool Matrix4x4::IsAffine() const
    {
        return IsEqual(m[3], 0.0f) && IsEqual(m[7], 0.0f) &&
            IsEqual(m[11], 0.0f) && IsEqual(m[15], 1.0f);
    }

    
    Vector3D Matrix4x4::GetTranslation() const
    {
        return Vector3D(m[12], m[13], m[14]);
    }

    Vector3D Matrix4x4::GetScale() const
    {
        float scaleX = Sqrt(m[0] * m[0] + m[1] * m[1] + m[2] * m[2]);
        float scaleY = Sqrt(m[4] * m[4] + m[5] * m[5] + m[6] * m[6]);
        float scaleZ = Sqrt(m[8] * m[8] + m[9] * m[9] + m[10] * m[10]);

        return Vector3D(scaleX, scaleY, scaleZ);
    }

    Matrix4x4 Matrix4x4::GetRotation() const
    {
        Matrix4x4 rotation;
        Vector3D scale = GetScale();

        if (IsZero(scale.x) || IsZero(scale.y) || IsZero(scale.z))
            return IdentityMatrix();

        
        for (int i = 0; i < 3; ++i)
        {
            rotation.m[i] = m[i] / scale.x;
            rotation.m[i + 4] = m[i + 4] / scale.y;
            rotation.m[i + 8] = m[i + 8] / scale.z;
        }

        rotation.m[12] = rotation.m[13] = rotation.m[14] = 0.0f;
        rotation.m[15] = 1.0f;

        return rotation;
    }

    void Matrix4x4::SetTranslation(const Vector3D& translation)
    {
        m[12] = translation.x;
        m[13] = translation.y;
        m[14] = translation.z;
    }

    void Matrix4x4::SetScale(const Vector3D& scale)
    {
        Vector3D currentX(m[0], m[1], m[2]);
        Vector3D currentY(m[4], m[5], m[6]);
        Vector3D currentZ(m[8], m[9], m[10]);

        if (!currentX.IsZero()) currentX.Normalize();
        if (!currentY.IsZero()) currentY.Normalize();
        if (!currentZ.IsZero()) currentZ.Normalize();

        m[0] = currentX.x * scale.x; m[1] = currentX.y * scale.x; m[2] = currentX.z * scale.x;
        m[4] = currentY.x * scale.y; m[5] = currentY.y * scale.y; m[6] = currentY.z * scale.y;
        m[8] = currentZ.x * scale.z; m[9] = currentZ.y * scale.z; m[10] = currentZ.z * scale.z;
    }
   
    Matrix4x4& Matrix4x4::Translate(const Vector3D& translation)
    {
        return Translate(translation.x, translation.y, translation.z);
    }

    Matrix4x4& Matrix4x4::Translate(float x, float y, float z)
    {
        Matrix4x4 translationMat = Translation(x, y, z);
        *this = *this * translationMat;  
        return *this;
    }

    Matrix4x4& Matrix4x4::Scale(const Vector3D& scale)
    {
        return Scale(scale.x, scale.y, scale.z);
    }

    Matrix4x4& Matrix4x4::Scale(float x, float y, float z)
    {
        Matrix4x4 scaleMat = Scaling(x, y, z);
        *this = *this * scaleMat;  
        return *this;
    }

    Matrix4x4& Matrix4x4::Scale(float uniformScale)
    {
		*this *= Scaling(uniformScale, uniformScale, uniformScale);
		
        return *this; // TODO:
    }

    Matrix4x4& Matrix4x4::RotateX(float radians)
    {
        Matrix4x4 rotationMat = RotationX(radians);
        *this = *this * rotationMat;  
        return *this;
    }

    Matrix4x4& Matrix4x4::RotateY(float radians)
    {
        Matrix4x4 rotationMat = RotationY(radians);
        *this = *this * rotationMat;  
        return *this;
    }

    Matrix4x4& Matrix4x4::RotateZ(float radians)
    {
        Matrix4x4 rotationMat = RotationZ(radians);
        *this = *this * rotationMat;  
        return *this;
    }

    Matrix4x4& Matrix4x4::Rotate(const Vector3D& axis, float radians)
    {
        Matrix4x4 rotationMat = Rotation(axis, radians);
        *this = *this * rotationMat;  
        return *this;
    }

    Matrix4x4& Matrix4x4::LookAt(const Vector3D& eye, const Vector3D& target, const Vector3D& up)
    {
        *this = LookAtMatrix(eye, target, up);
        return *this;
    }

    Matrix4x4& Matrix4x4::Perspective(float fovY, float aspect, float zNear, float zFar)
    {
        *this = PerspectiveMatrix(fovY, aspect, zNear, zFar);
        return *this;
    }

    Matrix4x4& Matrix4x4::Orthographic(float left, float right, float bottom, float top,
        float zNear, float zFar)
    {
        *this = OrthographicMatrix(left, right, bottom, top, zNear, zFar);
        return *this;
    }
   
    Matrix4x4 Matrix4x4::Zero()
    {
        Matrix4x4 mat;
        for (int i = 0; i < 16; ++i)
            mat.m[i] = 0.0f;
        return mat;
    }

    Matrix4x4 Matrix4x4::IdentityMatrix()
    {
        return Matrix4x4(1.0f);
    }

    Matrix4x4 Matrix4x4::Translation(const Vector3D& translation)
    {
        return Translation(translation.x, translation.y, translation.z);
    }

    Matrix4x4 Matrix4x4::Translation(float x, float y, float z)
    {
        return Matrix4x4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            x, y, z, 1.0f
        );
    }

    Matrix4x4 Matrix4x4::Scaling(const Vector3D& scale)
    {
        return Scaling(scale.x, scale.y, scale.z);
    }

    Matrix4x4 Matrix4x4::Scaling(float x, float y, float z)
    {
        return Matrix4x4(
            x, 0.0f, 0.0f, 0.0f,
            0.0f, y, 0.0f, 0.0f,
            0.0f, 0.0f, z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    Matrix4x4 Matrix4x4::Scaling(float uniformScale)
    {
        return Scaling(uniformScale, uniformScale, uniformScale);
    }

    Matrix4x4 Matrix4x4::RotationX(float radians)
    {
        float cosA = Cos(radians);
        float sinA = Sin(radians);

        return Matrix4x4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, cosA, sinA, 0.0f,
            0.0f, -sinA, cosA, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    Matrix4x4 Matrix4x4::RotationY(float radians)
    {
        float cosA = Cos(radians);
        float sinA = Sin(radians);

        return Matrix4x4(
            cosA, 0.0f, -sinA, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            sinA, 0.0f, cosA, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    Matrix4x4 Matrix4x4::RotationZ(float radians)
    {
        float cosA = Cos(radians);
        float sinA = Sin(radians);

        return Matrix4x4(
            cosA, sinA, 0.0f, 0.0f,
            -sinA, cosA, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    Matrix4x4 Matrix4x4::Rotation(const Vector3D& axis, float radians)
    {
        Vector3D normalizedAxis = axis.Normalized();
        float x = normalizedAxis.x;
        float y = normalizedAxis.y;
        float z = normalizedAxis.z;

        float cosA = Cos(radians);
        float sinA = Sin(radians);
        float oneMinusCosA = 1.0f - cosA;

        return Matrix4x4(
            cosA + x * x * oneMinusCosA, y * x * oneMinusCosA + z * sinA, z * x * oneMinusCosA - y * sinA, 0.0f,
            x * y * oneMinusCosA - z * sinA, cosA + y * y * oneMinusCosA, z * y * oneMinusCosA + x * sinA, 0.0f,
            x * z * oneMinusCosA + y * sinA, y * z * oneMinusCosA - x * sinA, cosA + z * z * oneMinusCosA, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    Matrix4x4 Matrix4x4::LookAtMatrix ( const Vector3D & eye, const Vector3D & target, const Vector3D & up )
        {
            // Вычисляем направление взгляда
        Vector3D zAxis = eye - target;

        // Защита от нулевого вектора
        float len = zAxis.Length ();
        if (CEMath::IsZero ( len ))
            {
            zAxis = Vector3D ( 0.0f, 0.0f, 1.0f ); // Смотрим вдоль Z по умолчанию
            }
        else
            {
            zAxis.Normalize ();
            }

            // Вычисляем правую ось
        Vector3D xAxis = up.Cross ( zAxis );
        len = xAxis.Length ();
        if (CEMath::IsZero ( len ))
            {
                // Если up и zAxis коллинеарны (смотрим строго вверх/вниз)
                // Используем стандартную правую ось
            xAxis = Vector3D ( 1.0f, 0.0f, 0.0f );
            }
        else
            {
            xAxis.Normalize ();
            }

            // Вычисляем верхнюю ось (ортогональную к xAxis и zAxis)
        Vector3D yAxis = zAxis.Cross ( xAxis );
        // yAxis автоматически нормализован

        return Matrix4x4 (
            xAxis.x, yAxis.x, zAxis.x, 0.0f,
            xAxis.y, yAxis.y, zAxis.y, 0.0f,
            xAxis.z, yAxis.z, zAxis.z, 0.0f,
            -xAxis.Dot ( eye ), -yAxis.Dot ( eye ), -zAxis.Dot ( eye ), 1.0f
        );
        }

    Matrix4x4 Matrix4x4::PerspectiveMatrix(float fovY, float aspect, float zNear, float zFar)
    {
        float tanHalfFov = Tan(fovY * 0.5f);
        float range = zNear - zFar;

        return Matrix4x4(
            -1.0f / (aspect * tanHalfFov), 0.0f, 0.0f, 0.0f,
            0.0f, -1.0f / tanHalfFov, 0.0f, 0.0f,
            0.0f, 0.0f, (zNear + zFar) / range, -1.0f,
            0.0f, 0.0f, (2.0f * zNear * zFar) / range, 0.0f
        );
    }

    Matrix4x4 Matrix4x4::OrthographicMatrix(float left, float right, float bottom, float top,
        float zNear, float zFar)
    {
        float width = right - left;
        float height = top - bottom;
        float depth = zFar - zNear;

        return Matrix4x4(
            -2.0f / width, 0.0f, 0.0f, 0.0f,
            0.0f, -2.0f / height, 0.0f, 0.0f,
            0.0f, 0.0f, -2.0f / depth, 0.0f,
            -(right + left) / width, -(top + bottom) / height, -(zFar + zNear) / depth, 1.0f
        );
    }
   
    void Matrix4x4::ToArray(float out[16]) const
    {
        for (int i = 0; i < 16; ++i)
            out[i] = m[i];
    }

    std::array<float, 16> Matrix4x4::ToStdArray() const
    {
        std::array<float, 16> arr;
        for (int i = 0; i < 16; ++i)
            arr[i] = m[i];
        return arr;
    }

   
    Vector4D operator*(const Vector4D& vec, const Matrix4x4& mat)
    {
        return Vector4D(
            vec.x * mat[0] + vec.y * mat[1] + vec.z * mat[2] + vec.w * mat[3],
            vec.x * mat[4] + vec.y * mat[5] + vec.z * mat[6] + vec.w * mat[7],
            vec.x * mat[8] + vec.y * mat[9] + vec.z * mat[10] + vec.w * mat[11],
            vec.x * mat[12] + vec.y * mat[13] + vec.z * mat[14] + vec.w * mat[15]
        );
    }

    Vector3D operator*(const Vector3D& vec, const Matrix4x4& mat)
    {
        
        float x = vec.x * mat[0] + vec.y * mat[4] + vec.z * mat[8] + mat[12];
        float y = vec.x * mat[1] + vec.y * mat[5] + vec.z * mat[9] + mat[13];
        float z = vec.x * mat[2] + vec.y * mat[6] + vec.z * mat[10] + mat[14];
        float w = vec.x * mat[3] + vec.y * mat[7] + vec.z * mat[11] + mat[15];

        if (!CEMath::IsZero(w) && !CEMath::IsEqual(w, 1.0f))
        {
            float invW = 1.0f / w;
            x *= invW;
            y *= invW;
            z *= invW;
        }

        return Vector3D(x, y, z);
    }

    Matrix4x4 operator*(float scalar, const Matrix4x4& mat)
    {
        return mat * scalar;
    }

    std::ostream& operator<<(std::ostream& os, const Matrix4x4& mat)
    {
        os << std::fixed << std::setprecision(6);
        os << "Matrix4x4 [" << std::endl;

        for (int row = 0; row < 4; ++row)
        {
            os << "  ";
            for (int col = 0; col < 4; ++col)
            {
                os << std::setw(12) << mat(row, col);
                if (col < 3) os << ", ";
            }
            if (row < 3) os << ",";
            os << std::endl;
        }
        os << "]";

        return os;
    }

    std::istream& operator>>(std::istream& is, Matrix4x4& mat)
    {
        std::string line;

        
        while (is && (is.peek() == ' ' || is.peek() == '\t' || is.peek() == '\n' ||
            is.peek() == 'M' || is.peek() == '['))
        {
            char c = static_cast<char>(is.get());
            if (c == '[') break;
        }

        
        float values[16];
        int count = 0;

        for (int i = 0; i < 16; ++i)
        {
            if (!(is >> values[i]))
            {
                
                is.clear();
                char c;
                is >> c;
                if (c == ',' || c == ']' || c == '[' || c == '(' || c == ')')
                {
                    --i; 
                    continue;
                }
                else
                {
                    is.putback(c);
                    break;
                }
            }
            count++;

           
            if (is.peek() == ',') is.ignore();
        }

        
        if (count == 16)
        {
            for (int i = 0; i < 16; ++i)
            {
                mat.m[i] = values[i];
            }
        }

        
        while (is && (is.peek() == ' ' || is.peek() == '\t' || is.peek() == '\n' ||
            is.peek() == ']' || is.peek() == ')'))
        {
            is.ignore();
        }

        return is;
    }
}