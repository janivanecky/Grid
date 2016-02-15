cbuffer ScreenBuffer: register(b0)
{
	matrix projection;
	matrix view;
}

struct VertexShaderInput
{
	float2 pos : POSITION;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 0.0f, 1.0f);
	pos = mul(pos, view);
	pos = mul(pos, projection);

	output.pos = pos;
	return output;
}
