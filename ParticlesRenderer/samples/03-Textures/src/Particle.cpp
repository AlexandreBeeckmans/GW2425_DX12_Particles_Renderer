#include "Particle.h"

#include "dx12lib/Material.h"
#include <TestApplication.h>
#include "dx12lib/Helpers.h"


Particle::Particle(Vec3 startPos, float size) :
    m_Size { size },
    m_Speed { 0.25f }
{
    m_PositionMatrix = DirectX::XMMatrixTranslation( startPos.X, startPos.Y, startPos.Z );
    m_ScaleMatrix = DirectX::XMMatrixScaling( m_Size, m_Size, m_Size );

    m_Direction.X = Math::GetRandomInRange( -1, 1 );
    m_Direction.Y = Math::GetRandomInRange( -1, 1 );
    m_Direction.Z = 0;
    m_Direction.Normalize();

    m_PerpendicularDirection.X = m_Direction.Y;
    m_PerpendicularDirection.Y = -m_Direction.X;
    m_PerpendicularDirection.Z = 0;
    m_PerpendicularDirection.Normalize();
    m_perpendicularSpeed = Math::GetRandomInRange( 0.25f, 2.25f );
}

void Particle::Update(float deltaTime, const Camera& camera, bool isAccelerationEnabled, bool isPerpendicularEnabled)
{
    Translate( deltaTime );   
    DirectX::XMMATRIX rotationMatrix { DirectX::XMMatrixIdentity() };
    DirectX::XMMATRIX worldMatrix { m_ScaleMatrix * rotationMatrix * m_PositionMatrix };

    DirectX::XMMATRIX viewMatrix { camera.get_ViewMatrix() };
    DirectX::XMMATRIX viewProjectionMatrix { viewMatrix * camera.get_ProjectionMatrix() };

    Math::ComputeMatrices( worldMatrix, viewMatrix, viewProjectionMatrix, m_Matrices );

    if (!isAccelerationEnabled) return;
    Accelerate( deltaTime );

    if (!isPerpendicularEnabled) return;
    MovePerpendicular(deltaTime);
}

DirectX::XMFLOAT3 Particle::GetPosition() const
{
    DirectX::XMFLOAT3 position;
    DirectX::XMStoreFloat3( &position, m_PositionMatrix.r[3] );
    return position;
}

void Particle::Translate(float deltaTime)
{
    const float offset { m_Speed * deltaTime };
    const float offsetX { m_Direction.X * offset };
    const float offsetY { m_Direction.Y * offset };
    const float offsetZ { m_Direction.Z * offset };

    const DirectX::XMMATRIX translationMatrix { DirectX::XMMatrixTranslation( offsetX, offsetY, offsetZ ) };

    m_PositionMatrix = DirectX::XMMatrixMultiply( m_PositionMatrix, translationMatrix );
}

void Particle::Accelerate( float deltaTime )
{
    m_Speed += m_Acceleration * deltaTime;
}

void Particle::MovePerpendicular( float deltaTime )
{
    m_AccumulatedPerpendicularTime += deltaTime;
    if (m_AccumulatedPerpendicularTime >= Math::PI * 2)
    {
        m_AccumulatedPerpendicularTime -= Math::PI * 2;
    }

    const float offset { sinf(m_AccumulatedPerpendicularTime) * m_perpendicularSpeed * deltaTime};
    const float offsetX { m_PerpendicularDirection.X * offset };
    const float offsetY { m_PerpendicularDirection.Y * offset };
    const float offsetZ { m_PerpendicularDirection.Z * offset };

    const DirectX::XMMATRIX translationMatrix { DirectX::XMMatrixTranslation( offsetX, offsetY, offsetZ ) };
    m_PositionMatrix = XMMatrixMultiply( m_PositionMatrix, translationMatrix );
}

