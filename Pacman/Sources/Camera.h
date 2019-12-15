#pragma once

#include <DirectXMath.h>

class Camera
{
public:
  explicit Camera();
  ~Camera();

  void LerpBetweenCameraPositions(float lerpCoef);

  void SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ);
  void SetOrthographicValues(float halfWidth, float halfHeight);
  void SetPosition(float x, float y, float z);
  void SetRotation(float x, float y, float z);
  void SetLookAtPos(float x, float y, float z);

  const DirectX::XMFLOAT4X4& GetViewMatrix() const { return m_viewMatrix; }
  const DirectX::XMFLOAT4X4& GetProjectionMatrix() const { return m_projectionMatrix; }
  const DirectX::XMFLOAT4X4& GetOrthographicMatrix() const { return m_orthoMatrix; }

  bool IsCameraLerpDone() { return m_lerpDone; }

private:
  void UpdateViewMatrix();

  const DirectX::XMVECTOR forwardVector = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
  const DirectX::XMVECTOR upVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

  DirectX::XMFLOAT3 m_position;
  DirectX::XMFLOAT3 m_rotation;

  DirectX::XMFLOAT4X4 m_viewMatrix;
  DirectX::XMFLOAT4X4 m_projectionMatrix;
  DirectX::XMFLOAT4X4 m_orthoMatrix;

  float m_lerpCoef;

  bool m_lerpDone;
};
