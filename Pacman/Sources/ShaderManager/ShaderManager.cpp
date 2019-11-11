#include "pch.h"

#include "ShaderManager.h"

// Vertex shaders
#include "VS_Indexed.h"
#include "VS_Instanced.h"

// Geometry shaders
#include "GS_General.h"

// Pixel shaders
#include "PS_General.h"

ShaderManager::ShaderManager(ID3D11Device1* device, ID3D11DeviceContext1* context) :
  m_device(device),
  m_context(context)
{
  AddPredefinedOnes();
}

ShaderManager::~ShaderManager()
{
}

void ShaderManager::SetPixelShader(PixelShader pixelShader)
{
  m_pixelShaders[static_cast<uint8_t>(pixelShader)]->ActivateShader(m_context);
}

void ShaderManager::BindConstantBuffersToVertexShader(VertexShader vertexShader, ID3D11Buffer** constantBuffers, uint8_t numberOfConstantBuffers)
{
  m_vertexShaders[static_cast<uint8_t>(vertexShader)]->BindConstantBuffers(m_context, constantBuffers, numberOfConstantBuffers);
}

void ShaderManager::BindConstantBuffersToGeometryShader(GeometryShader geometryShader, ID3D11Buffer** constantBuffers, uint8_t numberOfConstantBuffers)
{
  m_geometryShaders[static_cast<uint8_t>(geometryShader)]->BindConstantBuffers(m_context, constantBuffers, numberOfConstantBuffers);
}

void ShaderManager::UpdateConstantBuffer(ID3D11Buffer* constantBuffer, void* data, size_t dataLength)
{
  D3D11_MAPPED_SUBRESOURCE resource;
  m_context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
  memcpy(resource.pData, data, dataLength);
  m_context->Unmap(constantBuffer, 0);
}

void ShaderManager::SetGeometryShader(GeometryShader geometryShader)
{
  m_geometryShaders[static_cast<uint8_t>(geometryShader)]->ActivateShader(m_context);
}

void ShaderManager::SetVertexShader(VertexShader vertexShader)
{
  m_vertexShaders[static_cast<uint8_t>(vertexShader)]->ActivateShader(m_context);
}

void ShaderManager::ClearShadersFromThePipeline()
{
  m_context->VSSetShader(NULL, NULL, 0);
  m_context->GSSetShader(NULL, NULL, 0);
  m_context->PSSetShader(NULL, NULL, 0);
}

void ShaderManager::AddPredefinedOnes()
{
  // Pixel shaders
  m_pixelShaders[static_cast<uint8_t>(PixelShader::Color)] = std::make_unique<PS_General>(L"../bin/PS_Color.cso", m_device);
  m_pixelShaders[static_cast<uint8_t>(PixelShader::Texture)] = std::make_unique<PS_General>(L"../bin/PS_Texture.cso", m_device);
  m_pixelShaders[static_cast<uint8_t>(PixelShader::Phong)] = std::make_unique<PS_General>(L"../bin/PS_Phong.cso", m_device);

  // Geometry shaders
  m_geometryShaders[static_cast<uint8_t>(GeometryShader::Billboard)] = std::make_unique<GS_General>(L"../bin/GS_Billboard.cso", m_device);

  // Vertex shaders
  m_vertexShaders[static_cast<uint8_t>(VertexShader::Indexed)] = std::make_unique<VS_Indexed>(L"../bin/VS_Indexed.cso", m_device);
  m_vertexShaders[static_cast<uint8_t>(VertexShader::Instanced)] = std::make_unique<VS_Instanced>(L"../bin/VS_Instanced.cso", m_device);
}
