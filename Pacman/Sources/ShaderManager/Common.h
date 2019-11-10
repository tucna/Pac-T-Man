#ifndef _COMMON_H_
#define _COMMON_H_

struct VS_Input_Indexed
{
  float3 pos    : Position;
  float3 normal : Normal;
  float3 color  : Color;
};

struct VS_Input_Instanced
{
  float3 pos      : Position;
  float3 normal   : Normal;
  float3 color    : Color;
  float3 adjPos   : AdjPos;
  uint   sizeFlag : SizeFlag;
};

struct VS_Output
{
  float4 pos_wvp  : SV_Position;
  float4 pos_wv   : Position0;
  float4 pos_w    : Position1;
  float3 normal   : Normal;
  float3 color    : Color;
  uint   sizeFlag : SizeFlag;
};

struct GS_Output
{
  float4 pos      : SV_Position;
  float3 normal   : Normal;
  float3 color    : Color;
  float2 texCoord : TexCoord;
};

#endif // _COMMON_H_
