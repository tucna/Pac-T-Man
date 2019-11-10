#pragma once

#include "IShader.h"

class VS_Instanced : public IShader<ID3D11VertexShader>
{
public:
  VS_Instanced(const std::wstring& file, ID3D11Device1* device);
  ~VS_Instanced();

  // From IShader
  virtual void ActivateShader(ID3D11DeviceContext1* context, ID3D11Buffer** constantBuffers = nullptr, uint8_t numberOfConstantBuffers = 0) override; // TUCNA posledni dva ignore pro ted
  virtual void BindConstantBuffers(ID3D11DeviceContext1* context, ID3D11Buffer** constantBuffers, uint8_t numberOfConstantBuffers);

private:
  Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
};
