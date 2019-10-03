#pragma once

namespace Global
{

constexpr uint8_t worldSize = 21;

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

}
