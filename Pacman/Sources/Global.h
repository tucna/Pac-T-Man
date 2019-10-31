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
  DirectX::XMFLOAT3 pos;
  DirectX::XMFLOAT3 n;
  DirectX::XMFLOAT3 col;
};

struct CameraConstantBuffer
{
  DirectX::XMMATRIX world;
  DirectX::XMMATRIX view;
  DirectX::XMMATRIX projection;
};

struct CameraPerFrame
{
  DirectX::XMMATRIX view;
  DirectX::XMMATRIX projection;
};

struct CameraPerObject
{
  DirectX::XMMATRIX world;
};

struct SpriteConstantBuffer
{
  uint32_t spriteX;
  uint32_t spriteY;
  uint32_t spriteSheetColumns;
  uint32_t spriteSheetRows;

  DirectX::XMFLOAT4 billboardSize_0_0_0;
};

static_assert((sizeof(SpriteConstantBuffer) % 16) == 0, "SpriteConstantBuffer is not alligned!");
}
