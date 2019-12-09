#include "Common.h"

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
  VOut output;

  input.pos.y -= worldMatrix._m00;

  output.position = float4(input.pos, 1.0f);
  output.texCoord = input.color.rg; // TODO: first two used as a tex coordinates

  return output;
}
