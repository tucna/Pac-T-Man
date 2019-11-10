#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <string>
#include <wrl.h>

template <typename T>
class IShader
{
public:
  virtual ~IShader() {}

  T*  GetShader()   { return m_shader.Get(); }
  T** GetShaderPP() { return m_shader.GetAddressOf(); }

  virtual void ActivateShader(ID3D11DeviceContext1* context, ID3D11Buffer** constantBuffers = nullptr, uint8_t numberOfConstantBuffers = 0) = 0;
  virtual void BindConstantBuffers(ID3D11DeviceContext1* context, ID3D11Buffer** constantBuffers, uint8_t numberOfConstantBuffers) {} // TUCNA this should be a pure virtual

private:
  Microsoft::WRL::ComPtr<T> m_shader;
};