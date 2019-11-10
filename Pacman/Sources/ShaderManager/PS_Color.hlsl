#include "Common.h"

float4 main(GS_Output input) : SV_Target
{
  return float4(input.color, 1.0f);
}