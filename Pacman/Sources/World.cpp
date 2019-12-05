#include "pch.h"

#include "Global.h"
#include "World.h"

using namespace DirectX;

World::World() :
  m_map{
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0},
    {0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0},
    {0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
    {0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
    {2, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 2},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 4, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 1, 0, 1, 0, 1, 1, 3, 1, 1, 0, 1, 0, 1, 1, 1, 1, 2},
    {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0}}
{
  XMStoreFloat4x4(&m_worldMatrix, DirectX::XMMatrixIdentity()); // No need to transpose identity matrix
}

World::~World()
{
}

void World::AddBlock(float x, float z, float depth, bool north, bool west, bool south, bool east)
{
  auto AddIndces = [&]()
  {
    uint16_t lastIndex = m_indices.empty() ? 0 : m_indices.back() + 1;
    m_indices.push_back(lastIndex + 0);
    m_indices.push_back(lastIndex + 1);
    m_indices.push_back(lastIndex + 2);
    m_indices.push_back(lastIndex + 0);
    m_indices.push_back(lastIndex + 2);
    m_indices.push_back(lastIndex + 3);
  };

  // Top side
  m_vertices.push_back({{x    , 0.5, z    }, {0.0, 1.0, 0.0}, {0.8, 0.0, 0.0}});
  m_vertices.push_back({{x    , 0.5, z + depth}, {0.0, 1.0, 0.0}, {0.8, 0.0, 0.0}});
  m_vertices.push_back({{x + 1, 0.5, z + depth}, {0.0, 1.0, 0.0}, {0.8, 0.0, 0.0}});
  m_vertices.push_back({{x + 1, 0.5, z    }, {0.0, 1.0, 0.0}, {0.8, 0.0, 0.0}});

  AddIndces();

  // 2st side
  if (west)
  {
    m_vertices.push_back({{x    , 0.0, z    }, {-1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});
    m_vertices.push_back({{x    , 0.0, z + depth}, {-1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});
    m_vertices.push_back({{x    , 0.5, z + depth}, {-1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});
    m_vertices.push_back({{x    , 0.5, z    }, {-1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});

    AddIndces();
  }

  // 3rd side
  if (north)
  {
    m_vertices.push_back({{x    , 0.0, z + depth}, {0.0, 0.0, 1.0}, {0.2, 0.0, 0.8}});
    m_vertices.push_back({{x + 1, 0.0, z + depth}, {0.0, 0.0, 1.0}, {0.2, 0.0, 0.8}});
    m_vertices.push_back({{x + 1, 0.5, z + depth}, {0.0, 0.0, 1.0}, {0.2, 0.0, 0.8}});
    m_vertices.push_back({{x    , 0.5, z + depth}, {0.0, 0.0, 1.0}, {0.2, 0.0, 0.8}});

    AddIndces();
  }

  // 4th side
  if (east)
  {
    m_vertices.push_back({{x + 1, 0.0, z + depth}, {1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});
    m_vertices.push_back({{x + 1, 0.0, z    }, {1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});
    m_vertices.push_back({{x + 1, 0.5, z    }, {1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});
    m_vertices.push_back({{x + 1, 0.5, z + depth}, {1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});

    AddIndces();
  }

  // 5st side
  if (south)
  {
    m_vertices.push_back({{x + 1, 0.0, z    }, {0.0, 0.0, -1.0}, {0.2, 0.0, 0.8}});
    m_vertices.push_back({{x    , 0.0, z    }, {0.0, 0.0, -1.0}, {0.2, 0.0, 0.8}});
    m_vertices.push_back({{x    , 0.5, z    }, {0.0, 0.0, -1.0}, {0.2, 0.0, 0.8}});
    m_vertices.push_back({{x + 1, 0.5, z    }, {0.0, 0.0, -1.0}, {0.2, 0.0, 0.8}});

    AddIndces();
  }
}

void World::Generate(ID3D11Device1* device)
{
  for (uint8_t z = 0; z != Global::worldSize; z++)
    for (uint8_t x = 0; x != Global::worldSize; x++)
      if (m_map[z][x] == 1)
      {
        bool north = m_map[std::min(z + 1, 21)][x] == 1 ? false : true;
        bool south = m_map[std::max(z - 1, 0)][x] == 1 ? false : true;
        bool east = m_map[z][std::min(x + 1, 21)] == 1 ? false : true;
        bool west = m_map[z][std::max(x - 1, 0)] == 1 ? false : true;

        AddBlock(x, z, 1.0f, north, west, south, east);
      }
      else if (m_map[z][x] == 3)
      {
        AddBlock(x, z + 0.4f, 0.2f, true, false, true, false);
      }

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
  DX::ThrowIfFailed(device->CreateBuffer(&bd, &sd, &m_vertexBuffer));

  // Index buffer
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
}

void World::Init(ID3D11Device1* device)
{
  Generate(device);

  D3D11_RASTERIZER_DESC cmdesc = {};
  cmdesc.FillMode = D3D11_FILL_SOLID;
  cmdesc.CullMode = D3D11_CULL_BACK;
  cmdesc.FrontCounterClockwise = false;

  DX::ThrowIfFailed(device->CreateRasterizerState(&cmdesc, m_cullCW.GetAddressOf()));
}

void World::Draw(ID3D11DeviceContext1* context)
{
  // Bind vertex buffer and index buffer
  const UINT stride = sizeof(Global::Vertex);
  const UINT offset = 0;

  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
  context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);

  context->RSSetState(m_cullCW.Get());

  context->DrawIndexed((UINT)m_indices.size(), 0, 0);
}

bool World::IsPassable(uint8_t column, uint8_t row, bool canGoHome) // TODO: can go home is not a good name
{
  if (canGoHome && (m_map[row][column] == 0 || m_map[row][column] == 3))
    return true;
  else
    return m_map[row][column] == 0 ? true : false;
}
