#pragma once
#include <DirectXMath.h>

struct Mat
{
    DirectX::XMMATRIX ModelMatrix;
    DirectX::XMMATRIX ModelViewMatrix;
    DirectX::XMMATRIX InverseTransposeModelViewMatrix;
    DirectX::XMMATRIX ModelViewProjectionMatrix;
};

struct Vec3
{
    float X;
    float Y;
    float Z;

    float Length() const
    {
        return sqrtf( X * X + Y * Y + Z * Z );
    }

    void Normalize()
    {
        X /= Length();
        Y /= Length();
        Z /= Length();
    }
};

namespace Math
{
	float GetRandomInRange( float min, float max );
}