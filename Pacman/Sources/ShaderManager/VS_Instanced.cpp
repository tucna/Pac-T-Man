#include "pch.h"

#include "VS_Instanced.h"

VS_Instanced::VS_Instanced(const std::wstring & file, ID3D11Device1 * device)
{
  Microsoft::WRL::ComPtr<ID3DBlob> blob;
  D3DReadFileToBlob(file.c_str(), &blob);

  device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, GetShaderPP());

  const D3D11_INPUT_ELEMENT_DESC ied[] =
  {
    { "Position" ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
    { "Normal"   ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
    { "Color"    ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
    { "AdjPos"   ,0,DXGI_FORMAT_R32G32B32_FLOAT,1,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_INSTANCE_DATA,1 },
    { "SizeFlag" ,0,DXGI_FORMAT_R8_UINT,1,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_INSTANCE_DATA,1 }
  };

  device->CreateInputLayout(ied, (UINT)std::size(ied), blob->GetBufferPointer(), blob->GetBufferSize(), m_inputLayout.GetAddressOf());
}

VS_Instanced::~VS_Instanced()
{
}

void VS_Instanced::ActivateShader(ID3D11DeviceContext1* context)
{
  context->IASetInputLayout(m_inputLayout.Get());
  context->VSSetShader(GetShader(), NULL, 0);
}

void VS_Instanced::BindConstantBuffers(ID3D11DeviceContext1* context, ID3D11Buffer** constantBuffers, uint8_t numberOfConstantBuffers)
{
  if (constantBuffers && numberOfConstantBuffers > 0)
    context->VSSetConstantBuffers(0, numberOfConstantBuffers, constantBuffers);
}
