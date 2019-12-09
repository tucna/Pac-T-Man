Texture2D texture1 : register(t0); // TODO: ugly name
SamplerState samplerState : register(s0);

float4 main(float4 position : SV_POSITION, float2 texCoord : TexCoord) : SV_Target
{
  return texture1.Sample(samplerState, texCoord);
}
