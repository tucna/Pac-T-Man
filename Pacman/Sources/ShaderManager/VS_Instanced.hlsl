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

VS_Output main(VS_Input_Instanced input)
{
  VS_Output output;

  float4 pos = float4(input.adjPos, 1.0f);

  pos = mul(pos, world);
  output.pos_w = pos;

  pos = mul(pos, view);
  output.pos_wv = pos;

  pos = mul(pos, projection);
  output.pos_wvp = pos;

  output.color = input.color;
  output.normal = input.normal;
  output.sizeFlag = input.sizeFlag;

  return output;
}
