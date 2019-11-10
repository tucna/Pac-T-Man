#include "Common.h"

float4 main(VS_Output input) : SV_Target
{
  float3 n = normalize(input.normal);

  return float4(n, 1.0f);
}