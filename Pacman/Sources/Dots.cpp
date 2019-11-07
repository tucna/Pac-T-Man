#include "pch.h"

#include "Dots.h"
#include "Global.h"
#include "WICTextureLoader.h"

using namespace DirectX;

Dots::Dots() :
  m_dots{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0},
    {0, 0, 2, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 2, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 2, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 2, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
{
  XMStoreFloat4x4(&m_worldMatrix, DirectX::XMMatrixIdentity()); // No need to transpose identity matrix
}

Dots::~Dots()
{
}

void Dots::Draw(ID3D11DeviceContext1* context)
{
  unsigned int strides[2];
  unsigned int offsets[2];

  // Set the buffer strides.
  strides[0] = sizeof(Global::Vertex);
  strides[1] = sizeof(InstanceType);

  // Set the buffer offsets.
  offsets[0] = 0;
  offsets[1] = 0;

  // Set the array of pointers to the vertex and instance buffers
  ID3D11Buffer* bufferPointers[2];
  bufferPointers[0] = m_vertexBuffer.Get();
  bufferPointers[1] = m_instanceBuffer.Get();

  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
  context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

  context->PSSetShaderResources(0, 1, m_shaderResourceView.GetAddressOf());
  context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

  context->RSSetState(m_cullNone.Get());

  context->OMSetBlendState(m_blendState.Get(), NULL, 0xffffffff);

  context->DrawInstanced(1, (UINT)m_instances.size(), 0, 0);
}

void Dots::Init(ID3D11Device1* device)
{
  DX::ThrowIfFailed(CreateWICTextureFromFile(device, nullptr, L"Resources/dot.png", m_resource.GetAddressOf(), m_shaderResourceView.GetAddressOf()));

  m_vertices.push_back({{0, 0, 0}, {0.0, 1.0, 0.0}, {0.8, 0.0, 0.0}});

  // Vertex buffer
  D3D11_BUFFER_DESC bd = {};
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.CPUAccessFlags = 0;
  bd.MiscFlags = 0;
  bd.ByteWidth = UINT(sizeof(Global::Vertex) * m_vertices.size());
  bd.StructureByteStride = sizeof(Global::Vertex);

  D3D11_SUBRESOURCE_DATA sd = {};
  sd.pSysMem = m_vertices.data();
  device->CreateBuffer(&bd, &sd, &m_vertexBuffer);

  D3D11_RASTERIZER_DESC cmdesc = {};
  cmdesc.FillMode = D3D11_FILL_SOLID;
  cmdesc.FrontCounterClockwise = false;
  cmdesc.CullMode = D3D11_CULL_NONE;

  DX::ThrowIfFailed(device->CreateRasterizerState(&cmdesc, m_cullNone.GetAddressOf()));

  // Sampler
  D3D11_SAMPLER_DESC sampDesc = {};
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = 0;

  //Create the sample state
  DX::ThrowIfFailed(device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf()));

  // Blend state
  D3D11_BLEND_DESC omDesc = {};
  omDesc.RenderTarget[0].BlendEnable = true;
  omDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  omDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  omDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  omDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  omDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  omDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  omDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

  // check HR
  DX::ThrowIfFailed(device->CreateBlendState(&omDesc, m_blendState.GetAddressOf()));

  for (unsigned int z = 0; z != Global::worldSize; z++)
    for (unsigned int x = 0; x != Global::worldSize; x++)
      if (m_dots[z][x] > 0)
        m_instances.push_back({DirectX::XMFLOAT3(static_cast<float>(x) + 0.5f, 0.25f, static_cast<float>(z) + 0.5f), m_dots[z][x]});

  // Set up the description of the instance buffer
  D3D11_BUFFER_DESC instanceBufferDesc = {};
  instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  instanceBufferDesc.MiscFlags = 0;
  instanceBufferDesc.ByteWidth = sizeof(InstanceType) * (UINT)m_instances.size();
  instanceBufferDesc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA instanceData = {};
  instanceData.pSysMem = m_instances.data();
  instanceData.SysMemPitch = 0;
  instanceData.SysMemSlicePitch = 0;

  // Create the instance buffer
  DX::ThrowIfFailed(device->CreateBuffer(&instanceBufferDesc, &instanceData, m_instanceBuffer.GetAddressOf()));
}

void Dots::Update(uint8_t column, uint8_t row, ID3D11DeviceContext1* context)
{
  if (m_dots[row][column] > 0)
  {
    m_dots[row][column] = 0;

    m_instances.clear();

    for (unsigned int z = 0; z != Global::worldSize; z++)
      for (unsigned int x = 0; x != Global::worldSize; x++)
        if (m_dots[z][x] > 0)
          m_instances.push_back({DirectX::XMFLOAT3(static_cast<float>(x) + 0.5f, 0.25f, static_cast<float>(z) + 0.5f), m_dots[z][x]});

    if (m_instances.size() == 0)
    {
      // TODO: game end
    }
    else
    {
      D3D11_MAPPED_SUBRESOURCE resource;
      context->Map(m_instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
      memcpy(resource.pData, m_instances.data(), m_instances.size() * sizeof(InstanceType));
      context->Unmap(m_instanceBuffer.Get(), 0);
    }
  }
}
