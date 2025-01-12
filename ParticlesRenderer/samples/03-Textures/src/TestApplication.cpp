#include <dx12lib/d3dx12.h>

#include <TestApplication.h>

#include <SceneVisitor.h>

#include <GameFramework/GameFramework.h>
#include <GameFramework/Window.h>

#include <dx12lib/CommandList.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/Device.h>
#include <dx12lib/GUI.h>
#include <dx12lib/Helpers.h>
#include <dx12lib/RootSignature.h>
#include <dx12lib/Scene.h>
#include <dx12lib/SwapChain.h>
#include <dx12lib/Texture.h>

#include <imgui.h>

#include <wrl.h>
using namespace Microsoft::WRL;

#include <d3dcompiler.h>

using namespace dx12lib;
using namespace DirectX;

#include <algorithm>  // For std::min, std::max, and std::clamp.
#include <functional>  // For std::bind
#include <string>// For std::wstring


struct LightProperties
{
    uint32_t NumPointLights;
    uint32_t NumSpotLights;
};


// Builds a look-at (world) matrix from a point, up and direction vectors.
XMMATRIX XM_CALLCONV LookAtMatrix( FXMVECTOR Position, FXMVECTOR Direction, FXMVECTOR Up )
{
    assert( !XMVector3Equal( Direction, XMVectorZero() ) );
    assert( !XMVector3IsInfinite( Direction ) );
    assert( !XMVector3Equal( Up, XMVectorZero() ) );
    assert( !XMVector3IsInfinite( Up ) );

    XMVECTOR R2 = XMVector3Normalize( Direction );

    XMVECTOR R0 = XMVector3Cross( Up, R2 );
    R0          = XMVector3Normalize( R0 );

    XMVECTOR R1 = XMVector3Cross( R2, R0 );

    XMMATRIX M( R0, R1, R2, Position );

    return M;
}

TestApplication::TestApplication( const std::wstring& name, uint32_t width, uint32_t height, bool vSync )
: m_ScissorRect( CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ) )
, m_Viewport( CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>( width ), static_cast<float>( height ) ) )
, m_Forward( 0 )
, m_Backward( 0 )
, m_Left( 0 )
, m_Right( 0 )
, m_Up( 0 )
, m_Down( 0 )
, m_Pitch( 0 )
, m_Yaw( 0 )
, m_Shift( false )
, m_Width( static_cast<int>(width) )
, m_Height( static_cast<int>(height) )
, m_VSync( vSync )
{
    m_Logger = GameFramework::Get().CreateLogger( "Textures" );
    m_Window = GameFramework::Get().CreateWindow( name, width, height );

    m_Window->Update += UpdateEvent::slot( &TestApplication::OnUpdate, this );
    m_Window->KeyPressed += KeyboardEvent::slot( &TestApplication::OnKeyPressed, this );
    m_Window->KeyReleased += KeyboardEvent::slot( &TestApplication::OnKeyReleased, this );
    m_Window->MouseMoved += MouseMotionEvent::slot( &TestApplication::OnMouseMoved, this );
    m_Window->MouseWheel += MouseWheelEvent::slot( &TestApplication::OnMouseWheel, this );
    m_Window->Resize += ResizeEvent::slot( &TestApplication::OnResize, this );

    XMVECTOR cameraPos { XMVectorSet( 0, 5, -50, 1 ) };
    XMVECTOR cameraTarget { XMVectorSet( 0, 5, 0, 1 ) };
    XMVECTOR cameraUp { XMVectorSet( 0, 1, 0, 0 ) };

    m_Camera.set_LookAt( cameraPos, cameraTarget, cameraUp );
    m_Camera.set_FoV( 90.0f );

    m_pAlignedCameraData = static_cast<CameraData*>(_aligned_malloc( sizeof( CameraData ), 16 ));

    m_pAlignedCameraData->m_InitialCamPos = m_Camera.get_Translation();
    m_pAlignedCameraData->m_InitialCamRot = m_Camera.get_Rotation();
}

TestApplication::~TestApplication()
{
    _aligned_free( m_pAlignedCameraData );
}

uint32_t TestApplication::Run()
{
    LoadContent();

    m_Window->Show();

    uint32_t retCode {static_cast<uint32_t>(GameFramework::Get().Run()) };

    UnloadContent();

    return retCode;
}

bool TestApplication::LoadContent()
{
    // Create the DX12 device.
    m_Device = Device::Create();

    // Create a swap chain.
    m_SwapChain = m_Device->CreateSwapChain( m_Window->GetWindowHandle(), DXGI_FORMAT_R8G8B8A8_UNORM );
    m_SwapChain->SetVSync( m_VSync );

    auto& commandQueue = m_Device->GetCommandQueue( m_IsUsingMeshShaders ? D3D12_COMMAND_LIST_TYPE_DIRECT : D3D12_COMMAND_LIST_TYPE_COPY );

    m_CommandList  = commandQueue.GetCommandList();


    //Init particles
    m_ParticleSystem.Initialize(*m_CommandList);

    //init pipeline
    if (!m_IsUsingMeshShaders)
    {
        CreateGeometryShaderPipeline( commandQueue );  // geometry
    }
    else
    {
        CreateMeshShaderPipeline( commandQueue ); //mesh
    }
    

    InitializeColors();


    commandQueue.Flush();  // Wait for loading operations to complete before rendering the first frame.

    return true;
}

void TestApplication::OnResize( const ResizeEventArgs& event )
{
    m_Width  = std::max( 1, event.Width );
    m_Height = std::max( 1, event.Height );

    m_SwapChain->Resize( m_Width, m_Height );

    float aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
    m_Camera.set_Projection( 45.0f, aspectRatio, 0.1f, 100.0f );

    m_Viewport = CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>( m_Width ), static_cast<float>( m_Height ) );

    m_RenderTarget.Resize( m_Width, m_Height );
}

void TestApplication::CreateGeometryShaderPipeline( CommandQueue& commandQueue )
{
    // Start loading resources...
    commandQueue.ExecuteCommandList( m_CommandList );

    // Load the vertex shader.
    ComPtr<ID3DBlob> vertexShaderBlob{};
    ThrowIfFailed( D3DReadFileToBlob( L"data/shaders/03-Textures/VertexShader.cso", &vertexShaderBlob ) );

    // Load the pixel shader.
    ComPtr<ID3DBlob> pixelShaderBlob{};
    ThrowIfFailed( D3DReadFileToBlob( L"data/shaders/03-Textures/PixelShader.cso", &pixelShaderBlob ) );

    // Load the pixel shader.
    ComPtr<ID3DBlob> geometryShaderBlob {};
    ThrowIfFailed( D3DReadFileToBlob( L"data/shaders/03-Textures/GeometryShader.cso", &geometryShaderBlob ) );

    CreateRootSignature(D3D12_SHADER_VISIBILITY_ALL, D3D12_SHADER_VISIBILITY_GEOMETRY, D3D12_SHADER_VISIBILITY_VERTEX);

    // Setup the pipeline state.
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
        CD3DX12_PIPELINE_STATE_STREAM_GS                    GS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
    } pipelineStateStream{};

    // Check the best multisample quality level that can be used for the given back buffer format.
    m_SampleDesc = m_Device->GetMultisampleQualityLevels( m_BackbufferFormat );

    D3D12_RT_FORMAT_ARRAY rtvFormats {};
    rtvFormats.NumRenderTargets      = 1;
    rtvFormats.RTFormats[0]          = m_BackbufferFormat;

    pipelineStateStream.pRootSignature        = m_RootSignature->GetD3D12RootSignature().Get();
    pipelineStateStream.InputLayout           = VertexPositionTexture::InputLayout;
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    pipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE( vertexShaderBlob.Get() );
    pipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE( pixelShaderBlob.Get() );
    pipelineStateStream.GS                    = CD3DX12_SHADER_BYTECODE( geometryShaderBlob.Get() );
    pipelineStateStream.DSVFormat             = m_DepthBufferFormat;
    pipelineStateStream.RTVFormats            = rtvFormats;
    pipelineStateStream.SampleDesc            = m_SampleDesc;

    m_PipelineState = m_Device->CreatePipelineStateObject( pipelineStateStream );
}

void TestApplication::CreateMeshShaderPipeline( dx12lib::CommandQueue& commandQueue )
{
    commandQueue.ExecuteCommandList( m_CommandList );

    // Load the vertex shader.
    ComPtr<ID3DBlob> meshShaderBlob{};
    ThrowIfFailed( D3DReadFileToBlob( L"data/shaders/03-Textures/MeshShader.cso", &meshShaderBlob ) );

    // Load the pixel shader.
    ComPtr<ID3DBlob> pixelShaderBlob{};
    ThrowIfFailed( D3DReadFileToBlob( L"data/shaders/03-Textures/PixelShader.cso", &pixelShaderBlob ) );


    CreateRootSignature( D3D12_SHADER_VISIBILITY_MESH, D3D12_SHADER_VISIBILITY_MESH, D3D12_SHADER_VISIBILITY_MESH );

    // Setup the pipeline state for mesh shader.
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
        CD3DX12_PIPELINE_STATE_STREAM_MS                    MS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
    } pipelineStateStream;

    // Check the best multisample quality level that can be used for the given back buffer format.
    m_SampleDesc = m_Device->GetMultisampleQualityLevels( m_BackbufferFormat );

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets      = 1;
    rtvFormats.RTFormats[0]          = m_BackbufferFormat;

    pipelineStateStream.pRootSignature = m_RootSignature->GetD3D12RootSignature().Get();
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.PS         = CD3DX12_SHADER_BYTECODE( pixelShaderBlob.Get() );
    pipelineStateStream.MS         = CD3DX12_SHADER_BYTECODE( meshShaderBlob.Get() );  // Mesh shader
    pipelineStateStream.DSVFormat  = m_DepthBufferFormat;
    pipelineStateStream.RTVFormats = rtvFormats;
    pipelineStateStream.SampleDesc = m_SampleDesc;


    
    m_PipelineState = m_Device->CreatePipelineStateObject( pipelineStateStream );
    m_CommandList->SetPipelineState( m_PipelineState );
    m_CommandList->SetGraphicsRootSignature( m_RootSignature );
}

void TestApplication::UpdateCamera( float deltaTime )
{
    float speedMultipler = ( m_Shift ? 16.0f : 4.0f );

    XMVECTOR cameraTranslate = XMVectorSet( m_Right - m_Left, 0.0f, m_Forward - m_Backward, 1.0f ) * speedMultipler * deltaTime;
    XMVECTOR cameraPan =
        XMVectorSet( 0.0f, m_Up - m_Down, 0.0f, 1.0f ) * speedMultipler * deltaTime;
    m_Camera.Translate( cameraTranslate, Space::Local );
    m_Camera.Translate( cameraPan, Space::Local );

    XMVECTOR cameraRotation =
        XMQuaternionRotationRollPitchYaw( XMConvertToRadians( m_Pitch ), XMConvertToRadians( m_Yaw ), 0.0f );
    m_Camera.set_Rotation( cameraRotation );
}

void TestApplication::InitializeColors()
{
    // Create an off-screen render target with a single color buffer and a depth buffer.
    auto colorDesc = CD3DX12_RESOURCE_DESC::Tex2D( m_BackbufferFormat, m_Width, m_Height, 1, 1, m_SampleDesc.Count,
                                                   m_SampleDesc.Quality, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET );
    D3D12_CLEAR_VALUE colorClearValue;
    colorClearValue.Format   = colorDesc.Format;
    colorClearValue.Color[0] = 0.4f;
    colorClearValue.Color[1] = 0.6f;
    colorClearValue.Color[2] = 0.9f;
    colorClearValue.Color[3] = 1.0f;

    auto colorTexture = m_Device->CreateTexture( colorDesc, &colorClearValue );
    colorTexture->SetName( L"Color Render Target" );

    // Create a depth buffer.
    auto depthDesc = CD3DX12_RESOURCE_DESC::Tex2D( m_DepthBufferFormat, m_Width, m_Height, 1, 1, m_SampleDesc.Count,
                                                   m_SampleDesc.Quality, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL );
    D3D12_CLEAR_VALUE depthClearValue;
    depthClearValue.Format       = depthDesc.Format;
    depthClearValue.DepthStencil = { 1.0f, 0 };

    auto depthTexture = m_Device->CreateTexture( depthDesc, &depthClearValue );
    depthTexture->SetName( L"Depth Render Target" );

    // Attach the textures to the render target.
    m_RenderTarget.AttachTexture( AttachmentPoint::Color0, colorTexture );
    m_RenderTarget.AttachTexture( AttachmentPoint::DepthStencil, depthTexture );
}

void TestApplication::CreateRootSignature( const D3D12_SHADER_VISIBILITY& matricesVisibility,
                                     const D3D12_SHADER_VISIBILITY& fovSizeParticlesVisibility,
                                     const D3D12_SHADER_VISIBILITY& matrixVisibility )
{
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags
    {
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
    };



    CD3DX12_DESCRIPTOR_RANGE1 descriptorRange( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2 );
    CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters] {};
    rootParameters[RootParameters::MaterialCB].InitAsConstantBufferView( 0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                                                                         D3D12_SHADER_VISIBILITY_PIXEL );
    rootParameters[RootParameters::Textures].InitAsDescriptorTable( 1, &descriptorRange,
                                                                    D3D12_SHADER_VISIBILITY_PIXEL );

    rootParameters[RootParameters::MatricesCB].InitAsConstantBufferView( 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                                                                         matricesVisibility );
    rootParameters[RootParameters::FOVSizeAndNBParticles].InitAsConstants( 3, 1, 0, fovSizeParticlesVisibility );
    rootParameters[RootParameters::MatricesSRV].InitAsShaderResourceView( 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                                                                          matrixVisibility );

    CD3DX12_STATIC_SAMPLER_DESC linearRepeatSampler( 0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR );

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription {};
    rootSignatureDescription.Init_1_1( RootParameters::NumRootParameters, rootParameters, 1, &linearRepeatSampler, rootSignatureFlags );

    m_RootSignature = m_Device->CreateRootSignature( rootSignatureDescription.Desc_1_1 );
}

void TestApplication::UnloadContent()
{
    m_RenderTarget.Reset();

    m_RootSignature.reset();
    m_PipelineState.reset();

    //m_GUI.reset();
    m_SwapChain.reset();
    m_Device.reset();
}

void TestApplication::OnUpdate( UpdateEventArgs& e )
{
    if(!m_FPSCounter.Update( static_cast<float>(e.DeltaTime) ))
    {
        GameFramework::Get().Stop();
    };

    m_SwapChain->WaitForSwapChain();
    m_ParticleSystem.Update(e.DeltaTime, m_Camera, m_FPSCounter, m_MemoryCounter);

    OnRender();
    UpdateCamera( static_cast<float>( e.DeltaTime ) );
}

void XM_CALLCONV Math::ComputeMatrices(const FXMMATRIX& model, CXMMATRIX view, CXMMATRIX viewProjection, Mat& mat )
{
    mat.ModelMatrix                     = model;
    mat.ModelViewMatrix                 = model * view;
    mat.InverseTransposeModelViewMatrix = XMMatrixTranspose( XMMatrixInverse( nullptr, mat.ModelViewMatrix ) );
    mat.ModelViewProjectionMatrix       = model * viewProjection;
}

void TestApplication::OnRender()
{
    auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_DIRECT );
    auto  commandList  = commandQueue.GetCommandList();

    // Create a scene visitor that is used to perform the actual rendering of the meshes in the scenes.
    SceneVisitor visitor( *commandList );

    // Clear the render targets.
    {
        FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

        commandList->ClearTexture( m_RenderTarget.GetTexture( AttachmentPoint::Color0 ), clearColor );
        commandList->ClearDepthStencilTexture( m_RenderTarget.GetTexture( AttachmentPoint::DepthStencil ),
                                               D3D12_CLEAR_FLAG_DEPTH );
    }

    commandList->SetPipelineState( m_PipelineState );
    commandList->SetGraphicsRootSignature( m_RootSignature );


    commandList->SetViewport( m_Viewport );
    commandList->SetScissorRect( m_ScissorRect );

    commandList->SetRenderTarget( m_RenderTarget );

    m_ParticleSystem.Render(*m_Device, *commandList, visitor,m_Camera, m_IsUsingMeshShaders);


    // Resolve the MSAA render target to the swapchain's backbuffer.
    auto& swapChainRT         = m_SwapChain->GetRenderTarget();
    auto  swapChainBackBuffer = swapChainRT.GetTexture( AttachmentPoint::Color0 );
    auto  msaaRenderTarget    = m_RenderTarget.GetTexture( AttachmentPoint::Color0 );

    commandList->ResolveSubresource( swapChainBackBuffer, msaaRenderTarget );


    commandQueue.ExecuteCommandList( commandList );

    m_SwapChain->Present();
}



static bool g_AllowFullscreenToggle = true;

void TestApplication::OnKeyPressed( KeyEventArgs& e )
{
    if ( !ImGui::GetIO().WantCaptureKeyboard )
    {
        switch ( e.Key )
        {
        case KeyCode::Escape:
            GameFramework::Get().Stop();
            break;
        case KeyCode::Enter:
            if ( e.Alt )
            {
            case KeyCode::F11:
                if ( g_AllowFullscreenToggle )
                {
                    m_Window->ToggleFullscreen();
                    g_AllowFullscreenToggle = false;
                }
                break;
            }
        case KeyCode::V:
            m_SwapChain->ToggleVSync();
            break;
        case KeyCode::R:
            // Reset camera transform
            m_Camera.set_Translation( m_pAlignedCameraData->m_InitialCamPos );
            m_Camera.set_Rotation( m_pAlignedCameraData->m_InitialCamRot );
            m_Pitch = 0.0f;
            m_Yaw   = 0.0f;
            break;
        case KeyCode::Up:
        case KeyCode::W:
            m_Forward = 1.0f;
            break;
        case KeyCode::Left:
        case KeyCode::A:
            m_Left = 1.0f;
            break;
        case KeyCode::Down:
        case KeyCode::S:
            m_Backward = 1.0f;
            break;
        case KeyCode::Right:
        case KeyCode::D:
            m_Right = 1.0f;
            break;
        case KeyCode::Q:
            m_Down = 1.0f;
            break;
        case KeyCode::E:
            m_Up = 1.0f;
            break;
        case KeyCode::ShiftKey:
            m_Shift = true;
            break;
        }
    }
}

void TestApplication::OnKeyReleased( KeyEventArgs& e )
{
    switch ( e.Key )
    {
    case KeyCode::Enter:
        if ( e.Alt )
        {
        case KeyCode::F11:
            g_AllowFullscreenToggle = true;
        }
        break;
    case KeyCode::Up:
    case KeyCode::W:
        m_Forward = 0.0f;
        break;
    case KeyCode::Left:
    case KeyCode::A:
        m_Left = 0.0f;
        break;
    case KeyCode::Down:
    case KeyCode::S:
        m_Backward = 0.0f;
        break;
    case KeyCode::Right:
    case KeyCode::D:
        m_Right = 0.0f;
        break;
    case KeyCode::Q:
        m_Down = 0.0f;
        break;
    case KeyCode::E:
        m_Up = 0.0f;
        break;
    case KeyCode::ShiftKey:
        m_Shift = false;
        break;
    }
}

void TestApplication::OnMouseMoved( MouseMotionEventArgs& e )
{
    const float mouseSpeed = 0.1f;

    if ( e.LeftButton )
    {
        m_Pitch -= e.RelY * mouseSpeed;

        m_Pitch = std::clamp( m_Pitch, -90.0f, 90.0f );

        m_Yaw -= e.RelX * mouseSpeed;
    }
}

void TestApplication::OnMouseWheel( MouseWheelEventArgs& e )
{
    auto fov = m_Camera.get_FoV();
    
    fov -= e.WheelDelta;
    fov = std::clamp( fov, 12.0f, 90.0f );
    
    m_Camera.set_FoV( fov );
    
    m_Logger->info( "FoV: {:.7}", fov );
}
