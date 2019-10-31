#include "pch.h"

#include "Game.h"
#include "WICTextureLoader.h"

extern void ExitGame();

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept :
  m_window(nullptr),
  m_outputWidth(800),
  m_outputHeight(600),
  m_featureLevel(D3D_FEATURE_LEVEL_9_1),
  m_pacmanMovementRequest(Character::Movement::Stop),
  m_debugDraw(false),
  m_currentMode(Mode::Scatter)
{
}

void Game::Initialize(HWND window, int width, int height)
{
  m_window = window;
  m_outputWidth = std::max(width, 1);
  m_outputHeight = std::max(height, 1);

  CreateDevice();
  CreateResources();

  m_world.Init(m_d3dDevice.Get());
  m_dots.Init(m_d3dDevice.Get());

  m_characters[Characters::Pacman] = std::make_unique<Character>();
  m_characters[Characters::Pacman]->Init(m_d3dDevice.Get());
  m_characters[Characters::Pacman]->SetPosition(10.5f, 0.25f, 9.5f);
  m_characters[Characters::Pacman]->SetColumnsAndRowsOfAssociatedSpriteSheet(8, 1);
  m_characters[Characters::Pacman]->SetSpriteScaleFactor(Global::pacManSize);
  m_characters[Characters::Pacman]->SetFramesPerState(2);

  m_characters[Characters::Blinky] = std::make_unique<Character>();
  m_characters[Characters::Blinky]->Init(m_d3dDevice.Get());
  m_characters[Characters::Blinky]->SetPosition(10.5f, 0.3f, 13.5f);
  m_characters[Characters::Blinky]->SetMovement(Character::Movement::Left);
  m_characters[Characters::Blinky]->SetColumnsAndRowsOfAssociatedSpriteSheet(8, 4);
  m_characters[Characters::Blinky]->SetSpriteScaleFactor(Global::ghostSize);
  m_characters[Characters::Blinky]->SetFramesPerState(2);

  m_characters[Characters::Pinky] = std::make_unique<Character>();
  m_characters[Characters::Pinky]->Init(m_d3dDevice.Get());
  m_characters[Characters::Pinky]->SetPosition(11.5f, 0.3f, 13.5f);
  m_characters[Characters::Pinky]->SetMovement(Character::Movement::Left);
  m_characters[Characters::Pinky]->SetColumnsAndRowsOfAssociatedSpriteSheet(8, 4);
  m_characters[Characters::Pinky]->SetSpriteScaleFactor(Global::ghostSize);
  m_characters[Characters::Pinky]->SetFramesPerState(2);

  m_characters[Characters::Inky] = std::make_unique<Character>();
  m_characters[Characters::Inky]->Init(m_d3dDevice.Get());
  m_characters[Characters::Inky]->SetPosition(12.5f, 0.3f, 13.5f);
  m_characters[Characters::Inky]->SetMovement(Character::Movement::Left);
  m_characters[Characters::Inky]->SetColumnsAndRowsOfAssociatedSpriteSheet(8, 4);
  m_characters[Characters::Inky]->SetSpriteScaleFactor(Global::ghostSize);
  m_characters[Characters::Inky]->SetFramesPerState(2);

  m_characters[Characters::Clyde] = std::make_unique<Character>();
  m_characters[Characters::Clyde]->Init(m_d3dDevice.Get());
  m_characters[Characters::Clyde]->SetPosition(13.5f, 0.3f, 13.5f);
  m_characters[Characters::Clyde]->SetMovement(Character::Movement::Left);
  m_characters[Characters::Clyde]->SetColumnsAndRowsOfAssociatedSpriteSheet(8, 4);
  m_characters[Characters::Clyde]->SetSpriteScaleFactor(Global::ghostSize);
  m_characters[Characters::Clyde]->SetFramesPerState(2);

  DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), nullptr, L"Resources/pacman.png", m_pacManResource.GetAddressOf(), m_pacManShaderResourceView.GetAddressOf()));
  DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), nullptr, L"Resources/ghosts.png", m_ghostsResource.GetAddressOf(), m_ghostsShaderResourceView.GetAddressOf()));

  m_keyboard = std::make_unique<Keyboard>();

  m_camera.SetPosition(10.5f, 15.0f, 10.5f);
  //m_camera.SetPosition(10.5f, 5.0f, -2.5f);
  //m_camera.SetPosition(3, 2, 3);
  m_camera.SetLookAtPos(XMFLOAT3(10.5, 0, 10.5));

  m_camera.SetProjectionValues(75.0f, static_cast<float>(m_outputWidth) / static_cast<float>(m_outputHeight), 0.1f, 1000.0f); // Here or to resize?

  // Camera constant buffers
  XMMATRIX projection = DirectX::XMMatrixTranspose(m_camera.GetProjectionMatrix());

  D3D11_BUFFER_DESC cbd = {};
  cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbd.Usage = D3D11_USAGE_DYNAMIC;
  cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  cbd.MiscFlags = 0;
  cbd.ByteWidth = sizeof(XMMATRIX);
  cbd.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA csd = {};
  csd.pSysMem = &projection;

  m_d3dDevice->CreateBuffer(&cbd, &csd, &m_projectionMatrixConstantBuffer);

  // Camera constant buffers
  Global::CameraPerFrame cameraConstantBufferPerFrame = {};
  XMStoreFloat4x4(&cameraConstantBufferPerFrame.view, XMMatrixTranspose(m_camera.GetViewMatrix()));
  XMStoreFloat4x4(&cameraConstantBufferPerFrame.projection, XMMatrixTranspose(m_camera.GetProjectionMatrix()));

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
  XMStoreFloat4x4(&cameraConstantBufferPerObject.world, DirectX::XMMatrixIdentity());

  cbd_v2.ByteWidth = sizeof(Global::CameraPerObject);

  csd_v2.pSysMem = &cameraConstantBufferPerObject;

  m_d3dDevice->CreateBuffer(&cbd_v2, &csd_v2, &m_cameraPerObject);

  // Frame constant buffers
  Global::SpriteConstantBuffer frameConstantBuffer = {};

  cbd = {};
  cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbd.Usage = D3D11_USAGE_DYNAMIC;
  cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  cbd.MiscFlags = 0;
  cbd.ByteWidth = sizeof(Global::SpriteConstantBuffer);
  cbd.StructureByteStride = 0;

  csd = {};
  csd.pSysMem = &frameConstantBuffer;

  m_d3dDevice->CreateBuffer(&cbd, &csd, &m_frameBuffer);

  ID3D11Buffer* geometryShaderBuffers[2] = { m_projectionMatrixConstantBuffer.Get(), m_frameBuffer.Get() };
  m_shaderManager->BindConstantBuffersToGeometryShader(ShaderManager::GeometryShader::Billboard, geometryShaderBuffers, 2);

  ID3D11Buffer* vertexShaderBuffers[2] = { m_cameraPerFrame.Get(), m_cameraPerObject.Get() };
  m_shaderManager->BindConstantBuffersToVertexShader(ShaderManager::VertexShader::Instanced, vertexShaderBuffers, 2);
  m_shaderManager->BindConstantBuffersToVertexShader(ShaderManager::VertexShader::Indexed, vertexShaderBuffers, 2);
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
  if (timer.GetTotalSeconds() > 7 && timer.GetTotalSeconds() <= 27)
  {
    if (m_currentMode != Mode::Chase)
    {
      m_characters[Characters::Blinky]->RevereseMovementDirection();
      m_characters[Characters::Pinky]->RevereseMovementDirection();
      m_characters[Characters::Inky]->RevereseMovementDirection();
      m_characters[Characters::Clyde]->RevereseMovementDirection();
    }

    m_currentMode = Mode::Chase;
  }
  else if (timer.GetTotalSeconds() > 27 && timer.GetTotalSeconds() <= 34)
    m_currentMode = Mode::Scatter;
  else if (timer.GetTotalSeconds() > 34 && timer.GetTotalSeconds() <= 54)
    m_currentMode = Mode::Chase;
  else if (timer.GetTotalSeconds() > 54 && timer.GetTotalSeconds() <= 59)
    m_currentMode = Mode::Scatter;
  else if (timer.GetTotalSeconds() > 59)
    m_currentMode = Mode::Chase;

  auto kb = m_keyboard->GetState();

  if (kb.Escape)
    ExitGame();

  const Character::Movement pacmanMovement = m_characters[Characters::Pacman]->GetMovement();
  const DirectX::XMFLOAT3& pacmanPosCurrent = m_characters[Characters::Pacman]->GetPosition();

  if (pacmanMovement == Character::Movement::Left)
  {
    bool teleport = (pacmanPosCurrent.x - 0.5f) < Global::pacManSpeed ? true : false;

    if (teleport)
    {
      m_characters[Characters::Pacman]->SetPosition(20.5f, pacmanPosCurrent.y, pacmanPosCurrent.z);
      return;
    }
  }
  else if (pacmanMovement == Character::Movement::Right)
  {
    bool teleport = (20.5f - pacmanPosCurrent.x) < Global::pacManSpeed ? true : false;

    if (teleport)
    {
      m_characters[Characters::Pacman]->SetPosition(0.5f, pacmanPosCurrent.y, pacmanPosCurrent.z);
      return;
    }
  }

  bool isHorizontallyAligned = (fmod(pacmanPosCurrent.x - 0.5f, 1.0f) < Global::pacManSpeed);
  bool isVerticallyAligned = (fmod(pacmanPosCurrent.z - 0.5f, 1.0f) < Global::pacManSpeed);

  if (isVerticallyAligned)
  {
    if (kb.Right)
      m_pacmanMovementRequest = Character::Movement::Right;
    else if (kb.Left)
      m_pacmanMovementRequest = Character::Movement::Left;
  }

  if (isHorizontallyAligned)
  {
    if (kb.Up)
      m_pacmanMovementRequest = Character::Movement::Up;
    else if (kb.Down)
      m_pacmanMovementRequest = Character::Movement::Down;
  }

  switch (m_pacmanMovementRequest)
  {
  case Character::Movement::Right:
    if (m_world.IsPassable(static_cast<uint8_t>(pacmanPosCurrent.x + Global::pacManHalfSize), static_cast<uint8_t>(pacmanPosCurrent.z)))
    {
      if (m_pacmanMovementRequest != m_characters[Characters::Pacman]->GetMovement())
        m_characters[Characters::Pacman]->AlignToMap();

      m_characters[Characters::Pacman]->SetMovement(m_pacmanMovementRequest);
    }
    else
    {
      if (m_pacmanMovementRequest == m_characters[Characters::Pacman]->GetMovement())
      {
        m_characters[Characters::Pacman]->AlignToMap();
        m_characters[Characters::Pacman]->SetMovement(Character::Movement::Stop);
      }
      else
        m_pacmanMovementRequest = m_characters[Characters::Pacman]->GetMovement();
    }
    break;
  case Character::Movement::Left:
    if (m_world.IsPassable(static_cast<uint8_t>(pacmanPosCurrent.x - (Global::pacManHalfSize + Global::pacManSpeed)), static_cast<uint8_t>(pacmanPosCurrent.z)))
    {
      if (m_pacmanMovementRequest != m_characters[Characters::Pacman]->GetMovement())
        m_characters[Characters::Pacman]->AlignToMap();

      m_characters[Characters::Pacman]->SetMovement(m_pacmanMovementRequest);
    }
    else
    {
      if (m_pacmanMovementRequest == m_characters[Characters::Pacman]->GetMovement())
      {
        m_characters[Characters::Pacman]->AlignToMap();
        m_characters[Characters::Pacman]->SetMovement(Character::Movement::Stop);
      }
      else
        m_pacmanMovementRequest = m_characters[Characters::Pacman]->GetMovement();
    }
    break;
  case Character::Movement::Up:
    if (m_world.IsPassable(static_cast<uint8_t>(pacmanPosCurrent.x), static_cast<uint8_t>(pacmanPosCurrent.z + (Global::pacManHalfSize + Global::pacManSpeed))))
    {
      if (m_pacmanMovementRequest != m_characters[Characters::Pacman]->GetMovement())
        m_characters[Characters::Pacman]->AlignToMap();

      m_characters[Characters::Pacman]->SetMovement(m_pacmanMovementRequest);
    }
    else
    {
      if (m_pacmanMovementRequest == m_characters[Characters::Pacman]->GetMovement())
      {
        m_characters[Characters::Pacman]->AlignToMap();
        m_characters[Characters::Pacman]->SetMovement(Character::Movement::Stop);
      }
      else
        m_pacmanMovementRequest = m_characters[Characters::Pacman]->GetMovement();
    }
    break;
  case Character::Movement::Down:
    if (m_world.IsPassable(static_cast<uint8_t>(pacmanPosCurrent.x), static_cast<uint8_t>(pacmanPosCurrent.z - (Global::pacManHalfSize + Global::pacManSpeed))))
    {
      if (m_pacmanMovementRequest != m_characters[Characters::Pacman]->GetMovement())
        m_characters[Characters::Pacman]->AlignToMap();

      m_characters[Characters::Pacman]->SetMovement(m_pacmanMovementRequest);
    }
    else
    {
      if (m_pacmanMovementRequest == m_characters[Characters::Pacman]->GetMovement())
      {
        m_characters[Characters::Pacman]->AlignToMap();
        m_characters[Characters::Pacman]->SetMovement(Character::Movement::Stop);
      }
      else
        m_pacmanMovementRequest = m_characters[Characters::Pacman]->GetMovement();
    }
    break;
  default:
    // Nothing
    break;
  }

  m_characters[Characters::Pacman]->SetSpriteY(0);

  switch (m_characters[Characters::Pacman]->GetMovement())
  {
  case Character::Movement::Left:
    m_characters[Characters::Pacman]->SetSpriteXAddition(4);
    m_characters[Characters::Pacman]->AdjustPosition(-Global::pacManSpeed, 0, 0);
    break;
  case Character::Movement::Right:
    m_characters[Characters::Pacman]->SetSpriteXAddition(6);
    m_characters[Characters::Pacman]->AdjustPosition(Global::pacManSpeed, 0, 0);
    break;
  case Character::Movement::Up:
    m_characters[Characters::Pacman]->SetSpriteXAddition(0);
    m_characters[Characters::Pacman]->AdjustPosition(0, 0, Global::pacManSpeed);
    break;
  case Character::Movement::Down:
    m_characters[Characters::Pacman]->SetSpriteXAddition(2);
    m_characters[Characters::Pacman]->AdjustPosition(0, 0, -Global::pacManSpeed);
    break;
  default:
    // Nothing
    break;
  }

  m_dots.Update(static_cast<uint8_t>(pacmanPosCurrent.x), static_cast<uint8_t>(pacmanPosCurrent.z), m_d3dContext.Get());

  // Ghosts
  UpdatePositionOfBlinky();
  UpdatePositionOfPinky();
  UpdatePositionOfInky();
  UpdatePositionOfClyde();
}

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
  DrawDebug();

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
    OnDeviceLost();
  else
    DX::ThrowIfFailed(hr);
}

// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
}

void Game::OnResuming()
{
  m_timer.ResetElapsedTime();
}

void Game::OnWindowSizeChanged(int width, int height)
{
  m_outputWidth = std::max(width, 1);
  m_outputHeight = std::max(height, 1);

  CreateResources();
}

void Game::GetDefaultSize(int& width, int& height) const
{
  width = 800;
  height = 600;
}

void Game::DrawWorld()
{
  m_shaderManager->SetVertexShader(ShaderManager::VertexShader::Indexed);
  m_shaderManager->SetPixelShader(ShaderManager::PixelShader::Phong);

  Global::CameraPerObject cameraPerObjectConstantBuffer;
  XMStoreFloat4x4(&cameraPerObjectConstantBuffer.world, m_world.GetWorldMatrix());

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  m_world.Draw(m_d3dContext.Get());
}

void Game::DrawSprites()
{
  // Draw dots
  m_shaderManager->SetVertexShader(ShaderManager::VertexShader::Instanced);
  m_shaderManager->SetGeometryShader(ShaderManager::GeometryShader::Billboard);
  m_shaderManager->SetPixelShader(ShaderManager::PixelShader::Texture);

  Global::SpriteConstantBuffer spriteConstantBuffer = { 0, 0, 1, 1, DirectX::XMFLOAT4(0.2f, 0, 0, 0) };
  m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &spriteConstantBuffer, sizeof(spriteConstantBuffer));

  Global::CameraPerObject cameraPerObjectConstantBuffer = {};
  XMStoreFloat4x4(&cameraPerObjectConstantBuffer.world, m_dots.GetWorldMatrix());

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  m_dots.Draw(m_d3dContext.Get());

  // Draw pacman
  m_d3dContext->PSSetShaderResources(0, 1, m_pacManShaderResourceView.GetAddressOf());

  if (m_timer.GetFrameCount() % 10 == 0)
    m_characters[Characters::Pacman]->Update();

  SetSpriteConstantBufferForCharacter(spriteConstantBuffer, *m_characters[Characters::Pacman]);
  m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &spriteConstantBuffer, sizeof(spriteConstantBuffer));

  cameraPerObjectConstantBuffer.world = m_characters[Characters::Pacman]->GetWorldMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  m_characters[Characters::Pacman]->Draw(m_d3dContext.Get());

  // Draw ghosts
  m_d3dContext->PSSetShaderResources(0, 1, m_ghostsShaderResourceView.GetAddressOf());

  if (m_timer.GetFrameCount() % 10 == 0)
    m_characters[Characters::Blinky]->Update();

  SetSpriteConstantBufferForCharacter(spriteConstantBuffer, *m_characters[Characters::Blinky]);
  m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &spriteConstantBuffer, sizeof(spriteConstantBuffer));

  cameraPerObjectConstantBuffer.world = m_characters[Characters::Blinky]->GetWorldMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  m_characters[Characters::Blinky]->Draw(m_d3dContext.Get());

  // Pinky
  if (m_timer.GetFrameCount() % 10 == 0)
    m_characters[Characters::Pinky]->Update();

  SetSpriteConstantBufferForCharacter(spriteConstantBuffer, *m_characters[Characters::Pinky]);
  m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &spriteConstantBuffer, sizeof(spriteConstantBuffer));

  cameraPerObjectConstantBuffer.world = m_characters[Characters::Pinky]->GetWorldMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  m_characters[Characters::Pinky]->Draw(m_d3dContext.Get());

  // Inky
  if (m_timer.GetFrameCount() % 10 == 0)
    m_characters[Characters::Inky]->Update();

  SetSpriteConstantBufferForCharacter(spriteConstantBuffer, *m_characters[Characters::Inky]);
  m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &spriteConstantBuffer, sizeof(spriteConstantBuffer));

  cameraPerObjectConstantBuffer.world = m_characters[Characters::Inky]->GetWorldMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  m_characters[Characters::Inky]->Draw(m_d3dContext.Get());

  // Clyde
  if (m_timer.GetFrameCount() % 10 == 0)
    m_characters[Characters::Clyde]->Update();

  SetSpriteConstantBufferForCharacter(spriteConstantBuffer, *m_characters[Characters::Clyde]);
  m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &spriteConstantBuffer, sizeof(spriteConstantBuffer));

  cameraPerObjectConstantBuffer.world = m_characters[Characters::Clyde]->GetWorldMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  m_characters[Characters::Clyde]->Draw(m_d3dContext.Get());
}

void Game::DrawDebug()
{
  if (!m_debugDraw)
    return;

  for (auto& debugPoint : m_debugPoints)
  {
    // Create dummy character to represent a target place
    Character dummy;
    dummy.Init(m_d3dDevice.Get(), debugPoint.color.x, debugPoint.color.y, debugPoint.color.z);
    dummy.SetPosition(debugPoint.position.x, debugPoint.position.y, debugPoint.position.z);

    m_shaderManager->SetVertexShader(ShaderManager::VertexShader::Instanced);
    m_shaderManager->SetGeometryShader(ShaderManager::GeometryShader::Billboard);
    m_shaderManager->SetPixelShader(ShaderManager::PixelShader::Color);

    // TODO this should not be necessary but what the heck
    Global::SpriteConstantBuffer frameConstantBuffer = { 0, 0, 1, 1, DirectX::XMFLOAT4(0.25f, 0, 0, 0) };
    m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &frameConstantBuffer, sizeof(frameConstantBuffer));

    Global::CameraPerObject cameraPerObjectConstantBuffer;
    cameraPerObjectConstantBuffer.world = dummy.GetWorldMatrix();

    m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

    dummy.Draw(m_d3dContext.Get());
  }

  m_debugPoints.clear();
}

void Game::MoveCharacterTowardsPosition(float posX, float posZ, Character& character)
{
  /* TODO
  if (m_debugDraw)
  {
    if (&character == &m_blinky)
      m_debugPoints.push_back({ {posX, 0.3f, posZ}, {0.0f ,1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} });
    else if (&character == &m_pinky)
      m_debugPoints.push_back({ {posX, 0.3f, posZ}, {0.0f ,1.0f, 0.0f}, {1.0f, 0.6f, 0.8f} });
    else if (&character == &m_inky)
      m_debugPoints.push_back({ {posX, 0.3f, posZ}, {0.0f ,1.0f, 0.0f}, {0.2f, 1.0f, 1.0f} });
    else if (&character == &m_clyde)
      m_debugPoints.push_back({ {posX, 0.3f, posZ}, {0.0f ,1.0f, 0.0f}, {1.0f, 0.8f, 0.2f} });
    else
      m_debugPoints.push_back({ {posX, 0.3f, posZ}, {0.0f ,1.0f, 0.0f}, {0.0, 0.0, 0.0} });
  }
  */

  character.IncreaseFrameCounter();

  switch (character.GetMovement())
  {
  case Character::Movement::Left:
    character.AdjustPosition(-Global::ghostSpeed, 0, 0);
    break;
  case Character::Movement::Right:
    character.AdjustPosition(Global::ghostSpeed, 0, 0);
    break;
  case Character::Movement::Up:
    character.AdjustPosition(0, 0, Global::ghostSpeed);
    break;
  case Character::Movement::Down:
    character.AdjustPosition(0, 0, -Global::ghostSpeed);
    break;
  default:
    // Should not happen
    break;
  }

  const DirectX::XMFLOAT3& characterCurrentPos = character.GetPosition();

  if (character.GetMovement() == Character::Movement::Left)
  {
    bool teleport = (characterCurrentPos.x - 0.5f) < Global::ghostSpeed ? true : false;

    if (teleport)
    {
      character.SetPosition(20.5f, characterCurrentPos.y, characterCurrentPos.z);
      return; // TODO: really?
    }
  }
  else if (character.GetMovement() == Character::Movement::Right)
  {
    bool teleport = (20.5f - characterCurrentPos.x) < Global::ghostSpeed ? true : false;

    if (teleport)
    {
      character.SetPosition(0.5f, characterCurrentPos.y, characterCurrentPos.z);
      return; // TODO: really?
    }
  }

  bool isAligned = (fmod(characterCurrentPos.x - 0.5f, 1.0f) < Global::ghostSpeed) && (fmod(characterCurrentPos.z - 0.5f, 1.0f) < Global::ghostSpeed);

  if (isAligned)
  {
    bool moves[Character::Direction::_Count] = { false, false, false, false };

    moves[Character::Direction::Up] = m_world.IsPassable(static_cast<uint8_t>(characterCurrentPos.x), static_cast<uint8_t>(characterCurrentPos.z + 1.0f));
    moves[Character::Direction::Right] = m_world.IsPassable(static_cast<uint8_t>(characterCurrentPos.x + 1.0f), static_cast<uint8_t>(characterCurrentPos.z));
    moves[Character::Direction::Down] = m_world.IsPassable(static_cast<uint8_t>(characterCurrentPos.x), static_cast<uint8_t>(characterCurrentPos.z - 1.0f));
    moves[Character::Direction::Left] = m_world.IsPassable(static_cast<uint8_t>(characterCurrentPos.x - 1.0f), static_cast<uint8_t>(characterCurrentPos.z));

    std::array<float, Character::Direction::_Count> distances = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };

    if (moves[Character::Direction::Up])
      distances[Character::Direction::Up] = sqrt((posX - characterCurrentPos.x) * (posX - characterCurrentPos.x) + (posZ - (characterCurrentPos.z + 1.0f)) * (posZ - (characterCurrentPos.z + 1.0f)));

    if (moves[Character::Direction::Down])
      distances[Character::Direction::Down] = sqrt((posX - characterCurrentPos.x) * (posX - characterCurrentPos.x) + (posZ - (characterCurrentPos.z - 1.0f)) * (posZ - (characterCurrentPos.z - 1.0f)));

    if (moves[Character::Direction::Left])
      distances[Character::Direction::Left] = sqrt((posX - (characterCurrentPos.x - 1.0f)) * (posX - (characterCurrentPos.x - 1.0f)) + (posZ - characterCurrentPos.z) * (posZ - characterCurrentPos.z));

    if (moves[Character::Direction::Right])
      distances[Character::Direction::Right] = sqrt((posX - (characterCurrentPos.x + 1.0f)) * (posX - (characterCurrentPos.x + 1.0f)) + (posZ - characterCurrentPos.z) * (posZ - characterCurrentPos.z));

    const Character::Movement characterMovement = character.GetMovement();

    switch (characterMovement)
    {
    case Character::Movement::Up:
      distances[Character::Direction::Down] = FLT_MAX;
      break;
    case Character::Movement::Down:
      distances[Character::Direction::Up] = FLT_MAX;
      break;
    case Character::Movement::Left:
      distances[Character::Direction::Right] = FLT_MAX;
      break;
    case Character::Movement::Right:
      distances[Character::Direction::Left] = FLT_MAX;
      break;
    }

    Character::Movement newMoment = static_cast<Character::Movement>(std::min_element(distances.begin(), distances.end()) - distances.begin());

    if (newMoment != characterMovement && character.GetNumberOfFrames() >= Global::minFramesPerDirection)
    {
      character.AlignToMap();
      character.SetMovement(newMoment);

      character.ResetFrameCounter();
    }
  }
}

void Game::UpdatePositionOfBlinky()
{
  if (m_currentMode == Mode::Chase)
  {
    const DirectX::XMFLOAT3& pacmanPos = m_characters[Characters::Pacman]->GetPosition();

    m_characters[Characters::Blinky]->SetSpriteY(0);
    MoveCharacterTowardsPosition(pacmanPos.x, pacmanPos.z, *m_characters[Characters::Blinky]);
  }
  else
  {
    MoveCharacterTowardsPosition(18.5f, 21.5f, *m_characters[Characters::Blinky]);
  }
}

void Game::UpdatePositionOfPinky()
{
  m_characters[Characters::Pinky]->SetSpriteY(1);

  if (m_characters[Characters::Pinky]->GetMovement() == Character::Movement::InHouse)
    return;

  if (m_currentMode == Mode::Chase)
  {
    DirectX::XMFLOAT3 pacmanPos = m_characters[Characters::Pacman]->GetPosition();

    switch (m_characters[Characters::Pacman]->GetFacingDirection())
    {
    case Character::Direction::Left:
      pacmanPos.x -= 4;
      break;
    case Character::Direction::Right:
      pacmanPos.x += 4;
      break;
    case Character::Direction::Up:
      pacmanPos.z += 4;
      break;
    case Character::Direction::Down:
      pacmanPos.z -= 4;
      break;
    }

    MoveCharacterTowardsPosition(pacmanPos.x, pacmanPos.z, *m_characters[Characters::Pinky]);
  }
  else
  {
    MoveCharacterTowardsPosition(2.5f, 21.5f, *m_characters[Characters::Pinky]);
  }
}

void Game::UpdatePositionOfInky()
{
  m_characters[Characters::Inky]->SetSpriteY(2);

  if (m_characters[Characters::Inky]->GetMovement() == Character::Movement::InHouse)
    return;

  if (m_currentMode == Mode::Chase)
  {
    DirectX::XMFLOAT3 pacmanPos = m_characters[Characters::Pacman]->GetPosition();

    switch (m_characters[Characters::Pacman]->GetFacingDirection())
    {
    case Character::Direction::Left:
      pacmanPos.x -= 2;
      break;
    case Character::Direction::Right:
      pacmanPos.x += 2;
      break;
    case Character::Direction::Up:
      pacmanPos.z += 2;
      break;
    case Character::Direction::Down:
      pacmanPos.z -= 2;
      break;
    }

    const DirectX::XMFLOAT3 blinkyPos = m_characters[Characters::Blinky]->GetPosition();

    float finalPosX = 0;
    float finalPosZ = 0;

    finalPosX = pacmanPos.x + (pacmanPos.x - blinkyPos.x);
    finalPosZ = pacmanPos.z + (pacmanPos.z - blinkyPos.z);

    MoveCharacterTowardsPosition(finalPosX, finalPosZ, *m_characters[Characters::Inky]);
  }
  else
  {
    MoveCharacterTowardsPosition(21.5f, 0.0f, *m_characters[Characters::Inky]);
  }
}

void Game::UpdatePositionOfClyde()
{
  m_characters[Characters::Clyde]->SetSpriteY(3);

  if (m_characters[Characters::Clyde]->GetMovement() == Character::Movement::InHouse)
    return;

  if (m_currentMode == Mode::Chase)
  {
    const DirectX::XMFLOAT3& pacmanPos = m_characters[Characters::Pacman]->GetPosition();
    const DirectX::XMFLOAT3& clydePos = m_characters[Characters::Clyde]->GetPosition();

    float distance = sqrt((clydePos.x - pacmanPos.x) * (clydePos.x - pacmanPos.x) + (clydePos.z - pacmanPos.z) * (clydePos.z - pacmanPos.z));

    if (distance > 8)
    {
      // Behave as blinky
      MoveCharacterTowardsPosition(pacmanPos.x, pacmanPos.z, *m_characters[Characters::Clyde]);
    }
    else
    {
      // Scatter
      MoveCharacterTowardsPosition(0.0f, 0.0f, *m_characters[Characters::Clyde]);
    }
  }
  else
  {
    MoveCharacterTowardsPosition(0.0f, 0.0f, *m_characters[Characters::Clyde]);
  }
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
  UINT creationFlags = 0;

#ifdef _DEBUG
  creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  static const D3D_FEATURE_LEVEL featureLevels[] =
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
      D3D11_MESSAGE_ID hide[] =
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
  ID3D11RenderTargetView* nullViews[] = { nullptr };
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
  m_depthStencilView.Reset();
  m_renderTargetView.Reset();
  m_swapChain.Reset();
  m_d3dContext.Reset();
  m_d3dDevice.Reset();

  CreateDevice();

  CreateResources();
}

void Game::SetSpriteConstantBufferForCharacter(Global::SpriteConstantBuffer& spriteConstantBuffer, const Character& character)
{
  spriteConstantBuffer.spriteX = character.GetSpriteX();
  spriteConstantBuffer.spriteY = character.GetSpriteY();
  spriteConstantBuffer.spriteSheetColumns = character.GetSpriteSheetColumns();
  spriteConstantBuffer.spriteSheetRows = character.GetSpriteSheetRows();
  spriteConstantBuffer.billboardSize_0_0_0 = DirectX::XMFLOAT4(character.GetSpriteScaleFactor(), 0, 0, 0);
}
