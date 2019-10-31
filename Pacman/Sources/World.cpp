#include "pch.h"

#include "Global.h"
#include "World.h"

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
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 2},
    {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0}}
{
  m_worldMatrix = DirectX::XMMatrixIdentity();
}

World::~World()
{
}

void World::Generate(ID3D11Device1* device)
{
  size_t numberOfCubes = 0;

  for (int z = 0; z != Global::worldSize; z++)
    for (int x = 0; x != Global::worldSize; x++)
    {
      if (m_map[z][x] == 1)
      {
        float fX = static_cast<float>(x);
        float fZ = static_cast<float>(z);

        // 1st
        m_vertices.push_back({{fX    , 0.5, fZ    }, {0.0, 1.0, 0.0}, {0.8, 0.0, 0.0}});
        m_vertices.push_back({{fX    , 0.5, fZ + 1}, {0.0, 1.0, 0.0}, {0.8, 0.0, 0.0}});
        m_vertices.push_back({{fX + 1, 0.5, fZ + 1}, {0.0, 1.0, 0.0}, {0.8, 0.0, 0.0}});
        m_vertices.push_back({{fX + 1, 0.5, fZ    }, {0.0, 1.0, 0.0}, {0.8, 0.0, 0.0}});

        // 2st
        m_vertices.push_back({{fX    , 0.0, fZ    }, {-1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});
        m_vertices.push_back({{fX    , 0.0, fZ + 1}, {-1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});
        m_vertices.push_back({{fX    , 0.5, fZ + 1}, {-1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});
        m_vertices.push_back({{fX    , 0.5, fZ    }, {-1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});

        // 3rd
        m_vertices.push_back({{fX    , 0.0, fZ + 1}, {0.0, 0.0, 1.0}, {0.2, 0.0, 0.8}});
        m_vertices.push_back({{fX + 1, 0.0, fZ + 1}, {0.0, 0.0, 1.0}, {0.2, 0.0, 0.8}});
        m_vertices.push_back({{fX + 1, 0.5, fZ + 1}, {0.0, 0.0, 1.0}, {0.2, 0.0, 0.8}});
        m_vertices.push_back({{fX    , 0.5, fZ + 1}, {0.0, 0.0, 1.0}, {0.2, 0.0, 0.8}});

        // 4th
        m_vertices.push_back({{fX + 1, 0.0, fZ + 1}, {1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});
        m_vertices.push_back({{fX + 1, 0.0, fZ    }, {1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});
        m_vertices.push_back({{fX + 1, 0.5, fZ    }, {1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});
        m_vertices.push_back({{fX + 1, 0.5, fZ + 1}, {1.0, 0.0, 0.0}, {0.2, 0.0, 0.8}});

        // 5st
        m_vertices.push_back({{fX + 1, 0.0, fZ    }, {0.0, 0.0, -1.0}, {0.2, 0.0, 0.8}});
        m_vertices.push_back({{fX    , 0.0, fZ    }, {0.0, 0.0, -1.0}, {0.2, 0.0, 0.8}});
        m_vertices.push_back({{fX    , 0.5, fZ    }, {0.0, 0.0, -1.0}, {0.2, 0.0, 0.8}});
        m_vertices.push_back({{fX + 1, 0.5, fZ    }, {0.0, 0.0, -1.0}, {0.2, 0.0, 0.8}});

        numberOfCubes++;
      }
    }

  for (unsigned short i = 0; i != numberOfCubes; i++) // 148 - 149
  {
    constexpr uint8_t c = 19;

    m_indices.push_back((c * i) + i + 0);
    m_indices.push_back((c * i) + i + 1);
    m_indices.push_back((c * i) + i + 2);
    m_indices.push_back((c * i) + i + 0);
    m_indices.push_back((c * i) + i + 2);
    m_indices.push_back((c * i) + i + 3);

    m_indices.push_back((c * i) + i + 4);
    m_indices.push_back((c * i) + i + 5);
    m_indices.push_back((c * i) + i + 6);
    m_indices.push_back((c * i) + i + 4);
    m_indices.push_back((c * i) + i + 6);
    m_indices.push_back((c * i) + i + 7);

    m_indices.push_back((c * i) + i + 8 );
    m_indices.push_back((c * i) + i + 9 );
    m_indices.push_back((c * i) + i + 10);
    m_indices.push_back((c * i) + i + 8 );
    m_indices.push_back((c * i) + i + 10);
    m_indices.push_back((c * i) + i + 11);

    m_indices.push_back((c * i) + i + 12);
    m_indices.push_back((c * i) + i + 13);
    m_indices.push_back((c * i) + i + 14);
    m_indices.push_back((c * i) + i + 12);
    m_indices.push_back((c * i) + i + 14);
    m_indices.push_back((c * i) + i + 15);

    m_indices.push_back((c * i) + i + 16);
    m_indices.push_back((c * i) + i + 17);
    m_indices.push_back((c * i) + i + 18);
    m_indices.push_back((c * i) + i + 16);
    m_indices.push_back((c * i) + i + 18);
    m_indices.push_back((c * i) + i + 19);
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
  device->CreateBuffer(&bd, &sd, &m_vertexBuffer);

  // Index buffer
  D3D11_BUFFER_DESC ibd = {};
  ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  ibd.Usage = D3D11_USAGE_DEFAULT;
  ibd.CPUAccessFlags = 0;
  ibd.MiscFlags = 0;
  ibd.ByteWidth = UINT(m_indices.size() * sizeof(unsigned short));
  ibd.StructureByteStride = sizeof(unsigned short);

  D3D11_SUBRESOURCE_DATA isd = {};
  isd.pSysMem = m_indices.data();
  device->CreateBuffer(&ibd, &isd, &m_indexBuffer);
}

void World::Init(ID3D11Device1* device)
{
  Generate(device);

  D3D11_RASTERIZER_DESC cmdesc = {};
  cmdesc.FillMode = D3D11_FILL_SOLID;
  cmdesc.CullMode = D3D11_CULL_BACK;
  cmdesc.FrontCounterClockwise = false;

  // TUCNA check HR
  device->CreateRasterizerState(&cmdesc, m_cullCW.GetAddressOf());
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

const DirectX::XMMATRIX& World::GetWorldMatrix() const noexcept
{
  return m_worldMatrix;
}

bool World::IsPassable(uint8_t column, uint8_t row)
{
  return m_map[row][column] == 0 ? true : false;
}
