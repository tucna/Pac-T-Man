#pragma once

namespace Global
{

constexpr uint8_t worldSize = 21;
constexpr float speed = 0.1f;
constexpr float pacManSize = 1.0f;

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

struct FrameConstantBuffer
{
  DirectX::XMFLOAT2 frameID;
  DirectX::XMFLOAT2 framesNumber;
  DirectX::XMFLOAT4 billboardSize_0_0_0;
};

}