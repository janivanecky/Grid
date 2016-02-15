cbuffer PerSceneBuffer: register(b0)
{
	matrix projection;
	matrix view;
};

cbuffer PerModelBuffer: register(b1)
{
	matrix model;
	float4 color;
}

struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 texcoord: TEXCOORD0;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texcoord: TEXCOORD0;
	float4 color: COLOR;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);

	output.pos = pos;
	output.texcoord = input.texcoord;
	output.color = color;

	return output;
}
