Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

RasterizerOrderedTexture2D<float4> rovTexture : register(u1);

struct PSIn
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
};

float4 PS_main(PSIn input) : SV_TARGET
{
	float4 textureColor;
	float4 finalColor;

    //uint2 coords = uint2(input.tex[0], input.tex[1]);
	textureColor = shaderTexture.Sample(sampleType, input.tex);
    //textureColor = rovTexture[input.tex];
	finalColor = textureColor * input.color;

    //return float4(input.position.z, input.position.z, input.position.z, 1.f);
	return finalColor;
}