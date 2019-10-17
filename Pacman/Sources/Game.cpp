#include "pch.h"

#include "Global.h"
#include "Game.h"

extern void ExitGame();

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept :
  m_window(nullptr),
  m_outputWidth(800),
  m_outputHeight(600),
  m_featureLevel(D3D_FEATURE_LEVEL_9_1),
  m_movement(Movement::Stop)
{
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_window = window;
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateDevice();
    CreateResources();

    m_world.Init(m_d3dDevice.Get());
    m_dots.Init(m_d3dDevice.Get());
    m_pacman.Init(m_d3dDevice.Get());

    m_pacman.SetPosition(10.5, 0.25, 9.5);

    m_keyboard = std::make_unique<Keyboard>();

    m_camera.SetPosition(10.5f, 15.0f, 10.5f);
    //m_camera.SetPosition(10.5f, 5.0f, -2.5f);
    //m_camera.SetPosition(3, 2, 3);
    m_camera.SetLookAtPos(XMFLOAT3(10.5, 0, 10.5));

    m_camera.SetProjectionValues(75.0f, static_cast<float>(m_outputWidth) / static_cast<float>(m_outputHeight), 0.1f, 1000.0f); // Here or to resize?

    // Camera constant buffers
    Global::CameraConstantBuffer cameraConstantBuffer;

    cameraConstantBuffer.world = DirectX::XMMatrixIdentity();
    cameraConstantBuffer.view = DirectX::XMMatrixTranspose(m_camera.GetViewMatrix());
    cameraConstantBuffer.projection = DirectX::XMMatrixTranspose(m_camera.GetProjectionMatrix());

    D3D11_BUFFER_DESC cbd = {};
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.MiscFlags = 0;
    cbd.ByteWidth = sizeof(Global::CameraConstantBuffer);
    cbd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA csd = {};
    csd.pSysMem = &cameraConstantBuffer;

    m_d3dDevice->CreateBuffer(&cbd, &csd, &m_constantBuffer);

    // Camera constant buffers v2
    Global::CameraPerFrame cameraConstantBufferPerFrame;

    //cameraConstantBuffer.world = DirectX::XMMatrixIdentity();
    cameraConstantBufferPerFrame.view = DirectX::XMMatrixTranspose(m_camera.GetViewMatrix());
    cameraConstantBufferPerFrame.projection = DirectX::XMMatrixTranspose(m_camera.GetProjectionMatrix());

    D3D11_BUFFER_DESC cbd_v2 = {};
    cbd_v2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd_v2.Usage = D3D11_USAGE_DYNAMIC;
    cbd_v2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd_v2.MiscFlags = 0;
    cbd_v2.ByteWidth = sizeof(Global::CameraPerFrame);
    cbd_v2.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA csd_v2 = {};
    csd_v2.pSysMem = &cameraConstantBufferPerFrame;

    m_d3dDevice->CreateBuffer(&cbd_v2, &csd_v2, &m_cameraPerFrame);

    Global::CameraPerObject cameraConstantBufferPerObject;
    cameraConstantBufferPerObject.world = DirectX::XMMatrixIdentity();

    cbd_v2.ByteWidth = sizeof(Global::CameraPerObject);

    csd_v2.pSysMem = &cameraConstantBufferPerObject;

    m_d3dDevice->CreateBuffer(&cbd_v2, &csd_v2, &m_cameraPerObject);

    // Frame constant buffers
    Global::FrameConstantBuffer frameConstantBuffer;

    frameConstantBuffer.frameID = DirectX::XMFLOAT2(1, 0);
    frameConstantBuffer.framesNumber = DirectX::XMFLOAT2(2, 1); // Two frames on X axis and one frame on Y
    frameConstantBuffer.billboardSize_0_0_0 = DirectX::XMFLOAT4(1, 0, 0, 0);

    cbd = {};
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.MiscFlags = 0;
    cbd.ByteWidth = sizeof(Global::FrameConstantBuffer);
    cbd.StructureByteStride = 0;

    csd = {};
    csd.pSysMem = &frameConstantBuffer;

    m_d3dDevice->CreateBuffer(&cbd, &csd, &m_frameBuffer);

    ID3D11Buffer* geometryShaderBuffers[2] = { m_constantBuffer.Get(), m_frameBuffer.Get() };
    m_shaderManager->BindConstantBuffersToGeometryShader(ShaderManager::GeometryShader::Billboard, geometryShaderBuffers, 2);

    ID3D11Buffer* vertexShaderBuffers[2] = { m_cameraPerFrame.Get(), m_cameraPerObject.Get() };
    m_shaderManager->BindConstantBuffersToVertexShader(ShaderManager::VertexShader::Instanced, vertexShaderBuffers, 2);

    //ID3D11Buffer* vertexShaderBuffers_v2[1] = {m_constantBuffer.Get()};
    m_shaderManager->BindConstantBuffersToVertexShader(ShaderManager::VertexShader::Indexed, vertexShaderBuffers, 2);


    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
}

// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

void Game::Update(DX::StepTimer const& timer)
{
  (void)timer;

  auto kb = m_keyboard->GetState();

  if (kb.Escape)
    ExitGame();

  DirectX::XMFLOAT3 pacmanPosition = m_pacman.GetPosition();
  pacmanPosition.x = floor(pacmanPosition.x * 10) / 10.0f;
  pacmanPosition.z = floor(pacmanPosition.z * 10) / 10.0f;

  bool isVerticallyAligned = fmod(pacmanPosition.z - 0.5f, 1.0f) < 0.01f ? true : false;
  bool isHorizontallyAligned = fmod(pacmanPosition.x - 0.5f, 1.0f) < 0.01f ? true : false;

  if (isVerticallyAligned)
  {
    if (kb.Right)
      m_movementRequest = Movement::Right;
    else if (kb.Left)
      m_movementRequest = Movement::Left;
  }

  if (isHorizontallyAligned)
  {
    if (kb.Up)
      m_movementRequest = Movement::Up;
    else if (kb.Down)
      m_movementRequest = Movement::Down;
  }

  switch (m_movementRequest)
  {
    case Movement::Right:
      if (m_world.IsPassable(static_cast<uint8_t>(floor(pacmanPosition.x + 0.55f)), static_cast<uint8_t>(floor(pacmanPosition.z))))
      {
        m_movement = m_movementRequest;
      }
      else
      {
        if (m_movementRequest == m_movement)
        {
          m_pacman.AlignToMap();
          m_movement = Movement::Stop;
        }
        else
          m_movementRequest = m_movement;
      }
      break;
    case Movement::Left:
      if (m_world.IsPassable(static_cast<uint8_t>(floor(pacmanPosition.x - 0.55f)), static_cast<uint8_t>(floor(pacmanPosition.z))))
      {
        m_movement = m_movementRequest;
      }
      else
      {
        if (m_movementRequest == m_movement)
        {
          m_pacman.AlignToMap();
          m_movement = Movement::Stop;
        }
        else
          m_movementRequest = m_movement;
      }
      break;
    case Movement::Up:
      if (m_world.IsPassable(static_cast<uint8_t>(floor(pacmanPosition.x)), static_cast<uint8_t>(floor(pacmanPosition.z + 0.55f))))
      {
        m_movement = m_movementRequest;
      }
      else
      {
        if (m_movementRequest == m_movement)
        {
          m_pacman.AlignToMap();
          m_movement = Movement::Stop;
        }
        else
          m_movementRequest = m_movement;
      }
      break;
    case Movement::Down:
      if (m_world.IsPassable(static_cast<uint8_t>(floor(pacmanPosition.x)), static_cast<uint8_t>(floor(pacmanPosition.z - 0.55f))))
      {
        m_movement = m_movementRequest;
      }
      else
      {
        if (m_movementRequest == m_movement)
        {
          m_pacman.AlignToMap();
          m_movement = Movement::Stop;
        }
        else
          m_movementRequest = m_movement;
      }
      break;
    default:
      // Nothing
      break;
  }

  switch (m_movement)
  {
    case Movement::Right:
      m_pacman.SetDirection(0);
      m_pacman.AdjustPosition(Global::speed, 0, 0);
      break;
    case Movement::Left:
      m_pacman.SetDirection(1);
      m_pacman.AdjustPosition(-Global::speed, 0, 0);
      break;
    case Movement::Up:
      m_pacman.SetDirection(2);
      m_pacman.AdjustPosition(0, 0, Global::speed);
      break;
    case Movement::Down:
      m_pacman.SetDirection(3);
      m_pacman.AdjustPosition(0, 0, -Global::speed);
      break;
    default:
      // Nothing
      break;
  }

  m_dots.Update(static_cast<uint8_t>(floor(pacmanPosition.x)), static_cast<uint8_t>(floor(pacmanPosition.z)), m_d3dContext.Get());
}

// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    DrawWorld();
    DrawSprites();

    Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    // Clear the views.
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::Black);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);

    m_shaderManager->ClearShadersFromThePipeline();
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = m_swapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr);
    }
}

// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowSizeChanged(int width, int height)
{
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}

void Game::DrawWorld()
{
  m_shaderManager->SetVertexShader(ShaderManager::VertexShader::Indexed);
  //m_shaderManager->SetPixelShader(ShaderManager::PixelShader::Flat);
  m_shaderManager->SetPixelShader(ShaderManager::PixelShader::Phong);

  Global::CameraPerObject cameraPerObjectConstantBuffer;
  cameraPerObjectConstantBuffer.world = m_world.GetWorldMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  m_world.Draw(m_d3dContext.Get());
}

void Game::DrawSprites()
{
  // Draw dots
  m_shaderManager->SetVertexShader(ShaderManager::VertexShader::Instanced);
  m_shaderManager->SetGeometryShader(ShaderManager::GeometryShader::Billboard);
  m_shaderManager->SetPixelShader(ShaderManager::PixelShader::Texture);

  Global::FrameConstantBuffer frameConstantBuffer;
  frameConstantBuffer.frameID = DirectX::XMFLOAT2(0, 0);
  frameConstantBuffer.framesNumber = DirectX::XMFLOAT2(1, 1);
  frameConstantBuffer.billboardSize_0_0_0 = DirectX::XMFLOAT4(0.25f, 0, 0, 0);

  m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &frameConstantBuffer, sizeof(frameConstantBuffer));

  Global::CameraPerObject cameraPerObjectConstantBuffer;
  cameraPerObjectConstantBuffer.world = m_dots.GetWorldMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  m_dots.Draw(m_d3dContext.Get());

  // Draw pacman
  if (m_timer.GetFrameCount() % 10 == 0)
    m_pacman.Update();

  frameConstantBuffer.frameID = DirectX::XMFLOAT2(static_cast<float>(m_pacman.GetFrame()), m_pacman.GetDirection());
  frameConstantBuffer.framesNumber = DirectX::XMFLOAT2(2, 4);
  frameConstantBuffer.billboardSize_0_0_0 = DirectX::XMFLOAT4(Global::pacManSize, 0, 0, 0);

  m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &frameConstantBuffer, sizeof(frameConstantBuffer));

  cameraPerObjectConstantBuffer.world = m_pacman.GetWorldMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  m_pacman.Draw(m_d3dContext.Get());
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
    UINT creationFlags = 0;

#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    static const D3D_FEATURE_LEVEL featureLevels [] =
    {
        // TODO: Modify for supported Direct3D feature levels
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    // Create the DX11 API device object, and get a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    DX::ThrowIfFailed(D3D11CreateDevice(
        nullptr,                            // specify nullptr to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        device.ReleaseAndGetAddressOf(),    // returns the Direct3D device created
        &m_featureLevel,                    // returns feature level of device created
        context.ReleaseAndGetAddressOf()    // returns the device immediate context
        ));

#ifndef NDEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(device.As(&d3dDebug)))
    {
        ComPtr<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D11_MESSAGE_ID hide [] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // TODO: Add more message IDs here as needed.
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    DX::ThrowIfFailed(device.As(&m_d3dDevice));
    DX::ThrowIfFailed(context.As(&m_d3dContext));

    m_shaderManager = std::make_unique<ShaderManager>(m_d3dDevice.Get(), m_d3dContext.Get());
}


// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews [] = { nullptr };
    m_d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

    UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    UINT backBufferCount = 2;

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain)
    {
        HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            OnDeviceLost();

            // Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method
            // and correctly set up the new device.
            return;
        }
        else
        {
            DX::ThrowIfFailed(hr);
        }
    }
    else
    {
        // First, retrieve the underlying DXGI Device from the D3D Device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        ComPtr<IDXGIAdapter> dxgiAdapter;
        DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        // And obtain the factory object that created it.
        ComPtr<IDXGIFactory2> dxgiFactory;
        DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = backBufferCount;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        // Create a SwapChain from a Win32 window.
        DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
            m_d3dDevice.Get(),
            m_window,
            &swapChainDesc,
            &fsSwapChainDesc,
            nullptr,
            m_swapChain.ReleaseAndGetAddressOf()
            ));

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtain the backbuffer for this window which will be the final 3D rendertarget.
    ComPtr<ID3D11Texture2D> backBuffer;
    DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

    // Create a view interface on the rendertarget to use on bind.
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

    // Allocate a 2-D surface as the depth/stencil buffer and
    // create a DepthStencil view on this surface to use on bind.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    // TODO: Initialize windows-size dependent objects here.
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.

    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();

    CreateDevice();

    CreateResources();
}
