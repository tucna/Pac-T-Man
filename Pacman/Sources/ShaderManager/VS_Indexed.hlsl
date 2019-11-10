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

VS_Output main(VS_Input_Indexed input)
{
  VS_Output output;

  float4 pos = float4(input.pos, 1.0f);

  pos = mul(pos, world);
  output.pos_w = pos;

  pos = mul(pos, view);
  output.pos_wv = pos;

  pos = mul(pos, projection);
  output.pos_wvp = pos;

  float4 normal = float4(input.normal, 0.0f);

  normal = mul(normal, world);
  output.normal = normal.xyz;

  output.color = input.color;
  output.sizeFlag = 0;

  return output;
}
