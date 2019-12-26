#include "Common.h"

cbuffer cameraPerFrame : register(b0)
{
  matrix view;
  matrix projection;
};

cbuffer cameraPerObject : register(b1)
{
  matrix world;
};

struct VOut
{
  float4 position : SV_POSITION;
  float2 texCoord : TexCoord;
};

VOut main(VS_Input_Indexed input)
{
  VOut output;

  output.position = float4(input.pos, 1.0f);

  output.position = mul(output.position, world);
  output.position = mul(output.position, view);
  output.position = mul(output.position, projection);

  output.texCoord = input.texCoord;

  return output;
}
