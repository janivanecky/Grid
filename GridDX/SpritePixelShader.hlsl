struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texcoord: TEXCOORD0;
	float4 color: COLOR;
};

Texture2D spriteTexture;
SamplerState textureSampler;

float4 main(PixelShaderInput input) : SV_TARGET
{
	return spriteTexture.Sample(textureSampler, input.texcoord) * input.color;
}
