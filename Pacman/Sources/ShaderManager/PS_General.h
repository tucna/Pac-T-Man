#pragma once

#include "IShader.h"

class PS_General : public IShader<ID3D11PixelShader>
{
public:
  PS_General(const std::wstring& file, ID3D11Device1* device);
  ~PS_General();

  // From IShader
  virtual void ActivateShader(ID3D11DeviceContext1* context) override;
  virtual void BindConstantBuffers(ID3D11DeviceContext1* context, ID3D11Buffer** constantBuffers, uint8_t numberOfConstantBuffers) override;
};
