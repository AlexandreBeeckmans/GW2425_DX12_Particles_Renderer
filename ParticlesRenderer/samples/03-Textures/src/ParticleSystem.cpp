#include "Particle.h"
#include "TestApplication.h"

#include<SceneVisitor.h>

#include<ParticleSystem.h>

#include "../../DX12Lib/inc/dx12lib/CommandList.h"
#include "../../DX12Lib/inc/dx12lib/Scene.h"
#include "dx12lib/Material.h"
#include "dx12lib/StructuredBuffer.h"

#include <execution>

ParticleSystem::ParticleSystem(float particleSize, bool isAccelerationEnabled, bool isPerpendicularEnabled) :
    m_ParticlesSize { particleSize },
    m_IsAccelerationEnabled { isAccelerationEnabled },
    m_IsPerpendicularEnabled { isPerpendicularEnabled }
{
    AddParticleAmount( 500 );
}

ParticleSystem::~ParticleSystem()
{
    m_Plane.reset();
    m_DefaultTexture.reset();
}

void ParticleSystem::Initialize( dx12lib::CommandList& commandList )
{
    m_Plane          = commandList.CreatePoint();
    m_DefaultTexture = commandList.LoadTextureFromFile( L"Assets/Textures/explosion.tga", true );
}

void ParticleSystem::Update( float deltaTime, const Camera& camera, FPSCounter& fpsCounter, const MemoryCounter& memCounter)
{
    std::for_each
    ( 
        std::execution::par,
        m_Particles.begin(),
        m_Particles.end(),
        [this, deltaTime, &camera]( Particle& particle ) 
        {
            particle.Update( deltaTime, camera, m_IsAccelerationEnabled, m_IsPerpendicularEnabled );
        } 
    );

    accumulatedTime += deltaTime;
    if (accumulatedTime > intervalTime)
    {
        memCounter.Update( m_Particles.size() );
        accumulatedTime -= intervalTime;
        AddParticleAmount( m_Particles.size() );
        fpsCounter.UpdateSample( m_Particles.size() );
    }
}

void ParticleSystem::Render( dx12lib::Device& device, dx12lib::CommandList& commandList, SceneVisitor& visitor, const Camera& camera, bool isMeshShader ) const
{
    commandList.SetGraphicsDynamicConstantBuffer( RootParameters::MaterialCB, dx12lib::Material::White );
    commandList.SetShaderResourceView( RootParameters::Textures, 0, m_DefaultTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

    float constants[3] { camera.get_FoV(), m_ParticlesSize, static_cast<float>( m_Particles.size() ) };
    commandList.SetGraphics32BitConstants( RootParameters::FOVSizeAndNBParticles, 3, &constants );

    if (!isMeshShader)
    {
        TraditionalRender( commandList, visitor, camera );
        return;
    }
    MeshShaderRender( device, commandList);
}

void ParticleSystem::TraditionalRender( dx12lib::CommandList& commandList, SceneVisitor& visitor, const Camera& camera ) const
{

    for ( Particle particle: m_Particles )
    {
        const DirectX::XMFLOAT3 position = particle.GetPosition();
        commandList.SetGraphicsDynamicConstantBuffer( RootParameters::MatricesCB, particle.GetMatrices().ModelViewProjectionMatrix );
        m_Plane->Accept( visitor );
    }
}

void ParticleSystem::MeshShaderRender(dx12lib::Device& device, dx12lib::CommandList& commandList ) const
{
    //Upload all the particle matrices
    std::vector<DirectX::XMMATRIX> particleMatrices { GetAllMatrices() };
    std::shared_ptr<dx12lib::StructuredBuffer> matricesBuffer {};
    dx12lib::StructuredBuffer::UploadDataToStructuredBuffer( device, matricesBuffer, particleMatrices.data(), particleMatrices.size() * sizeof(DirectX::XMMATRIX) );
    commandList.SetShaderResourceView( RootParameters::MatricesSRV, matricesBuffer, D3D12_RESOURCE_STATE_GENERIC_READ );

    //Perform Draw
    int numParticles      = m_Particles.size();
    int particlesPerGroup = 1;

    int numDispatches = ( numParticles + particlesPerGroup - 1 ) / particlesPerGroup;
    commandList.MeshShaderDraw( numParticles / particlesPerGroup);
}

void ParticleSystem::AddParticle()
{
    m_Particles.emplace_back( Particle {m_Pos} );
}

void ParticleSystem::AddParticleAmount( int amount )
{
    for ( int i {0}; i < amount; ++i )
    {
        AddParticle();
    }
}

std::vector<DirectX::XMFLOAT3> ParticleSystem::GetAllPos() const
{
    std::vector<DirectX::XMFLOAT3> pos {};
    for (const Particle& particle : m_Particles)
    {
        pos.emplace_back( particle.GetPosition() );
    }
    return pos;
}

std::vector<DirectX::XMMATRIX> ParticleSystem::GetAllMatrices() const
{
    std::vector<DirectX::XMMATRIX> mat {};
    for ( const Particle& particle: m_Particles )
    {
        mat.emplace_back( particle.GetMVPMatrix() );
    }
    return mat;
}