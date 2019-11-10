#include "Common.h"

Texture2D     dotTexture    : register(t0);
SamplerState  samplerState  : register(s0);

float4 main(GS_Output input) : SV_Target
{
  return dotTexture.Sample(samplerState, input.texCoord);
}