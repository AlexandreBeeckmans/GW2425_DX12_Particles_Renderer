#pragma once
#include <vector>


namespace dx12lib
{
    class Device;
    class Scene;
    class CommandList;
    class Texture;
}
class Particle;
class SceneVisitor;
class Camera;

class ParticleSystem
{
public:
    ParticleSystem( float particleSize, bool isAccelerationEnabled, bool isPerpendicularEnabled );
    ~ParticleSystem();

    void Initialize( dx12lib::CommandList& commandList );

    void Update(float deltaTime, const Camera& camera, FPSCounter& fpsCounter, const MemoryCounter& memCounter);
    void Render( dx12lib::Device& device, dx12lib::CommandList& commandList, SceneVisitor& visitor, const Camera& camera, bool isMeshShader ) const;

    void AddParticle();
    void AddParticleAmount( int amount );

    

private:

    std::vector<DirectX::XMFLOAT3> GetAllPos() const;
    std::vector<DirectX::XMMATRIX> GetAllMatrices() const;

    void TraditionalRender( dx12lib::CommandList& commandList, SceneVisitor& visitor, const Camera& camera ) const;
    void MeshShaderRender( dx12lib::Device& device, dx12lib::CommandList& commandList) const;

    Vec3 m_Pos { 0, 3, 0 };
    std::vector<Particle> m_Particles;


    std::shared_ptr<dx12lib::Scene>   m_Plane;
    std::shared_ptr<dx12lib::Texture> m_DefaultTexture;

    float accumulatedTime{0};
    float intervalTime = 10.0f;

    float m_ParticlesSize;
    bool  m_IsAccelerationEnabled;
    bool  m_IsPerpendicularEnabled;
};