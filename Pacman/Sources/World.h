#pragma once

#include <vector>

#include "Global.h"

class World
{
public:
  explicit World();
  ~World();

  void Init(ID3D11Device1* device);
  void Draw(ID3D11DeviceContext1* context);

  bool IsPassable(uint8_t column, uint8_t row);

  const DirectX::XMFLOAT4X4& GetWorldMatrix() const { return m_worldMatrix; }

private:
  void AddBlock(uint8_t x, uint8_t z, float depth, bool north, bool west, bool south, bool east);
  void Generate(ID3D11Device1* device);

  const uint8_t m_map[Global::worldSize][Global::worldSize];

  Microsoft::WRL::ComPtr<ID3D11Buffer>           m_vertexBuffer;
  Microsoft::WRL::ComPtr<ID3D11Buffer>           m_indexBuffer;
  Microsoft::WRL::ComPtr<ID3D11RasterizerState>  m_cullCW;

  DirectX::XMFLOAT4X4 m_worldMatrix;

  std::vector<Global::Vertex> m_vertices;
  std::vector<uint16_t> m_indices;
};

