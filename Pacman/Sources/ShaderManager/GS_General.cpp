#include "pch.h"

#include "GS_General.h"

GS_General::GS_General(const std::wstring & file, ID3D11Device1* device)
{
  Microsoft::WRL::ComPtr<ID3DBlob> blob;
  D3DReadFileToBlob(file.c_str(), &blob);

  device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, GetShaderPP());
}

GS_General::~GS_General()
{
}

void GS_General::ActivateShader(ID3D11DeviceContext1* context, ID3D11Buffer** constantBuffers, uint8_t numberOfConstantBuffers)
{
  context->GSSetShader(GetShader(), NULL, 0);
}

void GS_General::BindConstantBuffers(ID3D11DeviceContext1* context, ID3D11Buffer** constantBuffers, uint8_t numberOfConstantBuffers)
{
  if (constantBuffers && numberOfConstantBuffers > 0)
    context->GSSetConstantBuffers(0, numberOfConstantBuffers, constantBuffers);
}
