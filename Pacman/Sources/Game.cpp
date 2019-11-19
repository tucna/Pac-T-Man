#include "pch.h"

#include "Game.h"
#include "WICTextureLoader.h"

extern void ExitGame();

using namespace DirectX;

using Microsoft::WRL::ComPtr;

#define CURRENT_PHASE m_phasesLevel1[m_currentPhaseIndex]

Game::Game() noexcept :
  m_window(nullptr),
  m_outputWidth(800),
  m_outputHeight(600),
  m_featureLevel(D3D_FEATURE_LEVEL_9_1),
  m_pacmanMovementRequest(Character::Movement::Stop),
  m_debugDraw(false),
  m_currentPhaseIndex(1),
  m_previousPhaseIndex(1),
  m_frightenedTransition(false)
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
  CreateResources();

  m_world.Init(m_d3dDevice.Get());
  m_dots.Init(m_d3dDevice.Get());

  for (auto& character : m_characters)
  {
    character = std::make_unique<Character>();
    character->Init(m_d3dDevice.Get());
    character->SetMovement(Character::Movement::Left);
    character->SetColumnsAndRowsOfAssociatedSpriteSheet(8, 6);
    character->SetSpriteScaleFactor(Global::ghostSize);
    character->SetFramesPerState(2);
  }

  m_characters[Characters::Blinky]->SetPosition(10.5f, 0.3f, 13.5f);
  m_characters[Characters::Pinky]->SetPosition(11.5f, 0.3f, 13.5f);
  m_characters[Characters::Inky]->SetPosition(12.5f, 0.3f, 13.5f);
  m_characters[Characters::Clyde]->SetPosition(13.5f, 0.3f, 13.5f);

  SetGhostsDefaultSprites();

  m_characters[Characters::Pacman]->SetPosition(10.5f, 0.25f, 9.5f);
  m_characters[Characters::Pacman]->SetColumnsAndRowsOfAssociatedSpriteSheet(8, 1);
  m_characters[Characters::Pacman]->SetSpriteScaleFactor(Global::pacManSize);
  m_characters[Characters::Pacman]->SetMovement(Character::Movement::Stop);
  m_characters[Characters::Pacman]->SetSpriteY(0);

  DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), nullptr, L"Resources/pacman.png", m_pacManResource.GetAddressOf(), m_pacManShaderResourceView.GetAddressOf()));
  DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), nullptr, L"Resources/ghosts.png", m_ghostsResource.GetAddressOf(), m_ghostsShaderResourceView.GetAddressOf()));

  m_keyboard = std::make_unique<Keyboard>();
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
      std::for_each(m_characters.begin() + 1, m_characters.end(), [](auto& character) { character->ReverseMovementDirection(); });
    }

    CURRENT_PHASE.startingTime = timer.GetTotalSeconds();

    m_frightenedTransition = false;
  }
  else if (CURRENT_PHASE.mode == Mode::Frightened && (timer.GetTotalSeconds() >= (CURRENT_PHASE.startingTime + CURRENT_PHASE.duration / 2.0)) && !m_frightenedTransition)
  {
    std::for_each(m_characters.begin() + 1, m_characters.end(), [](auto& character) { character->SetSpriteY(Global::ghostFrightenedTransitionSpriteRow); });
    m_frightenedTransition = true;
  }

  const auto& kb = m_keyboard->GetState();

  if (kb.Escape)
    ExitGame();

  const Character::Movement pacmanMovement = m_characters[Characters::Pacman]->GetMovement();
  const DirectX::XMFLOAT3& pacmanPosCurrent = m_characters[Characters::Pacman]->GetPosition();

  // Teleport handling
  if (pacmanMovement == Character::Movement::Left && (pacmanPosCurrent.x - 0.5f) < Global::pacManSpeed)
  {
    m_characters[Characters::Pacman]->SetPosition(20.5f, pacmanPosCurrent.y, pacmanPosCurrent.z);
    return;
  }
  else if (pacmanMovement == Character::Movement::Right && (20.5f - pacmanPosCurrent.x) < Global::pacManSpeed)
  {
    m_characters[Characters::Pacman]->SetPosition(0.5f, pacmanPosCurrent.y, pacmanPosCurrent.z);
    return;
  }

  bool isHorizontallyAligned = (fmod(pacmanPosCurrent.x - 0.5f, 1.0f) < Global::pacManSpeed);
  bool isVerticallyAligned = (fmod(pacmanPosCurrent.z - 0.5f, 1.0f) < Global::pacManSpeed);

  bool moves[Character::Direction::_Count] = {false, false, false, false};

  moves[Character::Direction::Up] = m_world.IsPassable(static_cast<uint8_t>(pacmanPosCurrent.x), static_cast<uint8_t>(pacmanPosCurrent.z + 1.0f));
  moves[Character::Direction::Right] = m_world.IsPassable(static_cast<uint8_t>(pacmanPosCurrent.x + 1.0f), static_cast<uint8_t>(pacmanPosCurrent.z));
  moves[Character::Direction::Down] = m_world.IsPassable(static_cast<uint8_t>(pacmanPosCurrent.x), static_cast<uint8_t>(pacmanPosCurrent.z - 1.0f));
  moves[Character::Direction::Left] = m_world.IsPassable(static_cast<uint8_t>(pacmanPosCurrent.x - 1.0f), static_cast<uint8_t>(pacmanPosCurrent.z));

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
        m_characters[Characters::Pacman]->AlignToMap();

      if (m_pacmanMovementRequest != pacmanMovement)
        m_characters[Characters::Pacman]->SetMovement(m_pacmanMovementRequest);
    }
    else if (alignment)
    {
      if (m_pacmanMovementRequest == pacmanMovement)
      {
        m_characters[Characters::Pacman]->AlignToMap();
        m_characters[Characters::Pacman]->SetMovement(Character::Movement::Stop);
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

  switch (m_characters[Characters::Pacman]->GetMovement())
  {
  case Character::Movement::Left:
    m_characters[Characters::Pacman]->AdjustPosition(-Global::pacManSpeed, 0, 0);
    break;
  case Character::Movement::Right:
    m_characters[Characters::Pacman]->AdjustPosition(Global::pacManSpeed, 0, 0);
    break;
  case Character::Movement::Up:
    m_characters[Characters::Pacman]->AdjustPosition(0, 0, Global::pacManSpeed);
    break;
  case Character::Movement::Down:
    m_characters[Characters::Pacman]->AdjustPosition(0, 0, -Global::pacManSpeed);
    break;
  default:
    // Nothing
    break;
  }

  uint8_t dotEaten = 0;
  m_dots.Update(static_cast<uint8_t>(pacmanPosCurrent.x), static_cast<uint8_t>(pacmanPosCurrent.z), m_d3dContext.Get(), dotEaten);

  if (dotEaten == 2) // TODO: ugly!
  {
    m_previousPhaseIndex = m_currentPhaseIndex == 0 ? m_previousPhaseIndex : m_currentPhaseIndex;
    m_frightenedTransition = false;
    m_currentPhaseIndex = 0; // Force frightened mode

    CURRENT_PHASE.startingTime = timer.GetTotalSeconds();

    std::for_each(m_characters.begin() + 1, m_characters.end(), [](auto& character) { character->SetSpriteY(Global::ghostFrightenedSpriteRow); });
    std::for_each(m_characters.begin() + 1, m_characters.end(), [](auto& character) { character->ReverseMovementDirection(); });
  }

  // GhosdotEatents
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

  m_world.Draw(m_d3dContext.Get());
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
  cameraPerObjectConstantBuffer.world = m_dots.GetWorldMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  m_dots.Draw(m_d3dContext.Get());

  // Draw pacman
  m_d3dContext->PSSetShaderResources(0, 1, m_pacManShaderResourceView.GetAddressOf());

  if (m_timer.GetFrameCount() % 10 == 0)
    m_characters[Characters::Pacman]->UpdateFrame();

  SetSpriteConstantBufferForCharacter(spriteConstantBuffer, *m_characters[Characters::Pacman]);
  m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &spriteConstantBuffer, sizeof(spriteConstantBuffer));

  cameraPerObjectConstantBuffer.world = m_characters[Characters::Pacman]->GetWorldMatrix();

  m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

  m_characters[Characters::Pacman]->Draw(m_d3dContext.Get());

  // Draw ghosts
  m_d3dContext->PSSetShaderResources(0, 1, m_ghostsShaderResourceView.GetAddressOf());

  if (m_timer.GetFrameCount() % 10 == 0)
    std::for_each(m_characters.begin() + 1, m_characters.end(), [](auto& character) { character->UpdateFrame(); });

  std::for_each(m_characters.begin() + 1, m_characters.end(), [&](auto& character)
  {
    SetSpriteConstantBufferForCharacter(spriteConstantBuffer, *character);
    m_shaderManager->UpdateConstantBuffer(m_frameBuffer.Get(), &spriteConstantBuffer, sizeof(spriteConstantBuffer));

    cameraPerObjectConstantBuffer.world = character->GetWorldMatrix();
    m_shaderManager->UpdateConstantBuffer(m_cameraPerObject.Get(), &cameraPerObjectConstantBuffer, sizeof(cameraPerObjectConstantBuffer));

    character->Draw(m_d3dContext.Get());
  });
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

void Game::MoveCharacterTowardsPosition(float posX, float posZ, Characters characterID)
{
  Character& character = *m_characters[characterID];

  if (m_debugDraw)
    switch (characterID)
    {
      case Characters::Blinky:
        m_debugPoints.push_back({{posX, 0.3f, posZ}, {0.0f ,1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});
        break;
      case Characters::Pinky:
        m_debugPoints.push_back({{posX, 0.3f, posZ}, {0.0f ,1.0f, 0.0f}, {1.0f, 0.6f, 0.8f}});
        break;
      case Characters::Inky:
        m_debugPoints.push_back({{posX, 0.3f, posZ}, {0.0f ,1.0f, 0.0f}, {0.2f, 1.0f, 1.0f}});
        break;
      case Characters::Clyde:
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

void Game::MoveCharacterTowardsRandomPosition(Characters characterID)
{
  const DirectX::XMFLOAT3 charPos = m_characters[characterID]->GetPosition();
  uint8_t direction = rand() % 4;

  switch (direction)
  {
    case 0:
      MoveCharacterTowardsPosition(charPos.x + 1.0f, charPos.z, characterID);
      break;
    case 1:
      MoveCharacterTowardsPosition(charPos.x - 1.0f, charPos.z, characterID);
      break;
    case 2:
      MoveCharacterTowardsPosition(charPos.x, charPos.z + 1.0f, characterID);
      break;
    case 3:
      MoveCharacterTowardsPosition(charPos.x, charPos.z - 1.0f, characterID);
      break;
  }
}

void Game::SetGhostsDefaultSprites()
{
  m_characters[Characters::Blinky]->SetSpriteY(0);
  m_characters[Characters::Pinky]->SetSpriteY(1);
  m_characters[Characters::Inky]->SetSpriteY(2);
  m_characters[Characters::Clyde]->SetSpriteY(3);
}

void Game::CreatePhases()
{
  m_phasesLevel1[0] = { Mode::Frightened, 0.0, 6.0 };
  m_phasesLevel1[1] = { Mode::Scatter, 0.0, 7.0 };
  m_phasesLevel1[2] = { Mode::Chase, 0.0, 20.0 };
  m_phasesLevel1[3] = { Mode::Scatter, 0.0, 7.0 };
  m_phasesLevel1[4] = { Mode::Chase, 0.0, 20.0 };
  m_phasesLevel1[5] = { Mode::Scatter, 0.0, 5.0 };
  m_phasesLevel1[6] = { Mode::Chase, 0.0, 20.0 };
  m_phasesLevel1[7] = { Mode::Scatter, 0.0, 5.0 };
  m_phasesLevel1[8] = { Mode::Chase, 0.0, 0.0 };
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

void Game::UpdatePositionOfBlinky()
{
  switch (CURRENT_PHASE.mode)
  {
    case Mode::Scatter:
      MoveCharacterTowardsPosition(18.5f, 21.5f, Characters::Blinky);
      break;
    case Mode::Chase:
      {
        const DirectX::XMFLOAT3& pacmanPos = m_characters[Characters::Pacman]->GetPosition();
        MoveCharacterTowardsPosition(pacmanPos.x, pacmanPos.z, Characters::Blinky);
      }
      break;
    case Mode::Frightened:
      MoveCharacterTowardsRandomPosition(Characters::Blinky);
      break;
  }
}

void Game::UpdatePositionOfPinky()
{
  if (m_characters[Characters::Pinky]->GetMovement() == Character::Movement::InHouse)
    return;

  switch (CURRENT_PHASE.mode)
  {
    case Mode::Scatter:
      MoveCharacterTowardsPosition(2.5f, 21.5f, Characters::Pinky);
      break;
    case Mode::Chase:
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

        MoveCharacterTowardsPosition(pacmanPos.x, pacmanPos.z, Characters::Pinky);
      }
      break;
    case Mode::Frightened:
      MoveCharacterTowardsRandomPosition(Characters::Pinky);
      break;
  }
}

void Game::UpdatePositionOfInky()
{
  if (m_characters[Characters::Inky]->GetMovement() == Character::Movement::InHouse)
    return;

  switch (CURRENT_PHASE.mode)
  {
    case Mode::Scatter:
      MoveCharacterTowardsPosition(21.5f, 0.0f, Characters::Inky);
      break;
    case Mode::Chase:
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

        MoveCharacterTowardsPosition(finalPosX, finalPosZ, Characters::Inky);
      }
      break;
    case Mode::Frightened:
      MoveCharacterTowardsRandomPosition(Characters::Inky);
      break;
  }
}

void Game::UpdatePositionOfClyde()
{
  if (m_characters[Characters::Clyde]->GetMovement() == Character::Movement::InHouse)
    return;

  switch (CURRENT_PHASE.mode)
  {
    case Mode::Scatter:
      MoveCharacterTowardsPosition(0.0f, 0.0f, Characters::Clyde);
      break;
    case Mode::Chase:
      {
        const DirectX::XMFLOAT3& pacmanPos = m_characters[Characters::Pacman]->GetPosition();
        const DirectX::XMFLOAT3& clydePos = m_characters[Characters::Clyde]->GetPosition();

        float distance = sqrt((clydePos.x - pacmanPos.x) * (clydePos.x - pacmanPos.x) + (clydePos.z - pacmanPos.z) * (clydePos.z - pacmanPos.z));

        if (distance > 8)
        {
          // Behave as blinky
          MoveCharacterTowardsPosition(pacmanPos.x, pacmanPos.z, Characters::Clyde);
        }
        else
        {
          // Scatter
          MoveCharacterTowardsPosition(0.0f, 0.0f, Characters::Clyde);
        }
      }
      break;
    case Mode::Frightened:
      MoveCharacterTowardsRandomPosition(Characters::Clyde);
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

  // Initialize windows-size dependent objects here.
  m_camera.SetProjectionValues(75.0f, static_cast<float>(m_outputWidth) / static_cast<float>(m_outputHeight), 0.1f, 1000.0f);
  m_camera.SetPosition(10.5f, 15.0f, 10.5f);
  //m_camera.SetPosition(10.5f, 5.0f, -2.5f);
  //m_camera.SetPosition(3, 2, 3);
  m_camera.SetLookAtPos(10.5, 0, 10.5);

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
