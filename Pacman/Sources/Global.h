#pragma once

namespace Global
{

constexpr uint8_t worldSize = 21;
constexpr uint8_t minFramesPerDirection = 3;

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

struct CameraPerObject
{
  DirectX::XMFLOAT4X4 world;
};

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
