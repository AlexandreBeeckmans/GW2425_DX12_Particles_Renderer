#pragma once

#include "Mat.h"
#include "MemoryCounter.h"
#include "FPSCounter.h"

#include <Camera.h>

#include <GameFramework/Events.h>
#include <GameFramework/GameFramework.h>

#include <dx12lib/RenderTarget.h>

#include <cstdint>  // For uint32_t
#include <memory>   // For std::unique_ptr and std::smart_ptr
#include <string>   // For std::wstring

#include<ParticleSystem.h>

#include <d3dx12.h>  // For CD3DX12_ROOT_PARAMETER1 and related utilities

namespace Math
{
void XM_CALLCONV ComputeMatrices( const DirectX::FXMMATRIX& model, DirectX::CXMMATRIX view, DirectX::CXMMATRIX viewProjection, Mat& mat );
}

enum RootParameters
{
    MatricesCB,         // ConstantBuffer<Mat> MatCB : register(b0);
    MaterialCB,         // ConstantBuffer<Material> MaterialCB : register( b0, space1 );
    Textures,           // Texture2D DiffuseTexture : register( t2 );
    FOVSizeAndNBParticles,        //(b1)
    MatricesSRV,
    NumRootParameters
};

namespace dx12lib
{
    class CommandQueue;
    class CommandList;
    class Device;
    class GUI;
    class Mesh;
    class RootSignature;
    class PipelineStateObject;
    class Scene;
    class SwapChain;
    class Texture;
}
class Window;  // From GameFramework

class TestApplication
{
public:
    TestApplication( const std::wstring& name, uint32_t width, uint32_t height, bool vSync = false );
    virtual ~TestApplication();

    /**
     * Start the game loop and return the error code.
     */
    uint32_t Run();

    /**
     *  Load content required for the demo.
     */
    bool LoadContent();

    /**
     *  Unload demo specific content that was loaded in LoadContent.
     */
    void UnloadContent();

protected:
    void OnUpdate( UpdateEventArgs& e );
    void OnRender();


    void OnKeyPressed( KeyEventArgs& e );
    virtual void OnKeyReleased( KeyEventArgs& e );
    virtual void OnMouseMoved( MouseMotionEventArgs& e );
    void OnMouseWheel( MouseWheelEventArgs& e );
    void OnResize( const ResizeEventArgs& event );

private:

    void CreateGeometryShaderPipeline( dx12lib::CommandQueue& commandQueue);
    void CreateMeshShaderPipeline( dx12lib::CommandQueue& commandQueue );

    void UpdateCamera(float deltaTime);
    void InitializeColors();

    void CreateRootSignature( const D3D12_SHADER_VISIBILITY& matricesVisibility,
                              const D3D12_SHADER_VISIBILITY& fovSizeParticlesVisibility,
                              const D3D12_SHADER_VISIBILITY& matrixVisibility );


    std::shared_ptr<Window> m_Window;  // Render window (from GameFramework)

    // DX12 Device.
    std::shared_ptr<dx12lib::Device>    m_Device;
    std::shared_ptr<dx12lib::SwapChain> m_SwapChain;


    // Render target
    dx12lib::RenderTarget m_RenderTarget;

    // Root signature
    std::shared_ptr<dx12lib::RootSignature> m_RootSignature;

    // Pipeline state object.
    std::shared_ptr<dx12lib::PipelineStateObject> m_PipelineState;

    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT     m_ScissorRect;

    Camera m_Camera;
    struct alignas( 16 ) CameraData
    {
        DirectX::XMVECTOR m_InitialCamPos;
        DirectX::XMVECTOR m_InitialCamRot;
    };
    CameraData* m_pAlignedCameraData;

    // Camera controller
    float m_Forward;
    float m_Backward;
    float m_Left;
    float m_Right;
    float m_Up;
    float m_Down;

    float m_Pitch;
    float m_Yaw;

    // Set to true if the Shift key is pressed.
    bool m_Shift;

    int  m_Width;
    int  m_Height;
    bool m_VSync;

    // Logger for logging messages
    Logger m_Logger;


    //test parameters
    static constexpr bool m_IsUsingMeshShaders { true };
    static constexpr float m_ParticlesSize { 0.5f };
    static constexpr bool  m_IsAccelerationEnabled { false };
    static constexpr bool  m_IsPerpendicularEnabled { false };

    //Implementation specific
    ParticleSystem m_ParticleSystem{ m_ParticlesSize, m_IsAccelerationEnabled, m_IsPerpendicularEnabled };
    std::shared_ptr<dx12lib::CommandList> m_CommandList;

    DXGI_FORMAT m_BackbufferFormat { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
    DXGI_FORMAT m_DepthBufferFormat { DXGI_FORMAT_D32_FLOAT };
    DXGI_SAMPLE_DESC m_SampleDesc {};
    FPSCounter       m_FPSCounter {};
    MemoryCounter    m_MemoryCounter {};
};