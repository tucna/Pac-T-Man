#pragma once

namespace Global
{

// It must be here because it is not a game state but more like a character state
enum class Mode
{
  Chase,
  Scatter,
  Frightened
};

constexpr uint8_t worldSize = 21;
constexpr uint8_t minFramesPerDirection = 3;
constexpr uint8_t phasesNum = 9;

constexpr uint8_t rowBlinky = 0;
constexpr uint8_t rowPinky = 1;
constexpr uint8_t rowInky = 2;
constexpr uint8_t rowClyde = 3;
constexpr uint8_t rowFrightened = 4;
constexpr uint8_t rowTransition = 5;
constexpr uint8_t rowDead = 6;
constexpr uint8_t numGhosts = 4;

constexpr float pacManSpeed = 0.1f;
constexpr float ghostSpeed = 0.08f;
constexpr float pacManSize = 1.0f;
constexpr float ghostSize = 1.0f;
constexpr float pacManHalfSize = pacManSize / 2.0f;

struct Vertex
{
  DirectX::XMFLOAT3 position;
  DirectX::XMFLOAT3 normal;
  DirectX::XMFLOAT3 color;
};

struct CameraPerFrame
{
  DirectX::XMFLOAT4X4 view;
  DirectX::XMFLOAT4X4 projection;
};

static_assert((sizeof(CameraPerFrame) % 16) == 0, "CameraPerFrame is not aligned!");

struct CameraPerObject
{
  DirectX::XMFLOAT4X4 world;
};

static_assert((sizeof(CameraPerObject) % 16) == 0, "CameraPerObject is not aligned!");

struct SpriteConstantBuffer
{
  uint32_t spriteX;
  uint32_t spriteY;
  uint32_t spriteSheetColumns;
  uint32_t spriteSheetRows;

  DirectX::XMFLOAT4 billboardSize_0_0_0;
};

static_assert((sizeof(SpriteConstantBuffer) % 16) == 0, "SpriteConstantBuffer is not aligned!");
}
