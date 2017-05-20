/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D reflectionTexture : register(t2);
Texture2D depthMapTexture : register(t3);
SamplerState SampleType : register(s0);

cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightDirection;
	float specularPower;
	float4 specularColor;
};

//////////////
// TYPEDEFS //
//////////////
struct PSIn
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 viewDirection : TEXCOORD1;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float4 reflectionPosition : TEXCOORD2;
	float4 lightViewPosition : TEXCOORD3;
	float3 lightPos : TEXCOORD4;
};

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 PS_main(PSIn input) : SV_TARGET
{
	float4 reflectionColor;
	float2 reflectTexCoord;
	float4 specular;
	float4 color;
    float4 textureColor; 
	float4 bumpMap;
	float3 bumpNormal;
	float2 projectTexCoord;
	float bias = 0.001f;

	projectTexCoord.x =  input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;

	reflectTexCoord.x = input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
    reflectTexCoord.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;

    reflectionColor = reflectionTexture.Sample(SampleType, reflectTexCoord);
    
	bumpMap = normalTexture.Sample(SampleType, input.tex);
	bumpMap = (bumpMap * 2.f) - 1.f;

	bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normal);
	bumpNormal = normalize(bumpNormal);

	color = ambientColor;

	specular = float4(0.f, 0.f, 0.f, 0.f);

	if((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
	{
		float depthValue = depthMapTexture.Sample(SampleType, projectTexCoord).r;
		float lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;
		lightDepthValue = lightDepthValue - bias;

		if(lightDepthValue < depthValue)
		{
			float lightIntensity = saturate(dot(bumpNormal, -lightDirection));
			if (lightIntensity > 0.f)
			{
				color += saturate(diffuseColor * lightIntensity);
		
				float3 reflection = normalize(2 * lightIntensity * bumpNormal + input.lightPos);
				specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower) * specularColor;
			}
		}
	}

	textureColor = shaderTexture.Sample(SampleType, input.tex);

	//float4 reflectionMultiplier = lerp(textureColor, reflectionColor, 0.25f);

	color = color * textureColor;// * reflectionMultiplier;

	color = saturate(color + specular);

	//return specular;
	//return bumpMap;
	//return textureColor;
	//return float4(input.normal, 1);
	//return float4(bumpNormal , 1);
    return color;
}