#pragma once

#include <DirectXMath.h>

class Camera
{
public:
  Camera();
  ~Camera();

  void SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ);

  const DirectX::XMMATRIX& GetViewMatrix() const;
  const DirectX::XMMATRIX& GetProjectionMatrix() const;

  void SetPosition(float x, float y, float z);
  void SetRotation(float x, float y, float z);

  void SetLookAtPos(DirectX::XMFLOAT3 lookAtPos);

private:
  void UpdateViewMatrix();

  DirectX::XMVECTOR posVector;
  DirectX::XMVECTOR rotVector;
  DirectX::XMFLOAT3 pos;
  DirectX::XMFLOAT3 rot;
  DirectX::XMMATRIX viewMatrix;
  DirectX::XMMATRIX projectionMatrix;

  const DirectX::XMVECTOR DEFAULT_FORWARD_VECTOR = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
  const DirectX::XMVECTOR DEFAULT_UP_VECTOR = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
};

