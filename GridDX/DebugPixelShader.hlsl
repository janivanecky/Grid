struct PixelShaderInput
{
	float4 pos : SV_POSITION;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	return float4(1,1,1,1);
}
