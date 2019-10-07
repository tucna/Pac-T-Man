#pragma once

#include <vector>

#include "Global.h"

class Dots
{
public:
  struct InstanceType
  {
    DirectX::XMFLOAT3 newPosition;
  };

  explicit Dots();
  ~Dots();

  void Draw(ID3D11DeviceContext1* context);
  void Init(ID3D11Device1* device);

  DirectX::XMMATRIX GetWorldMatrix() const noexcept;

private:
  const uint8_t m_dots[Global::worldSize][Global::worldSize];

  unsigned short m_numberOfDots;

  std::vector<Global::Vertex> m_vertices;
  std::vector<InstanceType>   m_instances;

  Microsoft::WRL::ComPtr<ID3D11Buffer>              m_vertexBuffer;
  Microsoft::WRL::ComPtr<ID3D11RasterizerState>     m_cullNone;
  Microsoft::WRL::ComPtr<ID3D11Resource>            m_resource;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  m_shaderResourceView;
  Microsoft::WRL::ComPtr<ID3D11SamplerState>        m_samplerState;
  Microsoft::WRL::ComPtr<ID3D11BlendState>          m_blendState;
  Microsoft::WRL::ComPtr<ID3D11Buffer>              m_instanceBuffer;
};

