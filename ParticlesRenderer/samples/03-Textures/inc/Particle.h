#pragma once
#include "Camera.h"
#include "Mat.h"

class SceneVisitor;


namespace dx12lib
{
    class Scene;
    class CommandList;
    class Texture;
}

class Particle
{
public:
    Particle( Vec3 startPos, float size = 0.1f);

    void Update( float deltaTime, const Camera& camera, bool isAccelerationEnabled, bool isPerpendicularEnabled );

    Mat GetMatrices() const
    {
        return m_Matrices;
    }
    DirectX::XMMATRIX GetMVPMatrix()const
    {
        return m_Matrices.ModelViewProjectionMatrix;
    }

    DirectX::XMFLOAT3 GetPosition() const;

private:

    void Translate( float deltaTime);
    void  Accelerate( float deltaTime );
    void MovePerpendicular( float deltaTime );

    //modified parameters for the test
    float m_Size;
    float m_Speed;

    static constexpr float m_Acceleration { 0.05f };

    //random
    Vec3 m_Direction;
    Vec3 m_PerpendicularDirection;


    float m_AccumulatedPerpendicularTime {};
    float m_perpendicularSpeed {};


    Mat m_Matrices;

    DirectX::XMMATRIX m_PositionMatrix;
    DirectX::XMMATRIX m_ScaleMatrix;
};
