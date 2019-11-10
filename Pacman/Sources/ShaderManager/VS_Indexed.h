#pragma once

#include "IShader.h"

class VS_Indexed : public IShader<ID3D11VertexShader>
{
public:
  VS_Indexed(const std::wstring& file, ID3D11Device1* device);
  ~VS_Indexed();

  // From IShader
  virtual void ActivateShader(ID3D11DeviceContext1* context, ID3D11Buffer** constantBuffers = nullptr, uint8_t numberOfConstantBuffers = 0) override;
  virtual void BindConstantBuffers(ID3D11DeviceContext1* context, ID3D11Buffer** constantBuffers, uint8_t numberOfConstantBuffers);

private:
  Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
};
