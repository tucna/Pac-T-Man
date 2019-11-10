#pragma once

#include "IShader.h"

class GS_General : public IShader<ID3D11GeometryShader>
{
public:
  GS_General(const std::wstring& file, ID3D11Device1* device);
  ~GS_General();

  // From IShader
  virtual void ActivateShader(ID3D11DeviceContext1* context, ID3D11Buffer** constantBuffers = nullptr, uint8_t numberOfConstantBuffers = 0) override;
  virtual void BindConstantBuffers(ID3D11DeviceContext1* context, ID3D11Buffer** constantBuffers, uint8_t numberOfConstantBuffers);
};
