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

  const DirectX::XMMATRIX& GetWorldMatrix() const noexcept;

  bool IsPassable(uint8_t column, uint8_t row);

private:
  void Generate(ID3D11Device1* device);

  const uint8_t m_map[Global::worldSize][Global::worldSize];

  DirectX::XMMATRIX m_worldMatrix;

  std::vector<Global::Vertex> m_vertices;
  std::vector<unsigned short> m_indices;

  Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

  Microsoft::WRL::ComPtr <ID3D11RasterizerState>  m_cullCW;
};

