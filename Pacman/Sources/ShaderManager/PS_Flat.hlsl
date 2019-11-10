#include "Common.h"

float4 main(VS_Output input) : SV_Target
{
  float3 dir = float3(0.0f, -1.0f, 0.0f);
  float3 ambient = float3(0.2f, 0.2f, 0.2f);

  float3 dirDotN = dot(normalize(-dir), normalize(input.normal));

  return float4(saturate(ambient + (input.color.rgb * dirDotN)), 1.0f);
}