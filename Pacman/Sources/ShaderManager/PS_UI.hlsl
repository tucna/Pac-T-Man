Texture2D textureUI : register(t0);
SamplerState samplerState : register(s0);

float4 main(float4 position : SV_POSITION, float2 texCoord : TexCoord) : SV_Target
{
  return textureUI.Sample(samplerState, texCoord);
}
