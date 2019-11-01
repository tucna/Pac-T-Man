#include "pch.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera() :
  m_position(0.0f, 0.0f, 0.0f),
  m_rotation(0.0f, 0.0f, 0.0f)
{
  UpdateViewMatrix();
}

Camera::~Camera()
{
}

void Camera::SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ)
{
  float fovRadians = (fovDegrees / 360.0f) * XM_2PI;

  XMStoreFloat4x4(&m_projectionMatrix, XMMatrixTranspose(XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ)));
}

void Camera::SetPosition(float x, float y, float z)
{
  m_position.x = x;
  m_position.y = y;
  m_position.z = z;

  UpdateViewMatrix();
}

void Camera::SetRotation(float x, float y, float z)
{
  m_rotation.x = x;
  m_rotation.y = y;
  m_rotation.z = z;

  UpdateViewMatrix();
}

void Camera::SetLookAtPos(float x, float y, float z)
{
  XMFLOAT3 lookAtPos = {};

  lookAtPos.x = m_position.x - x;
  lookAtPos.y = m_position.y - y;
  lookAtPos.z = m_position.z - z;

  float pitch = 0.0f;
  if (lookAtPos.y != 0.0f)
  {
    const float distance = sqrt(lookAtPos.x * lookAtPos.x + lookAtPos.z * lookAtPos.z);

    pitch = atan(lookAtPos.y / distance);
  }

  float yaw = 0.0f;
  if (lookAtPos.x != 0.0f)
    yaw = atan(lookAtPos.x / lookAtPos.z);

  if (lookAtPos.z > 0)
    yaw += XM_PI;

  SetRotation(pitch, yaw, 0.0f);
}

void Camera::UpdateViewMatrix()
{
  XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
  XMVECTOR camTarget = XMVector3TransformCoord(forwardVector, camRotationMatrix);
  XMVECTOR positionVector = XMLoadFloat3(&m_position);
  XMVECTOR upDir = XMVector3TransformCoord(upVector, camRotationMatrix);

  camTarget += positionVector;

  XMStoreFloat4x4(&m_viewMatrix, XMMatrixTranspose(XMMatrixLookAtLH(positionVector, camTarget, upDir)));
}
