#pragma once

#include "ShaderManager/ShaderManager.h"

#include "Camera.h"
#include "Dots.h"
#include "Global.h"
#include "Ghost.h"
#include "Character.h"
#include "Keyboard.h"
#include "Pacman.h"
#include "StepTimer.h"
#include "World.h"

class Game
{
public:
  enum class Ghosts
  {
    Blinky,
    Pinky,
    Inky,
    Clyde,
    None
  };

  struct Phase
  {
    Global::Mode mode;
    double startingTime;
    double duration;
  };

  Game() noexcept;

  // Initialization and management
  void Initialize(HWND window, uint16_t width, uint16_t height);

  // Basic game loop
  void Tick();

  // Messages
  void OnActivated();
  void OnDeactivated();
  void OnSuspending();
  void OnResuming();
  void OnWindowSizeChanged(uint16_t width, uint16_t height);

  // Properties
  void GetDefaultSize(uint16_t& width, uint16_t& height) const;

private:
  void DrawWorld();
  void DrawSprites();
  void DrawDebug();

  void UpdatePositionOfBlinky();
  void UpdatePositionOfPinky();
  void UpdatePositionOfInky();
  void UpdatePositionOfClyde();
  void Update(const DX::StepTimer& timer);

  void Render();
  void Clear();
  void Present();

  void CreateDevice();
  void CreateResources();

  void OnDeviceLost();

  void SetSpriteConstantBufferForCharacter(Global::SpriteConstantBuffer& spriteConstantBuffer, const Character& character);
  void MoveCharacterTowardsPosition(float posX, float posZ, Character& character);
  void MoveCharacterTowardsRandomPosition(Character& character);
  void SetGhostsDefaultSprites();
  void CreatePhases();
  void HandleCollisions();

  bool AreMovementsOppositeOrSame(Character::Movement m1, Character::Movement m2);

  float DistanceBetweenCharacters(const Character& ch1, const Character& ch2);

  D3D_FEATURE_LEVEL                                 m_featureLevel;
  Microsoft::WRL::ComPtr<ID3D11Device1>             m_d3dDevice;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext1>      m_d3dContext;
  Microsoft::WRL::ComPtr<IDXGISwapChain1>           m_swapChain;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView>    m_renderTargetView;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilView>    m_depthStencilView;
  Microsoft::WRL::ComPtr<ID3D11Buffer>              m_projectionMatrixConstantBuffer;
  Microsoft::WRL::ComPtr<ID3D11Buffer>              m_frameBuffer;
  Microsoft::WRL::ComPtr<ID3D11Buffer>              m_cameraPerFrame;
  Microsoft::WRL::ComPtr<ID3D11Buffer>              m_cameraPerObject;
  Microsoft::WRL::ComPtr<ID3D11Resource>            m_pacManResource;
  Microsoft::WRL::ComPtr<ID3D11Resource>            m_ghostsResource;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  m_pacManShaderResourceView;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  m_ghostsShaderResourceView;

  HWND m_window;
  DX::StepTimer m_timer;
  Character::Movement m_pacmanMovementRequest;
  Phase m_frightenedPhase;

  Camera m_camera;
  Dots m_dots;
  World m_world;

  std::unique_ptr<ShaderManager> m_shaderManager;
  std::unique_ptr<DirectX::Keyboard> m_keyboard;

  std::vector<Global::Vertex> m_debugPoints;

  std::array<std::unique_ptr<Ghost>, Global::numGhosts> m_ghosts;
  std::unique_ptr<Pacman> m_pacman;

  std::array<Phase, Global::phasesNum> m_phasesLevel1; // Level 1 have 8 phases + two more frightened (blue + blink)

  uint8_t m_currentPhaseIndex;
  uint8_t m_previousPhaseIndex;
  Ghosts m_currentGhostCounter;

  bool m_debugDraw;
  bool m_frightenedTransition;

  uint16_t m_outputWidth;
  uint16_t m_outputHeight;
};
