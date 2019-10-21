#include "pch.h"

#include "Character.h"
#include "WICTextureLoader.h"

Character::Character() :
  m_currentFrame(0),
  m_direction(0),
  m_position(0, 0, 0),
  m_movement(Movement::Stop)
{
}

Character::~Character()
{
}

void Character::SetPosition(float x, float y, float z)
{
  m_position.x = x;
  m_position.y = y;
  m_position.z = z;
}

void Character::SetFrame(uint8_t frameID)
{
  m_currentFrame = frameID;
}

void Character::AdjustPosition(float x, float y, float z)
{
  m_position.x += x;
  m_position.y += y;
  m_position.z += z;
}

DirectX::XMFLOAT3 Character::GetPosition()
{
  return m_position;
}

uint8_t Character::GetFrame()
{
  return m_currentFrame;
}

void Character::SetRowInSheet(uint8_t direction)
{
  m_direction = direction;
}

uint8_t Character::GetRowInSheet()
{
  return m_direction;
}

void Character::SetMovement(Movement movement)
{
  m_movement = movement;
}

Character::Movement Character::GetMovement()
{
  return m_movement;
}

void Character::Update(uint8_t coefMod, uint8_t coefAdd)
{
  m_currentFrame = ++m_currentFrame % coefMod + coefAdd;
}

void Character::Init(ID3D11Device1* device, const wchar_t* fileName)
{
  // Texture - check HR
  CreateWICTextureFromFile(device, nullptr, fileName, m_resource.GetAddressOf(), m_shaderResourceView.GetAddressOf());

  m_vertices.push_back({{0,0,0}, {0.0, 1.0, 0.0}, {0.8, 0.0, 0.0}});

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

  // Rasterizer
  D3D11_RASTERIZER_DESC cmdesc = {};
  cmdesc.FillMode = D3D11_FILL_SOLID;
  cmdesc.FrontCounterClockwise = false;
  cmdesc.CullMode = D3D11_CULL_NONE;

  // TUCNA check HR
  device->CreateRasterizerState(&cmdesc, m_cullNone.GetAddressOf());

  // Sampler
  D3D11_SAMPLER_DESC sampDesc = {};
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = 0;

  //Create the Sample State
  device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf());

  // Instances
  m_instances.push_back({{0,0,0}});

  // Set up the description of the instance buffer.
  D3D11_BUFFER_DESC instanceBufferDesc = {};
  instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  instanceBufferDesc.ByteWidth = sizeof(InstanceType) * (UINT)m_instances.size();
  instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  instanceBufferDesc.CPUAccessFlags = 0;
  instanceBufferDesc.MiscFlags = 0;
  instanceBufferDesc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA instanceData = {};
  instanceData.pSysMem = m_instances.data();
  instanceData.SysMemPitch = 0;
  instanceData.SysMemSlicePitch = 0;

  // Create the instance buffer.
  device->CreateBuffer(&instanceBufferDesc, &instanceData, m_instanceBuffer.GetAddressOf());
}

void Character::Draw(ID3D11DeviceContext1* context)
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

  context->DrawInstanced(1, (UINT)m_instances.size(), 0, 0);
}

void Character::AlignToMap()
{
  m_position.x = floor(m_position.x) + 0.5f;
  m_position.z = floor(m_position.z) + 0.5f;

  /*
  unsigned int mod_x = static_cast<unsigned int>(round(m_position.x * 100)) % 50;
  unsigned int mod_z = static_cast<unsigned int>(round(m_position.z * 100)) % 50;

  unsigned int diff_x = mod_x < 2 ? mod_x : 5 - mod_x;
  unsigned int diff_z = mod_z < 2 ? mod_z : 5 - mod_z;

  m_position.x += diff_x / 10.0f;
  m_position.z += diff_z / 10.0f;
  */
}

DirectX::XMMATRIX Character::GetWorldMatrix() const noexcept
{
  return DirectX::XMMatrixTranspose(
    DirectX::XMMatrixTranslation(
      m_position.x,
      m_position.y,
      m_position.z
    ));
}
