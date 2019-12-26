#pragma once

#include <vector>

#include "Global.h"

class Caption
{
public:
  Caption();
  ~Caption();

  void Draw(ID3D11DeviceContext1* context);
  void Init(ID3D11Device1* device, uint16_t width, uint16_t height);

  void AdjustY(float value, float minimum);
  void SetPosition(float x, float y, float z);

  float GetOffsetY() { return 0.6f;/*return m_offsetY; TODO*/ }
  uint16_t GetWidth() { return m_width; }
  uint16_t GetHeight() { return m_height; }

  const DirectX::XMFLOAT4X4& GetWorldMatrix() const { return m_worldMatrix; }

private:
  void UpdateWorldMatrix();

  Microsoft::WRL::ComPtr<ID3D11Buffer>             m_vertexBuffer;
  Microsoft::WRL::ComPtr<ID3D11RasterizerState>    m_cullNone;
  Microsoft::WRL::ComPtr<ID3D11Resource>           m_resource;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
  Microsoft::WRL::ComPtr<ID3D11SamplerState>       m_samplerState;
  Microsoft::WRL::ComPtr<ID3D11BlendState>         m_blendState;
  Microsoft::WRL::ComPtr<ID3D11Buffer>             m_indexBuffer;

  DirectX::XMFLOAT3 m_position;
  DirectX::XMFLOAT4X4 m_worldMatrix;

  std::vector<Global::Vertex> m_vertices;
  std::vector<uint16_t> m_indices;

  uint16_t m_width;
  uint16_t m_height;
};
