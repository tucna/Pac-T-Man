#pragma once

#include <vector>

#include "Global.h"

class Character
{
public:
  // Preferably over enum class because of the main usage as an index
  enum Direction
  {
    Up,
    Right,
    Down,
    Left,
    _Count
  };

  enum class Movement
  {
    Up,
    Right,
    Down,
    Left,
    Stop,
    InHouse,
    Dead
  };

  struct InstanceType
  {
    DirectX::XMFLOAT3 position;
    uint8_t sizeFlag;
  };

  Character();
  ~Character();

  void AdjustPosition(float x, float y, float z);
  void UpdateFrame();
  void Init(ID3D11Device1* device, float r, float g, float b);
  void Init(ID3D11Device1* device);
  void Draw(ID3D11DeviceContext1* context);
  void AlignToMap();

  void IncreaseFrameCounter();
  void ResetFrameCounter();
  void RevereseMovementDirection();

  void SetPosition(float x, float y, float z);
  void SetMovement(Movement movement);

  void SetSpriteY(uint8_t spriteY) { m_spriteY = spriteY; }
  void SetColumnsAndRowsOfAssociatedSpriteSheet(uint8_t columns, uint8_t rows);
  void SetSpriteScaleFactor(float scale) { m_spriteScaleFactor = scale; }
  void SetSpriteXAddition(uint8_t xAddition) { m_spriteXAddition = xAddition; }
  void SetFramesPerState(uint8_t frames) { m_framesPerState = frames; }

  const DirectX::XMFLOAT4X4& GetWorldMatrix() const { return m_worldMatrix; }
  const DirectX::XMFLOAT3& GetPosition() const { return m_position; }

  Direction GetFacingDirection() const { return m_facingDirection; }
  Movement GetMovement() const { return m_movement; }

  float GetSpriteScaleFactor() const { return m_spriteScaleFactor; }

  uint8_t GetNumberOfFrames() const { return m_frameCounter; }
  uint8_t GetSpriteY() const { return m_spriteY; }
  uint8_t GetSpriteX() const { return m_currentFrame; }
  uint8_t GetSpriteSheetColumns() const { return m_spriteSheetColumns; }
  uint8_t GetSpriteSheetRows() const { return m_spriteSheetRows; }

private:
  void UpdateWorldMatrix();

  Microsoft::WRL::ComPtr<ID3D11Buffer>          m_vertexBuffer;
  Microsoft::WRL::ComPtr<ID3D11SamplerState>    m_samplerState;
  Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_cullNone;
  Microsoft::WRL::ComPtr<ID3D11Buffer>          m_instanceBuffer;

  DirectX::XMFLOAT4X4 m_worldMatrix;
  DirectX::XMFLOAT3 m_position;

  std::vector<InstanceType> m_instances;
  std::vector<Global::Vertex> m_vertices;

  Movement m_movement;
  Direction m_facingDirection;

  uint8_t m_currentFrame;
  uint8_t m_spriteY;
  uint8_t m_frameCounter;
  uint8_t m_spriteSheetColumns;
  uint8_t m_spriteSheetRows;
  uint8_t m_spriteXAddition;
  uint8_t m_framesPerState;

  float m_spriteScaleFactor;
};

