#pragma once

#include "ShaderManager/include/ShaderManager.h"

#include "Camera.h"
#include "Dots.h"
#include "Global.h"
#include "Character.h"
#include "Keyboard.h"
#include "StepTimer.h"
#include "World.h"

class Game
{

  TODO> fix camera, fix types (there should be XMFLOAT4 as member not XMVECTOR - XMVECTOR should be used just for processing and be converted to it), write type strong enum array initialization

public:
  enum Characters
  {
    Pacman,
    Blinky,
    Pinky,
    Inky,
    Clyde
  };

  enum class Mode
  {
    Chase,
    Scatter
  };

  Game() noexcept;

  // Initialization and management
  void Initialize(HWND window, int width, int height);

  // Basic game loop
  void Tick();

  // Messages
  void OnActivated();
  void OnDeactivated();
  void OnSuspending();
  void OnResuming();
  void OnWindowSizeChanged(int width, int height);

  // Properties
  void GetDefaultSize( int& width, int& height ) const;

private:
  void DrawWorld();
  void DrawSprites();
  void DrawDebug();

  void MoveCharacterTowardsPosition(float posX, float posZ, Character& character);
  void UpdatePositionOfBlinky();
  void UpdatePositionOfPinky();
  void UpdatePositionOfInky();
  void UpdatePositionOfClyde();

  void Update(DX::StepTimer const& timer);
  void Render();

  void Clear();
  void Present();

  void CreateDevice();
  void CreateResources();

  void OnDeviceLost();

  // Device resources
  HWND                                            m_window;
  int                                             m_outputWidth;
  int                                             m_outputHeight;

  D3D_FEATURE_LEVEL                               m_featureLevel;
  Microsoft::WRL::ComPtr<ID3D11Device1>           m_d3dDevice;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    m_d3dContext;

  Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_depthStencilView;
  Microsoft::WRL::ComPtr<ID3D11Buffer>            m_constantBuffer;
  Microsoft::WRL::ComPtr<ID3D11Buffer>            m_frameBuffer;

  Microsoft::WRL::ComPtr<ID3D11Buffer>            m_cameraPerFrame;
  Microsoft::WRL::ComPtr<ID3D11Buffer>            m_cameraPerObject;

  // Rendering loop timer.
  DX::StepTimer                                   m_timer;

  Camera                                          m_camera;
  Dots                                            m_dots;
  World                                           m_world;
  Character                                       m_pacman;

  Character m_blinky;
  Character m_pinky;
  Character m_inky;
  Character m_clyde;

  std::unique_ptr<ShaderManager>                  m_shaderManager;

  std::unique_ptr<DirectX::Keyboard>              m_keyboard;

  Character::Movement m_pacmanMovementRequest;

  Microsoft::WRL::ComPtr<ID3D11Resource>            m_pacManResource;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  m_pacManShaderResourceView;

  Microsoft::WRL::ComPtr<ID3D11Resource>            m_ghostsResource;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  m_ghostsShaderResourceView;

  bool m_debugDraw;
  std::vector<Global::Vertex> m_debugPoints;
  Mode m_currentMode;
};