#include "Common.h"

cbuffer cameraPerFrame : register(b0)
{
  matrix view;
  matrix projection;
};

// todo - inconsistent naming
cbuffer cbParameters : register(b1)
{
  float4x4 worldMatrix;
};

struct VOut
{
  float4 position : SV_POSITION;
  float2 texCoord : TexCoord;
};

VOut main(VS_Input_Indexed input)
{
  // TODO constant should come from outside
  float2 rcpDim;
  rcpDim.x = 1.0f / 800.0f;
  rcpDim.y = 1.0f / 600.0f;

  VOut output;

  // TODO this is stupid
  //input.pos.y -= worldMatrix._m00;

  /* THIS was working
  input.pos.xy = input.pos.xy * rcpDim * 2;
  input.pos.xy -= 1;
  input.pos.y *= -1;
  input.pos.z = 0;
  */

  output.position = float4(input.pos, 1.0f);

  output.position = mul(output.position, worldMatrix);
  output.position = mul(output.position, view);
  output.position = mul(output.position, projection);

  output.texCoord = input.texCoord;

  return output;
}
