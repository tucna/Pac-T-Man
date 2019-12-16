#include "pch.h"

#include "Camera.h"
#include "Global.h"

using namespace DirectX;

Camera::Camera() :
  m_position(0.0f, 0.0f, 0.0f),
  m_rotation(0.0f, 0.0f, 0.0f),
  m_lerpCoef(0.0f),
  m_lerpDone(false)
{
  SetPosition(Global::frontCamera.x, Global::frontCamera.y, Global::frontCamera.z);
  SetLookAtPos(10.5, 0, 10.5);

  UpdateViewMatrix();
}

Camera::~Camera()
{
}

void Camera::LerpBetweenCameraPositions(float lerpCoef)
{
  m_lerpCoef += lerpCoef;

  if (m_lerpCoef > 1.0f)
  {
    m_lerpCoef = 1.0f;
    m_lerpDone = true;
  }
  else
  {
    m_lerpDone = false;
  }

  float posX = Global::frontCamera.x + m_lerpCoef * (Global::upCamera.x - Global::frontCamera.x);
  float posY = Global::frontCamera.y + m_lerpCoef * (Global::upCamera.y - Global::frontCamera.y);
  float posZ = Global::frontCamera.z + m_lerpCoef * (Global::upCamera.z - Global::frontCamera.z);

  SetPosition(posX, posY, posZ);
  SetLookAtPos(10.5, 0, 10.5);
}

void Camera::InverseLerpBetweenCameraPositions(float lerpCoef)
{
  m_lerpCoef += lerpCoef;

  if (m_lerpCoef > 1.0f)
  {
    m_lerpCoef = 1.0f;
    m_lerpDone = true;
  }
  else
  {
    m_lerpDone = false;
  }

  float posX = Global::upCamera.x + m_lerpCoef * (Global::frontCamera.x - Global::upCamera.x);
  float posY = Global::upCamera.y + m_lerpCoef * (Global::frontCamera.y - Global::upCamera.y);
  float posZ = Global::upCamera.z + m_lerpCoef * (Global::frontCamera.z - Global::upCamera.z);

  SetPosition(posX, posY, posZ);
  SetLookAtPos(10.5, 0, 10.5);
}

void Camera::SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ)
{
  float fovRadians = (fovDegrees / 360.0f) * XM_2PI;

  XMStoreFloat4x4(&m_projectionMatrix, XMMatrixTranspose(XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ))); // TODO> here could be RH instead of LH
}

void Camera::SetOrthographicValues(float halfWidth, float halfHeight)
{
  XMStoreFloat4x4(&m_orthoMatrix, XMMatrixTranspose(XMMatrixOrthographicOffCenterLH(-halfWidth, halfWidth, -halfHeight, halfHeight, 0.0f, 100.0f)));
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
