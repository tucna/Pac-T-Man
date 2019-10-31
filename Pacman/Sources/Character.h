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
    Dead // TODO
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

  uint8_t GetFrame() const;

  void SetRowInSheet(uint8_t direction);
  uint8_t GetRowInSheet() const;

  void SetMovement(Movement movement);
  Movement GetMovement() const;
  Direction GetFacingDirection() const;
  void SetFacingDirection(Direction direction);

  void Update(uint8_t coefMod, uint8_t coefAdd);

  void Init(ID3D11Device1* device, float r, float g, float b);
  void Init(ID3D11Device1* device);
  void Draw(ID3D11DeviceContext1* context);

  void AlignToMap();

  const DirectX::XMMATRIX& GetWorldMatrix() const noexcept;

  void IncreaseFrameCounter();
  uint8_t GetNumberOfFrames();
  void ResetFrameCounter();

  void RevereseMovementDirection();

  void SetColumnsAndRowsOfAssociatedSpriteSheet(uint8_t columns, uint8_t rows);
  uint8_t GetSpriteSheetColumns() const { return m_spriteSheetColumns; }
  uint8_t GetSpriteSheetRows() const { return m_spriteSheetRows; }

  void SetSpriteScaleFactor(float scale) { m_spriteScaleFactor = scale; }
  float GetSpriteScaleFactor() const { return m_spriteScaleFactor; }

private:
  void UpdateWorldMatrix();

  uint8_t m_currentFrame;
  uint8_t m_direction;

  DirectX::XMMATRIX m_worldMatrix;

  DirectX::XMFLOAT3 m_position;

  Microsoft::WRL::ComPtr<ID3D11Buffer>              m_vertexBuffer;
  Microsoft::WRL::ComPtr<ID3D11SamplerState>        m_samplerState;
  Microsoft::WRL::ComPtr<ID3D11RasterizerState>     m_cullNone;
  Microsoft::WRL::ComPtr<ID3D11Buffer>              m_instanceBuffer;

  std::vector<InstanceType>   m_instances;
  std::vector<Global::Vertex> m_vertices;

  Movement m_movement;
  Direction m_facingDirection;

  uint8_t m_frameCounter;

  uint8_t m_spriteSheetColumns;
  uint8_t m_spriteSheetRows;

  float m_spriteScaleFactor;
};

