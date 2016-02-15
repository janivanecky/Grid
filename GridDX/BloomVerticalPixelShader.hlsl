struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texcoord: TEXCOORD0;
	float4 color: COLOR;
};

Texture2D spriteTexture;
SamplerState textureSampler;

static const float BlurWeights[11] =
{
   0.014644 / 0.98687,
   0.036021 / 0.98687,
   0.072537 / 0.98687,
   0.119593 / 0.98687,
   0.161434 / 0.98687,
   0.178412 / 0.98687,
   0.161434 / 0.98687,
   0.119593 / 0.98687,
   0.072537 / 0.98687,
   0.036021 / 0.98687,
   0.014644 / 0.98687,
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 result = 0;
	float2 step = float2(1.0f / 1280.0f, 1.0f / 720.0f) * 1.5f;
	for (int y = -5; y <= 5; y++)
	{
		float4 value = spriteTexture.Sample(textureSampler, input.texcoord + float2(0, y) * step);
		result.rgb += value.rgb * BlurWeights[y + 5];
		result.a += value.a;	
	}
	return float4(result.rgb, result.a);//spriteTexture.Sample(textureSampler, input.texcoord) * input.color;
}