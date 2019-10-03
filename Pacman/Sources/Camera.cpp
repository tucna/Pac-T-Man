#include "pch.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
  pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
  posVector = XMLoadFloat3(&pos);
  rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
  rotVector = XMLoadFloat3(&rot);
  UpdateViewMatrix();
}

Camera::~Camera()
{
}

const XMMATRIX& Camera::GetViewMatrix() const
{
  return viewMatrix;
}

const XMMATRIX& Camera::GetProjectionMatrix() const
{
  return projectionMatrix;
}

void Camera::SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ)
{
  float fovRadians = (fovDegrees / 360.0f) * XM_2PI;
  projectionMatrix = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
}

void Camera::SetPosition(float x, float y, float z)
{
  pos = XMFLOAT3(x, y, z);
  posVector = XMLoadFloat3(&pos);
  UpdateViewMatrix();
}

void Camera::SetRotation(float x, float y, float z)
{
  rot = XMFLOAT3(x, y, z);
  rotVector = XMLoadFloat3(&rot);
  UpdateViewMatrix();
}

void Camera::SetLookAtPos(XMFLOAT3 lookAtPos)
{
  //Verify that look at pos is not the same as cam pos. They cannot be the same as that wouldn't make sense and would result in undefined behavior.
  if (lookAtPos.x == this->pos.x && lookAtPos.y == this->pos.y && lookAtPos.z == this->pos.z)
    return;

  lookAtPos.x = this->pos.x - lookAtPos.x;
  lookAtPos.y = this->pos.y - lookAtPos.y;
  lookAtPos.z = this->pos.z - lookAtPos.z;

  float pitch = 0.0f;
  if (lookAtPos.y != 0.0f)
  {
    const float distance = sqrt(lookAtPos.x * lookAtPos.x + lookAtPos.z * lookAtPos.z);
    pitch = atan(lookAtPos.y / distance);
  }

  float yaw = 0.0f;
  if (lookAtPos.x != 0.0f)
  {
    yaw = atan(lookAtPos.x / lookAtPos.z);
  }
  if (lookAtPos.z > 0)
    yaw += XM_PI;

  this->SetRotation(pitch, yaw, 0.0f);
}

void Camera::UpdateViewMatrix() //Updates view matrix and also updates the movement vectors
{
  //Calculate camera rotation matrix
  XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(this->rot.x, this->rot.y, this->rot.z);
  //Calculate unit vector of cam target based off camera forward value transformed by cam rotation matrix
  XMVECTOR camTarget = XMVector3TransformCoord(this->DEFAULT_FORWARD_VECTOR, camRotationMatrix);
  //Adjust cam target to be offset by the camera's current position
  camTarget += this->posVector;
  //Calculate up direction based on current rotation
  XMVECTOR upDir = XMVector3TransformCoord(this->DEFAULT_UP_VECTOR, camRotationMatrix);
  //Rebuild view matrix
  this->viewMatrix = XMMatrixLookAtLH(this->posVector, camTarget, upDir);
}
