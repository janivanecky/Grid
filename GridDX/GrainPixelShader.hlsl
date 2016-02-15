
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texcoord: TEXCOORD0;
	float4 color: COLOR;
};

Texture2D spriteTexture;
SamplerState textureSampler;

float random( float2 p )
{
  const float2 r = float2(
    23.1406926327792690,  // e^pi (Gelfond's constant)
     2.6651441426902251); // 2^sqrt(2) (Gelfond–Schneider constant)
  return frac( cos( fmod( 123456789., 1e-7 + 256. * dot(p,r) ) ) );  
}

//float random(float2 uv)
//{
//	float2 noise = (frac(sin(dot(uv ,float2(12.9898,78.233))) * 1500.0));
//    return abs(noise.x + noise.y) * 0.5;
//}

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 color = spriteTexture.Sample(textureSampler, input.texcoord);
	float2 randomCoord = input.texcoord;
	float size = 0.025f;
	float n = random(randomCoord) * size - size / 2.0f;
	color.r += n;
	color.g += n;
	color.b += n;
	return color * input.color;
}