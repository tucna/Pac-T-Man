#include "pch.h"

#include <cmath>

#include "Game.h"
#include "WICTextureLoader.h"

extern void ExitGame();

using namespace DirectX;

using Microsoft::WRL::ComPtr;

#define CURRENT_PHASE m_phasesLevel1[m_currentPhaseIndex]

#define BLINKY m_ghosts[static_cast<uint8_t>(Game::Ghosts::Blinky)]
#define PINKY m_ghosts[static_cast<uint8_t>(Game::Ghosts::Pinky)]
#define INKY m_ghosts[static_cast<uint8_t>(Game::Ghosts::Inky)]
#define CLYDE m_ghosts[static_cast<uint8_t>(Game::Ghosts::Clyde)]
#define PACMAN m_pacman

Game::Game() noexcept :
  m_window(nullptr),
  m_outputWidth(800),
  m_outputHeight(600),
  m_featureLevel(D3D_FEATURE_LEVEL_9_1),
  m_pacmanMovementRequest(Character::Movement::Stop),
  m_debugDraw(false),
  m_currentPhaseIndex(1),
  m_previousPhaseIndex(1),
  m_frightenedTransition(false),
  m_currentGhostCounter(Ghosts::Pinky),
  m_gameState(State::Intro),
  m_gamePaused(true)
{
  CreatePhases();
}

void Game::Initialize(HWND window, uint16_t width, uint16_t height)
{
  assert(width > 1 && height > 1);

  m_window = window;
  m_outputWidth = width;
  m_outputHeight = height;

  CreateDevice();

  m_caption = std::make_unique<Caption>();
  m_caption->Init(m_d3dDevice.Get(), 400, 100);

  CreateResources();

  m_world.Init(m_d3dDevice.Get());

  for (auto& ghost : m_ghosts)
  {
    ghost = std::make_unique<Ghost>();
    ghost->Init(m_d3dDevice.Get());
  }

  PACMAN = std::make_unique<Pacman>();
  PACMAN->Init(m_d3dDevice.Get());

  NewGameInitialization();

  DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), nullptr, L"Resources/pacman.png", m_pacManResource.GetAddressOf(), m_pacManShaderResourceView.GetAddressOf()));
  DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), nullptr, L"Resources/ghosts.png", m_ghostsResource.GetAddressOf(), m_ghostsShaderResourceView.GetAddressOf()));

  m_keyboard = std::make_unique<Keyboard>();

  Global::LightConstantBuffer lightCB;
  lightCB.values = XMFLOAT4(10.5f, 2.5f, 11.5f, 1.0f);

  D3D11_BUFFER_DESC cbd = {};
  cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbd.Usage = D3D11_USAGE_DYNAMIC;
  cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  cbd.MiscFlags = 0;
  cbd.ByteWidth = sizeof(Global::LightConstantBuffer);
  cbd.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA csd = {};
  csd.pSysMem = &lightCB;

  DX::ThrowIfFailed(m_d3dDevice->CreateBuffer(&cbd, &csd, &m_light));

  m_shaderManager->BindConstantBuffersToPixelShader(ShaderManager::PixelShader::Phong, m_light.GetAddressOf(), 1);

  m_timer.SetFixedTimeStep(true);
  m_timer.SetTargetElapsedSeconds(1.f / 60.f);
}

void Game::Tick()
{
  m_timer.Tick([&]()
  {
    Update(m_timer);
  });

  Render();
}

void Game::Update(const DX::StepTimer& timer)
{
  const auto& kb = m_keyboard->GetState();

  if (kb.Escape)
    ExitGame();

  if (kb.Space)
  {
    if (m_gameState == Game::State::Intro)
      m_gameState = Game::State::Start;

    if (m_gameState == Game::State::Level)
    {
      m_gamePaused = false;
      m_pacmanMovementRequest = Character::Movement::Left;
    }
  }

  if (PACMAN->IsDead() && PACMAN->IsAnimationDone() && m_gameState == Game::State::Level)
    m_gameState = Game::State::Dead;

  switch (m_gameState)
  {
  case Game::State::Intro:
    m_caption->AdjustY(5, 80);
    m_camera.ResetLerp();
    break;
  case Game::State::Start:
    m_camera.LerpBetweenCameraPositions(0.04f);

    if (m_camera.IsCameraLerpDone())
    {
      NewGameInitialization();
      m_gameState = Game::State::Level;
      CURRENT_PHASE.startingTime = timer.GetTotalSeconds();
    }
    break;
  case Game::State::Level:
    m_camera.ResetLerp();
    break;
  case Game::State::Dead:
    m_camera.InverseLerpBetweenCameraPositions(0.04f);

    if (m_camera.IsCameraLerpDone())
      m_gameState = Game::State::Intro;
    break;
  }

  // Skip all simulation in a case that game did not start and current phase timer should not move in case of a paused game
  if (m_gameState != Game::State::Level || m_gamePaused)
  {
    CURRENT_PHASE.startingTime = timer.GetTotalSeconds();
    return;
  }

  // Update frames
  PACMAN->UpdateFrame(m_timer.GetElapsedSeconds());
  std::for_each(m_ghosts.begin(), m_ghosts.end(), [&](auto& ghost) { ghost->UpdateFrame(m_timer.GetElapsedSeconds()); });

  if (timer.GetTotalSeconds() >= (CURRENT_PHASE.startingTime + CURRENT_PHASE.duration) && m_currentPhaseIndex < Global::phasesNum - 1)
  {
    if (m_currentPhaseIndex == 0)
    {
      m_currentPhaseIndex = m_previousPhaseIndex;

      SetGhostsDefaultSprites();
    }
    else
    {
      m_currentPhaseIndex++;
      std::for_each(m_ghosts.begin(), m_ghosts.end(), [](auto& ghost) { ghost->ReverseMovementDirection(); });
    }

    CURRENT_PHASE.startingTime = timer.GetTotalSeconds();

    // Force current mode to the ghost
    std::for_each(m_ghosts.begin(), m_ghosts.end(), [&](auto& ghost)
    {
      if (ghost->IsAlive())
        ghost->SetMode(CURRENT_PHASE.mode);
    });

    m_frightenedTransition = false;
  }
  else if (CURRENT_PHASE.mode == Global::Mode::Frightened && (timer.GetTotalSeconds() >= (CURRENT_PHASE.startingTime + CURRENT_PHASE.duration / 2.0)) && !m_frightenedTransition)
  {
    std::for_each(m_ghosts.begin(), m_ghosts.end(), [](auto& ghost)
    {
      if (ghost->GetMode() == Global::Mode::Frightened && ghost->IsAlive())
        ghost->SetSpriteY(Global::rowTransition);
    });

    m_frightenedTransition = true;
  }

  switch (m_currentGhostCounter)
  {
  case Ghosts::Pinky:
    m_currentGhostCounter = Ghosts::Inky;
    PINKY->SetMovement(Character::Movement::InHouse);
    break;
  case Ghosts::Inky:
    if (INKY->ReadyToLeaveHouse())
    {
      m_currentGhostCounter = Ghosts::Clyde;
      INKY->SetMovement(Character::Movement::InHouse);
    }
    break;
  case Ghosts::Clyde:
    if (CLYDE->ReadyToLeaveHouse())
    {
      m_currentGhostCounter = Ghosts::None;
      CLYDE->SetMovement(Character::Movement::InHouse);
    }
    break;
  }

  const Character::Movement pacmanMovement = PACMAN->GetMovement();
  const DirectX::XMFLOAT3& pacmanPosCurrent = PACMAN->GetPosition();

  // Teleport handling
  if (pacmanMovement == Character::Movement::Left && (pacmanPosCurrent.x - 0.5f) < Global::pacManSpeed)
  {
    PACMAN->SetPosition(20.5f, pacmanPosCurrent.y, pacmanPosCurrent.z);
    return;
  }
  else if (pacmanMovement == Character::Movement::Right && (20.5f - pacmanPosCurrent.x) < Global::pacManSpeed)
  {
    PACMAN->SetPosition(0.5f, pacmanPosCurrent.y, pacmanPosCurrent.z);
    return;
  }

  // Dead handling
  if (PACMAN->IsDead())
    return;

  bool isHorizontallyAligned = (fmod(pacmanPosCurrent.x - 0.5f, 1.0f) < Global::pacManSpeed);
  bool isVerticallyAligned = (fmod(pacmanPosCurrent.z - 0.5f, 1.0f) < Global::pacManSpeed);

  bool moves[Character::Direction::_Count] = { false, false, false, false };

  moves[Character::Direction::Up] = m_world.IsPassable(static_cast<uint8_t>(pacmanPosCurrent.x), static_cast<uint8_t>(pacmanPosCurrent.z + 1.0f), false);
  moves[Character::Direction::Right] = m_world.IsPassable(static_cast<uint8_t>(pacmanPosCurrent.x + 1.0f), static_cast<uint8_t>(pacmanPosCurrent.z), false);
  moves[Character::Direction::Down] = m_world.IsPassable(static_cast<uint8_t>(pacmanPosCurrent.x), static_cast<uint8_t>(pacmanPosCurrent.z - 1.0f), false);
  moves[Character::Direction::Left] = m_world.IsPassable(static_cast<uint8_t>(pacmanPosCurrent.x - 1.0f), static_cast<uint8_t>(pacmanPosCurrent.z), false);

  if (isVerticallyAligned)
  {
    if (kb.Right && moves[Character::Direction::Right])
      m_pacmanMovementRequest = Character::Movement::Right;
    else if (kb.Left && moves[Character::Direction::Left])
      m_pacmanMovementRequest = Character::Movement::Left;
  }

  if (isHorizontallyAligned)
  {
    if (kb.Up && moves[Character::Direction::Up])
      m_pacmanMovementRequest = Character::Movement::Up;
    else if (kb.Down && moves[Character::Direction::Down])
      m_pacmanMovementRequest = Character::Movement::Down;
  }

  auto pacmanHandleMovement = [&](bool isPassable, bool alignment)
  {
    if (isPassable)
    {
      if (!AreMovementsOppositeOrSame(m_pacmanMovementRequest, pacmanMovement))
        PACMAN->AlignToMap();

      if (m_pacmanMovementRequest != pacmanMovement)
        PACMAN->SetMovement(m_pacmanMovementRequest);
    }
    else if (alignment)
    {
      if (m_pacmanMovementRequest == pacmanMovement)
      {
        PACMAN->AlignToMap();
        PACMAN->SetMovement(Character::Movement::Stop);
      }
      else
        m_pacmanMovementRequest = pacmanMovement;
    }
  };

  switch (m_pacmanMovementRequest)
  {
  case Character::Movement::Right:
    pacmanHandleMovement(moves[Character::Direction::Right], isHorizontallyAligned);
    break;
  case Character::Movement::Left:
    pacmanHandleMovement(moves[Character::Direction::Left], isHorizontallyAligned);
    break;
  case Character::Movement::Up:
    pacmanHandleMovement(moves[Character::Direction::Up], isVerticallyAligned);
    break;
  case Character::Movement::Down:
    pacmanHandleMovement(moves[Character::Direction::Down], isVerticallyAligned);
    break;
  default:
    // Nothing
    break;
  }

  switch (PACMAN->GetMovement())
  {
  case Character::Movement::Left:
    PACMAN->AdjustPosition(-Global::pacManSpeed, 0, 0);
    break;
  case Character::Movement::Right:
    PACMAN->AdjustPosition(Global::pacManSpeed, 0, 0);
    break;
  case Character::Movement::Up:
    PACMAN->AdjustPosition(0, 0, Global::pacManSpeed);
    break;
  case Character::Movement::Down:
    PACMAN->AdjustPosition(0, 0, -Global::pacManSpeed);
    break;
  default:
    // Nothing
    break;
  }

  Dots::Type dotEaten;
  m_dots->Update(static_cast<uint8_t>(pacmanPosCurrent.x), static_cast<uint8_t>(pacmanPosCurrent.z), m_d3dContext.Get(), dotEaten);

  if (dotEaten == Dots::Type::LastOne)
  {
    NewGameInitialization();
    m_gameState = Game::State::Level;
  }

  if (dotEaten != Dots::Type::Nothing && m_currentGhostCounter != Ghosts::None)
    m_ghosts[static_cast<uint8_t>(m_currentGhostCounter)]->IncrementEatenDots();

  if (dotEaten == Dots::Type::Extra)
  {
    m_previousPhaseIndex = m_currentPhaseIndex == 0 ? m_previousPhaseIndex : m_currentPhaseIndex;
    m_frightenedTransition = false;
    m_currentPhaseIndex = 0; // Force frightened mode

    CURRENT_PHASE.startingTime = timer.GetTotalSeconds();

    std::for_each(m_ghosts.begin(), m_ghosts.end(), [](auto& ghost)
    {
      if (ghost->IsAlive())
      {
        ghost->SetMode(Global::Mode::Frightened);
        ghost->SetSpriteY(Global::rowFrightened);
        ghost->ReverseMovementDirection();
      }
    });
  }

  // Ghosts
  UpdatePositionOfBlinky();
  UpdatePositionOfPinky();
  UpdatePositionOfInky();
  UpdatePositionOfClyde();

  // Collision handling
  HandleCollisions();
}

void Game::Render()
{
  // Don't try to render anything before the first Update.
  if (m_timer.GetFrameCount() == 0)
    return;

  Clear();

  switch (m_gameState)
  {
  case Game::State::Intro:
    DrawWorld();
    DrawIntro();
    break;
  case Game::State::Start:
    DrawWorld();
    break;
  case Game::State::Level:
    DrawWorld();
    DrawSprites();
    DrawDebug();
    break;
  case Game::State::Dead:
    DrawWorld();
    break;
  }

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

void Game::OnWindowSizeChanged(uint16_t width, uint16_t height)
{
  assert(width > 1 && height > 1);

  m_outputWidth = width;
  m_outputHeight = height;

  CreateResources();
}

void Game::GetDefaultSize(uint16_t& width, uint16_t& height) const
{
  width = 800;
  height = 600;
}

void Game::DrawWorld()
{
  m_shaderManager->SetVertexShader(ShaderManager::VertexShader::Indexed);
  m_shaderManager->SetPixelShader(ShaderManager::PixelShader::Phong);

  Global::CameraPerObject cameraPerObjectConstantBuffer;
  cameraPerObjectConstantBuffer.world = m_world.GetWorldMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  // Camera
  Global::CameraPerFrame cameraConstantBufferPerFrame = {};
  cameraConstantBufferPerFrame.view = m_camera.GetViewMatrix();
  cameraConstantBufferPerFrame.projection = m_camera.GetProjectionMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerFrame.Get(), &cameraConstantBufferPerFrame, sizeof(cameraConstantBufferPerFrame));

  m_world.Draw(m_d3dContext.Get());
}

void Game::DrawIntro()
{
  m_shaderManager->SetVertexShader(ShaderManager::VertexShader::UI);
  m_shaderManager->SetPixelShader(ShaderManager::PixelShader::UI);

  Global::CameraPerFrame cameraConstantBufferPerFrame = {};
  XMStoreFloat4x4(&cameraConstantBufferPerFrame.view, DirectX::XMMatrixIdentity());
  cameraConstantBufferPerFrame.projection = m_camera.GetOrthographicMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerFrame.Get(), &cameraConstantBufferPerFrame, sizeof(cameraConstantBufferPerFrame));

  Global::CameraPerObject cameraPerObjectConstantBuffer = {};
  cameraPerObjectConstantBuffer.world = m_caption->GetWorldMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  m_caption->Draw(m_d3dContext.Get());
}

void Game::DrawSprites()
{
  // Draw dots
  m_shaderManager->SetVertexShader(ShaderManager::VertexShader::Instanced);
  m_shaderManager->SetGeometryShader(ShaderManager::GeometryShader::Billboard);
  m_shaderManager->SetPixelShader(ShaderManager::PixelShader::Texture);

  Global::SpriteConstantBuffer spriteConstantBuffer = { 0, 0, 1, 1, DirectX::XMFLOAT4(0.2f, 0.4f, 0, 0) };
  m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &spriteConstantBuffer, sizeof(spriteConstantBuffer));

  Global::CameraPerObject cameraPerObjectConstantBuffer = {};
  cameraPerObjectConstantBuffer.world = m_dots->GetWorldMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  m_dots->Draw(m_d3dContext.Get());

  // Draw pacman
  m_d3dContext->PSSetShaderResources(0, 1, m_pacManShaderResourceView.GetAddressOf());

  SetSpriteConstantBufferForCharacter(spriteConstantBuffer, *PACMAN);
  m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &spriteConstantBuffer, sizeof(spriteConstantBuffer));

  cameraPerObjectConstantBuffer.world = PACMAN->GetWorldMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  PACMAN->Draw(m_d3dContext.Get());

  // Draw ghosts
  // This should happen only when pacman is not dead
  if (PACMAN->IsAlive())
  {
    m_d3dContext->PSSetShaderResources(0, 1, m_ghostsShaderResourceView.GetAddressOf());

    std::for_each(m_ghosts.begin(), m_ghosts.end(), [&](auto& ghost)
    {
      SetSpriteConstantBufferForCharacter(spriteConstantBuffer, *ghost);
      m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &spriteConstantBuffer, sizeof(spriteConstantBuffer));

      cameraPerObjectConstantBuffer.world = ghost->GetWorldMatrix();
      m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

      ghost->Draw(m_d3dContext.Get());
    });
  }
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

    Global::SpriteConstantBuffer frameConstantBuffer = { 0, 0, 1, 1, DirectX::XMFLOAT4(0.25f, 0, 0, 0) };
    m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &frameConstantBuffer, sizeof(frameConstantBuffer));

    Global::CameraPerObject cameraPerObjectConstantBuffer;
    cameraPerObjectConstantBuffer.world = dummy.GetWorldMatrix();

    m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

    dummy.Draw(m_d3dContext.Get());
  }

  m_debugPoints.clear();
}

void Game::MoveGhostTowardsPosition(float posX, float posZ, Game::Ghosts ghost)
{
  Ghost& character = *m_ghosts[static_cast<uint8_t>(ghost)];

  if (m_debugDraw)
    switch (ghost)
    {
      case Ghosts::Blinky:
        m_debugPoints.push_back({{posX, 0.3f, posZ}, {0.0f ,1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});
        break;
      case Ghosts::Pinky:
        m_debugPoints.push_back({{posX, 0.3f, posZ}, {0.0f ,1.0f, 0.0f}, {1.0f, 0.6f, 0.8f}});
        break;
      case Ghosts::Inky:
        m_debugPoints.push_back({{posX, 0.3f, posZ}, {0.0f ,1.0f, 0.0f}, {0.2f, 1.0f, 1.0f}});
        break;
      case Ghosts::Clyde:
        m_debugPoints.push_back({{posX, 0.3f, posZ}, {0.0f ,1.0f, 0.0f}, {1.0f, 0.8f, 0.2f}});
        break;
    }

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
  bool isCharacterDead = character.IsDead() || (character.GetMovement() == Character::Movement::InHouse);

  if (isCharacterDead)
  {
    float d = sqrt((characterCurrentPos.x - posX) * (characterCurrentPos.x - posX) + (characterCurrentPos.z - posZ) * (characterCurrentPos.z - posZ));

    if (d < 0.1f)
    {
      character.AlignToMap();
      character.SetMovement(Character::Movement::InHouse);
      character.SetDead(false);

      return;
    }
  }


  if (character.GetMovement() == Character::Movement::Left)
  {
    bool teleport = (characterCurrentPos.x - 0.5f) < Global::ghostSpeed ? true : false;

    if (teleport)
    {
      character.SetPosition(20.5f, characterCurrentPos.y, characterCurrentPos.z);
      return;
    }
  }
  else if (character.GetMovement() == Character::Movement::Right)
  {
    bool teleport = (20.5f - characterCurrentPos.x) < Global::ghostSpeed ? true : false;

    if (teleport)
    {
      character.SetPosition(0.5f, characterCurrentPos.y, characterCurrentPos.z);
      return;
    }
  }

  bool isAligned = (fmod(characterCurrentPos.x - 0.5f, 1.0f) < Global::ghostSpeed) && (fmod(characterCurrentPos.z - 0.5f, 1.0f) < Global::ghostSpeed);

  if (isAligned)
  {
    bool moves[Character::Direction::_Count] = { false, false, false, false };

    moves[Character::Direction::Up] = m_world.IsPassable(static_cast<uint8_t>(characterCurrentPos.x), static_cast<uint8_t>(characterCurrentPos.z + 1.0f), isCharacterDead);
    moves[Character::Direction::Right] = m_world.IsPassable(static_cast<uint8_t>(characterCurrentPos.x + 1.0f), static_cast<uint8_t>(characterCurrentPos.z), isCharacterDead);
    moves[Character::Direction::Down] = m_world.IsPassable(static_cast<uint8_t>(characterCurrentPos.x), static_cast<uint8_t>(characterCurrentPos.z - 1.0f), isCharacterDead);
    moves[Character::Direction::Left] = m_world.IsPassable(static_cast<uint8_t>(characterCurrentPos.x - 1.0f), static_cast<uint8_t>(characterCurrentPos.z), isCharacterDead);

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

    /*
    if ((newMoment == Character::Movement::Down && m_world.GetTile(static_cast<uint8_t>(characterCurrentPos.x), static_cast<uint8_t>(characterCurrentPos.z - 1.0f)) == 3))
      character.SetPosition(characterCurrentPos.x, 1.0, characterCurrentPos.z);
    else
      character.SetPosition(characterCurrentPos.x, 0.3, characterCurrentPos.z);
    */

    if (newMoment != characterMovement && character.GetNumberOfFrames() >= Global::minFramesPerDirection)
    {
      character.AlignToMap();
      character.SetMovement(newMoment);

      character.ResetFrameCounter();
    }
  }
}

void Game::MoveGhostTowardsRandomPosition(Game::Ghosts ghost)
{
  Ghost& character = *m_ghosts[static_cast<uint8_t>(ghost)];

  const DirectX::XMFLOAT3 charPos = character.GetPosition();
  uint8_t direction = rand() % 4;

  switch (direction)
  {
  case 0:
    MoveGhostTowardsPosition(charPos.x + 1.0f, charPos.z, ghost);
    break;
  case 1:
    MoveGhostTowardsPosition(charPos.x - 1.0f, charPos.z, ghost);
    break;
  case 2:
    MoveGhostTowardsPosition(charPos.x, charPos.z + 1.0f, ghost);
    break;
  case 3:
    MoveGhostTowardsPosition(charPos.x, charPos.z - 1.0f, ghost);
    break;
  }
}

void Game::SetGhostsDefaultSprites()
{
  if (BLINKY->IsAlive()) BLINKY->SetSpriteY(Global::rowBlinky);
  if (PINKY->IsAlive())  PINKY->SetSpriteY(Global::rowPinky);
  if (INKY->IsAlive())   INKY->SetSpriteY(Global::rowInky);
  if (CLYDE->IsAlive())  CLYDE->SetSpriteY(Global::rowClyde);
}

void Game::CreatePhases()
{
  m_phasesLevel1[0] = { Global::Mode::Frightened, 0.0, 6.0 };
  m_phasesLevel1[1] = { Global::Mode::Scatter, 0.0, 7.0 };
  m_phasesLevel1[2] = { Global::Mode::Chase, 0.0, 20.0 };
  m_phasesLevel1[3] = { Global::Mode::Scatter, 0.0, 7.0 };
  m_phasesLevel1[4] = { Global::Mode::Chase, 0.0, 20.0 };
  m_phasesLevel1[5] = { Global::Mode::Scatter, 0.0, 5.0 };
  m_phasesLevel1[6] = { Global::Mode::Chase, 0.0, 20.0 };
  m_phasesLevel1[7] = { Global::Mode::Scatter, 0.0, 5.0 };
  m_phasesLevel1[8] = { Global::Mode::Chase, 0.0, 0.0 };
}

void Game::HandleCollisions()
{
  // Distance
  std::for_each(m_ghosts.begin(), m_ghosts.end(), [&](auto& ghost)
  {
    float distance = DistanceBetweenCharacters(*PACMAN, *ghost);

    if (distance < 0.1f)
    {
      if (ghost->GetMode() == Global::Mode::Frightened)
      {
        ghost->SetSpriteY(Global::rowDead);
        ghost->SetDead(true);
      }
      else
      {
        PACMAN->SetDead(true);
        PACMAN->SetSpriteX(0);
        PACMAN->SetSpriteY(1);
        PACMAN->SetFramesPerState(12);
        PACMAN->SetOneCycle(true);
        PACMAN->Restart();
      }
    }
  });
}

void Game::UpdateCameraForStartAnimation()
{
  Global::CameraPerFrame cameraConstantBufferPerFrame = {};
  cameraConstantBufferPerFrame.view = m_camera.GetViewMatrix();
  cameraConstantBufferPerFrame.projection = m_camera.GetProjectionMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerFrame.Get(), &cameraConstantBufferPerFrame, sizeof(cameraConstantBufferPerFrame));
}

void Game::NewGameInitialization()
{
  // Level
  if (m_dots)
    m_dots.reset();

  m_dots = std::make_unique<Dots>();
  m_dots->Init(m_d3dDevice.Get());

  // Ghosts
  BLINKY->SetPosition(10.5f, 0.30f, 13.5f);
  BLINKY->SetMovement(Character::Movement::Left);
  BLINKY->SetMode(Global::Mode::Scatter);
  BLINKY->SetSpriteY(Global::rowBlinky);

  PINKY->SetPosition(10.5f, 0.31f, 11.5f);
  PINKY->SetMovement(Character::Movement::Stop);
  PINKY->SetMode(Global::Mode::Scatter);
  PINKY->SetSpriteX(0);
  PINKY->SetDotLimit(0);
  PINKY->ResetEatenDots();
  PINKY->SetSpriteY(Global::rowPinky);

  INKY->SetPosition(9.5f, 0.32f, 11.5f);
  INKY->SetMovement(Character::Movement::Stop);
  INKY->SetMode(Global::Mode::Scatter);
  INKY->SetSpriteX(0);
  INKY->SetDotLimit(30);
  INKY->ResetEatenDots();
  INKY->SetSpriteY(Global::rowInky);

  CLYDE->SetPosition(11.5f, 0.33f, 11.5f);
  CLYDE->SetMovement(Character::Movement::Stop);
  CLYDE->SetMode(Global::Mode::Scatter);
  CLYDE->SetSpriteX(0);
  CLYDE->SetDotLimit(60);
  CLYDE->ResetEatenDots();
  CLYDE->SetSpriteY(Global::rowClyde);

  // Pacman
  PACMAN->SetDead(false);
  PACMAN->SetPosition(10.5f, 0.25f, 5.5f);
  PACMAN->SetMovement(Character::Movement::Left);
  PACMAN->SetSpriteY(0);
  PACMAN->SetFramesPerState(2);
  PACMAN->SetOneCycle(false);
  PACMAN->Restart();

  // Game
  m_gamePaused = true;
  m_pacmanMovementRequest = Character::Movement::Stop;
  m_currentPhaseIndex = 1;
  m_previousPhaseIndex = 1;
  m_frightenedTransition = false;
  m_currentGhostCounter = Ghosts::Pinky;
  m_timer.ResetElapsedTime();
}

bool Game::AreMovementsOppositeOrSame(Character::Movement m1, Character::Movement m2)
{
  if (m1 == m2) return true;

  if (m1 == Character::Movement::Up && m2 == Character::Movement::Down) return true;
  if (m1 == Character::Movement::Right && m2 == Character::Movement::Left) return true;
  if (m1 == Character::Movement::Down && m2 == Character::Movement::Up) return true;
  if (m1 == Character::Movement::Left && m2 == Character::Movement::Right) return true;

  return false;
}

float Game::DistanceBetweenCharacters(const Character& ch1, const Character& ch2)
{
  const XMFLOAT3& pos1 = ch1.GetPosition();
  const XMFLOAT3& pos2 = ch2.GetPosition();

  return sqrt((pos1.x - pos2.x) * (pos1.x - pos2.x) + (pos1.z - pos2.z) * (pos1.z - pos2.z));
}

void Game::UpdatePositionOfBlinky()
{
  if (BLINKY->IsDead())
  {
    MoveGhostTowardsPosition(10.5f, 11.5f, Ghosts::Blinky);
    return;
  }

  if (BLINKY->GetMovement() == Character::Movement::InHouse)
  {
    BLINKY->SetMode(CURRENT_PHASE.mode);
    BLINKY->SetSpriteY(Global::rowBlinky);
  }

  switch (BLINKY->GetMode())
  {
  case Global::Mode::Scatter:
    MoveGhostTowardsPosition(18.5f, 21.5f, Ghosts::Blinky);
    break;
  case Global::Mode::Chase:
  {
    const DirectX::XMFLOAT3& pacmanPos = PACMAN->GetPosition();
    MoveGhostTowardsPosition(pacmanPos.x, pacmanPos.z, Ghosts::Blinky);
  }
  break;
  case Global::Mode::Frightened:
    MoveGhostTowardsRandomPosition(Ghosts::Blinky);
    break;
  }
}

void Game::UpdatePositionOfPinky()
{
  if (PINKY->GetMovement() == Character::Movement::Stop)
    return;

  if (PINKY->IsDead())
  {
    MoveGhostTowardsPosition(10.5f, 11.5f, Ghosts::Pinky);
    return;
  }

  if (PINKY->GetMovement() == Character::Movement::InHouse)
  {
    PINKY->SetMode(CURRENT_PHASE.mode);
    PINKY->SetSpriteY(Global::rowPinky);
  }

  switch (PINKY->GetMode())
  {
  case Global::Mode::Scatter:
    MoveGhostTowardsPosition(2.5f, 21.5f, Ghosts::Pinky);
    break;
  case Global::Mode::Chase:
  {
    DirectX::XMFLOAT3 pacmanPos = PACMAN->GetPosition();

    switch (PACMAN->GetFacingDirection())
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

    MoveGhostTowardsPosition(pacmanPos.x, pacmanPos.z, Ghosts::Pinky);
  }
  break;
  case Global::Mode::Frightened:
    MoveGhostTowardsRandomPosition(Ghosts::Pinky);
    break;
  }
}

void Game::UpdatePositionOfInky()
{
  if (INKY->GetMovement() == Character::Movement::Stop)
    return;

  if (INKY->IsDead())
  {
    MoveGhostTowardsPosition(10.5f, 11.5f, Ghosts::Inky);
    return;
  }

  if (INKY->GetMovement() == Character::Movement::InHouse)
  {
    INKY->SetMode(CURRENT_PHASE.mode);
    INKY->SetSpriteY(Global::rowInky);
  }

  switch (INKY->GetMode())
  {
  case Global::Mode::Scatter:
    MoveGhostTowardsPosition(21.5f, 0.0f, Ghosts::Inky);
    break;
  case Global::Mode::Chase:
  {
    DirectX::XMFLOAT3 pacmanPos = PACMAN->GetPosition();

    switch (PACMAN->GetFacingDirection())
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

    const DirectX::XMFLOAT3 blinkyPos = BLINKY->GetPosition();

    float finalPosX = 0;
    float finalPosZ = 0;

    finalPosX = pacmanPos.x + (pacmanPos.x - blinkyPos.x);
    finalPosZ = pacmanPos.z + (pacmanPos.z - blinkyPos.z);

    MoveGhostTowardsPosition(finalPosX, finalPosZ, Ghosts::Inky);
  }
  break;
  case Global::Mode::Frightened:
    MoveGhostTowardsRandomPosition(Ghosts::Inky);
    break;
  }
}

void Game::UpdatePositionOfClyde()
{
  if (CLYDE->GetMovement() == Character::Movement::Stop)
    return;

  if (CLYDE->IsDead())
  {
    MoveGhostTowardsPosition(10.5f, 11.5f, Ghosts::Clyde);
    return;
  }

  if (CLYDE->GetMovement() == Character::Movement::InHouse)
  {
    CLYDE->SetMode(CURRENT_PHASE.mode);
    CLYDE->SetSpriteY(Global::rowClyde);
  }

  switch (CLYDE->GetMode())
  {
  case Global::Mode::Scatter:
    MoveGhostTowardsPosition(0.0f, 0.0f, Ghosts::Clyde);
    break;
  case Global::Mode::Chase:
  {
    const DirectX::XMFLOAT3& pacmanPos = PACMAN->GetPosition();
    const DirectX::XMFLOAT3& clydePos = CLYDE->GetPosition();

    float distance = sqrt((clydePos.x - pacmanPos.x) * (clydePos.x - pacmanPos.x) + (clydePos.z - pacmanPos.z) * (clydePos.z - pacmanPos.z));

    if (distance > 8)
    {
      // Behave as blinky
      MoveGhostTowardsPosition(pacmanPos.x, pacmanPos.z, Ghosts::Clyde);
    }
    else
    {
      // Scatter
      MoveGhostTowardsPosition(0.0f, 0.0f, Ghosts::Clyde);
    }
  }
  break;
  case Global::Mode::Frightened:
    MoveGhostTowardsRandomPosition(Ghosts::Clyde);
    break;
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
          // Add more message IDs here as needed.
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

  // Initialize windows-size dependent objects here.
  m_camera.SetProjectionValues(75.0f, static_cast<float>(m_outputWidth) / static_cast<float>(m_outputHeight), 0.1f, 1000.0f);
  m_camera.SetOrthographicValues(static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));

  XMFLOAT4X4 projection = m_camera.GetProjectionMatrix();

  D3D11_BUFFER_DESC cbd = {};
  cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbd.Usage = D3D11_USAGE_DYNAMIC;
  cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  cbd.MiscFlags = 0;
  cbd.ByteWidth = sizeof(XMFLOAT4X4);
  cbd.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA csd = {};
  csd.pSysMem = &projection;

  DX::ThrowIfFailed(m_d3dDevice->CreateBuffer(&cbd, &csd, &m_projectionMatrixConstantBuffer));

  Global::CameraPerFrame cameraConstantBufferPerFrame = {};
  cameraConstantBufferPerFrame.view = m_camera.GetViewMatrix();
  cameraConstantBufferPerFrame.projection = m_camera.GetProjectionMatrix();

  cbd = {};
  cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbd.Usage = D3D11_USAGE_DYNAMIC;
  cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  cbd.MiscFlags = 0;
  cbd.ByteWidth = sizeof(Global::CameraPerFrame);
  cbd.StructureByteStride = 0;

  csd = {};
  csd.pSysMem = &cameraConstantBufferPerFrame;

  DX::ThrowIfFailed(m_d3dDevice->CreateBuffer(&cbd, &csd, &m_cameraPerFrame));

  Global::CameraPerObject cameraConstantBufferPerObject;
  XMStoreFloat4x4(&cameraConstantBufferPerObject.world, DirectX::XMMatrixIdentity());

  cbd.ByteWidth = sizeof(Global::CameraPerObject);

  csd.pSysMem = &cameraConstantBufferPerObject;

  DX::ThrowIfFailed(m_d3dDevice->CreateBuffer(&cbd, &csd, &m_cameraPerObject));

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

  DX::ThrowIfFailed(m_d3dDevice->CreateBuffer(&cbd, &csd, &m_frameBuffer));

  ID3D11Buffer* geometryShaderBuffers[2] = { m_projectionMatrixConstantBuffer.Get(), m_frameBuffer.Get() };
  m_shaderManager->BindConstantBuffersToGeometryShader(ShaderManager::GeometryShader::Billboard, geometryShaderBuffers, 2);

  ID3D11Buffer* vertexShaderBuffers[2] = { m_cameraPerFrame.Get(), m_cameraPerObject.Get() };
  m_shaderManager->BindConstantBuffersToVertexShader(ShaderManager::VertexShader::Instanced, vertexShaderBuffers, 2);
  m_shaderManager->BindConstantBuffersToVertexShader(ShaderManager::VertexShader::Indexed, vertexShaderBuffers, 2);

  m_caption->SetPosition(static_cast<float>((m_outputWidth / 2) - (m_caption->GetWidth() / 2)), static_cast<float>(-m_caption->GetHeight() * 2), 0);
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
