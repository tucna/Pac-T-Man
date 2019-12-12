#include "pch.h"

#include "Caption.h"
#include "WICTextureLoader.h"

using namespace DirectX;

Caption::Caption()
{
  XMMATRIX view = XMMatrixIdentity();
  XMMATRIX projection = XMMatrixOrthographicOffCenterLH(0.0f, 800.0f, 600.0f, 0.0f, 0.0f, 100.0f); // TODO: stupid, here should be real camera values not 800x600
  XMMATRIX worldMatrix = XMMatrixMultiply(view, projection);

  float spriteWidth = 800.0f,
        spriteHeight = 600.0f,
        spritePosX = 0.0f,
        spritePosY = 0.0f;

  XMMATRIX l_translation = XMMatrixTranslation(spritePosX, spritePosY, 0.0f);
  XMMATRIX l_rotationZ = XMMatrixRotationZ(0.0f);
  XMMATRIX l_scale = XMMatrixScaling(1.0f * spriteWidth, 1.0f * spriteHeight, 1.0f);
  XMMATRIX l_spriteWVP = l_scale * l_rotationZ * l_translation; // TODO: all of these is wrong

  // __________ Prepare World Coordinates to send to the shader
  XMMATRIX l_worldMatrix = XMMatrixMultiply(l_spriteWVP, worldMatrix);

  XMStoreFloat4x4(&m_worldMatrix, XMMatrixTranspose(l_worldMatrix));

  //XMStoreFloat4x4(&m_worldMatrix, DirectX::XMMatrixIdentity()); // No need to transpose identity matrix
}

Caption::~Caption()
{
}

void Caption::Draw(ID3D11DeviceContext1 * context)
{
  unsigned int stride = sizeof(Global::Vertex);
  unsigned int offset = 0;

  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
  context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);

  context->PSSetShaderResources(0, 1, m_shaderResourceView.GetAddressOf());
  context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

  context->RSSetState(m_cullNone.Get());

  context->OMSetBlendState(m_blendState.Get(), NULL, 0xffffffff);

  context->DrawIndexed(6, 0, 0);
}

void Caption::Init(ID3D11Device1 * device)
{
  DX::ThrowIfFailed(CreateWICTextureFromFile(device, nullptr, L"Resources/caption.png", m_resource.GetAddressOf(), m_shaderResourceView.GetAddressOf()));

  // Vertex buffer
  m_vertices.push_back({{-0.8f, -0.2f, 0.0f}, {0.0, 1.0, 0.0}, {0.0, 1.0, 0.0}, {0, 1} });
  m_vertices.push_back({{-0.8f,  0.2f, 0.0f}, {0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, {0, 0} });
  m_vertices.push_back({{0.8f, -0.2f, 0.0f}, {0.0, 1.0, 0.0}, {1.0, 1.0, 0.0}, {1, 1} });
  m_vertices.push_back({{0.8f,  0.2f, 0.0f}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0}, {1, 0} });

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

  // Index buffer
  m_indices.push_back(0);
  m_indices.push_back(1);
  m_indices.push_back(2);
  m_indices.push_back(2);
  m_indices.push_back(1);
  m_indices.push_back(3);

  D3D11_BUFFER_DESC ibd = {};
  ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  ibd.Usage = D3D11_USAGE_DEFAULT;
  ibd.CPUAccessFlags = 0;
  ibd.MiscFlags = 0;
  ibd.ByteWidth = UINT(m_indices.size() * sizeof(uint16_t));
  ibd.StructureByteStride = sizeof(uint16_t);

  D3D11_SUBRESOURCE_DATA isd = {};
  isd.pSysMem = m_indices.data();
  DX::ThrowIfFailed(device->CreateBuffer(&ibd, &isd, &m_indexBuffer));

  // Rasterizer
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

  DX::ThrowIfFailed(device->CreateBlendState(&omDesc, m_blendState.GetAddressOf()));
}
