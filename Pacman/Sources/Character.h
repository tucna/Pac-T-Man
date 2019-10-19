#pragma once

#include <vector>

#include "Global.h"

class Character
{
public:
  enum class Movement
  {
    Up,
    Right,
    Down,
    Left,
    Stop
  };

  struct InstanceType
  {
    DirectX::XMFLOAT3 newPosition;
  };

  Character();
  ~Character();

  void SetPosition(float x, float y, float z);
  void SetFrame(uint8_t frameID);

  void AdjustPosition(float x, float y, float z);

  DirectX::XMFLOAT3 GetPosition();

  uint8_t GetFrame();

  void SetRowInSheet(uint8_t direction);
  uint8_t GetRowInSheet();

  void SetMovement(Movement movement);
  Movement GetMovement();

  void Update(uint8_t coefMod, uint8_t coefAdd);

  void Init(ID3D11Device1* device, const wchar_t* fileName);
  void Draw(ID3D11DeviceContext1* context);

  void AlignToMap();

  DirectX::XMMATRIX GetWorldMatrix() const noexcept;

private:
  uint8_t m_currentFrame;
  uint8_t m_direction;

  DirectX::XMFLOAT3 m_position;

  Microsoft::WRL::ComPtr<ID3D11Buffer>              m_vertexBuffer;
  Microsoft::WRL::ComPtr<ID3D11Resource>            m_resource;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  m_shaderResourceView;
  Microsoft::WRL::ComPtr<ID3D11SamplerState>        m_samplerState;
  Microsoft::WRL::ComPtr<ID3D11RasterizerState>     m_cullNone;
  Microsoft::WRL::ComPtr<ID3D11Buffer>              m_instanceBuffer;

  std::vector<InstanceType>   m_instances;
  std::vector<Global::Vertex> m_vertices;

  Movement m_movement;
};

