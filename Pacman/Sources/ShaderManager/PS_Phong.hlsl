#include "Common.h"

static const float3 materialColor = { 0.7f, 0.7f, 0.9f };
static const float3 ambient = { 0.15f, 0.15f, 0.15f };
static const float3 diffuseColor = { 1.0f, 1.0f, 1.0f };;
static const float diffuseIntensity = 1.0f;
static const float attConst = 1.0f;
static const float attLin = 0.045f;
static const float attQuad = 0.0075f;

cbuffer lightParams : register(b0)
{
  float4 lightPos;
};

float4 main(VS_Output input) : SV_Target
{
  // fragment to light vector data
  const float3 vToL = lightPos.xyz - input.pos_w.xyz;
  const float distToL = length(vToL);
  const float3 dirToL = vToL / distToL;

  // attenuation
  const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));

  // diffuse intensity
  const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f,dot(dirToL, input.normal));

  // final color
  return float4(saturate((diffuse + ambient) * materialColor),1.0f);
}
