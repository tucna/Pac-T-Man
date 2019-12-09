#include "pch.h"

#include "PS_OneCB.h"

PS_OneCB::PS_OneCB(const std::wstring& file, ID3D11Device1* device)
{
  Microsoft::WRL::ComPtr<ID3DBlob> blob;
  D3DReadFileToBlob(file.c_str(), &blob);

  device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, GetShaderPP());
}

void PS_OneCB::ActivateShader(ID3D11DeviceContext1 * context)
{
  context->PSSetShader(GetShader(), NULL, 0);
}

void PS_OneCB::BindConstantBuffers(ID3D11DeviceContext1* context, ID3D11Buffer** constantBuffers, uint8_t numberOfConstantBuffers)
{
  if (constantBuffers && numberOfConstantBuffers > 0)
    context->PSSetConstantBuffers(0, numberOfConstantBuffers, constantBuffers);
}
