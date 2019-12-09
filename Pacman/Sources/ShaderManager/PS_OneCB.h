#pragma once

#include "IShader.h"

class PS_OneCB : public IShader<ID3D11PixelShader>
{
public:
  PS_OneCB(const std::wstring& file, ID3D11Device1* device);
  ~PS_OneCB() {}

  // From IShader
  virtual void ActivateShader(ID3D11DeviceContext1* context) override;
  virtual void BindConstantBuffers(ID3D11DeviceContext1* context, ID3D11Buffer** constantBuffers, uint8_t numberOfConstantBuffers) override;
};

