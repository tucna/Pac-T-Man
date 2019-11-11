#pragma once

#include <array>
#include <vector>

#include "IShader.h"

class ShaderManager
{
public:
  enum class VertexShader
  {
    Indexed,
    Instanced,
    _Count
  };

  enum class GeometryShader
  {
    Billboard,
    _Count
  };

  enum class PixelShader
  {
    Color,
    Texture,
    Phong,
    _Count
  };

  ShaderManager(ID3D11Device1* device, ID3D11DeviceContext1* context);
  ~ShaderManager();

  void SetVertexShader(VertexShader vertexShader);
  void SetGeometryShader(GeometryShader geometryShader);
  void SetPixelShader(PixelShader pixelShader);

  void BindConstantBuffersToVertexShader(VertexShader vertexShader, ID3D11Buffer** constantBuffers, uint8_t numberOfConstantBuffers);
  void BindConstantBuffersToGeometryShader(GeometryShader geometryShader, ID3D11Buffer** constantBuffers, uint8_t numberOfConstantBuffers);

  void UpdateConstantBuffer(ID3D11Buffer* constantBuffer, void* data, size_t dataLength);

  void ClearShadersFromThePipeline();

private:
  void AddPredefinedOnes();

  std::array<std::unique_ptr<IShader<ID3D11PixelShader>>, static_cast<uint8_t>(PixelShader::_Count)>        m_pixelShaders;
  std::array<std::unique_ptr<IShader<ID3D11GeometryShader>>, static_cast<uint8_t>(GeometryShader::_Count)>  m_geometryShaders;
  std::array<std::unique_ptr<IShader<ID3D11VertexShader>>, static_cast<uint8_t>(VertexShader::_Count)>      m_vertexShaders;

  ID3D11Device1*        m_device;
  ID3D11DeviceContext1* m_context;
};

