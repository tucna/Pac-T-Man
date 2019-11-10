#include "Common.h"

cbuffer ProjectionMatrix : register(b0)
{
  matrix projection;
}

cbuffer FrameConstantBuffer : register(b1)
{
  uint spriteX;
  uint spriteY;
  uint spriteSheetColumns;
  uint spriteSheetRows;

  float4 billboardSize_0_0_0;
};

[maxvertexcount(4)]
void main(point VS_Output input[1], inout TriangleStream<GS_Output> outputStream)
{
  GS_Output a;
  a.color = input[0].color;
  a.normal = input[0].normal;
  a.pos = input[0].pos_wv;

  GS_Output b = a;
  GS_Output c = a;
  GS_Output d = a;

  float billboardSize = 0;

  if (input[0].sizeFlag == 1)
    billboardSize = billboardSize_0_0_0.x;
  else if (input[0].sizeFlag == 2)
    billboardSize = billboardSize_0_0_0.y;

  a.pos.xy += float2(-0.5, -0.5) * billboardSize;
  b.pos.xy += float2(0.5, -0.5) * billboardSize;
  c.pos.xy += float2(-0.5, 0.5) * billboardSize;
  d.pos.xy += float2(0.5, 0.5) * billboardSize;

  a.pos = mul(a.pos, projection);
  b.pos = mul(b.pos, projection);
  c.pos = mul(c.pos, projection);
  d.pos = mul(d.pos, projection);

  float xOffset = 1.0f / spriteSheetColumns;
  float yOffset = 1.0f / spriteSheetRows;

  a.texCoord = float2(spriteX * xOffset, (spriteY + 1) * yOffset);
  b.texCoord = float2((spriteX + 1) * xOffset, (spriteY + 1) * yOffset);
  c.texCoord = float2(spriteX * xOffset, spriteY * yOffset);
  d.texCoord = float2((spriteX + 1) * xOffset, spriteY * yOffset);

  outputStream.Append(a);
  outputStream.Append(b);
  outputStream.Append(c);
  outputStream.Append(d);
}
