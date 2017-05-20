/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
};

cbuffer CameraBuffer : register(b1)
{
	float3 cameraPosition;
	float padding;
};

cbuffer ObjectBufferType : register(b2)
{
	float textureScale;
	float3 pad;
};

cbuffer ReflectionBuffer : register(b3)
{
	matrix reflectionMatrix;
};

cbuffer LightBuffer : register(b4)
{
	float3 lightPosition;
	float paddddd;
};

//////////////
// TYPEDEFS //
//////////////
struct VSIn
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

struct PSIn
{
    float4 position	: SV_POSITION;
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
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PSIn VS_main(VSIn input)
{
    PSIn output;
	float4 worldPosition;
	matrix reflectProjectWorld;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	output.lightViewPosition = mul(input.position, worldMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightViewMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightProjectionMatrix);

    output.tex = input.tex * textureScale; // * 3.f;

	output.normal = mul(float4(input.normal,0), worldMatrix).xyz;
	output.normal = normalize(output.normal);
	output.tangent = mul(float4(input.tangent, 0), worldMatrix).xyz;
	output.tangent = normalize(output.tangent);
	output.binormal = mul(float4(input.binormal, 0), worldMatrix).xyz;
	output.binormal = normalize(output.binormal);
    
	worldPosition = mul(input.position, worldMatrix);

	output.viewDirection = cameraPosition.xyz - worldPosition.xyz;
	output.viewDirection = normalize(output.viewDirection);

    reflectProjectWorld = mul(reflectionMatrix, projectionMatrix);
    reflectProjectWorld = mul(worldMatrix, reflectProjectWorld);

    output.reflectionPosition = mul(input.position, reflectProjectWorld);

	output.lightPos = lightPosition.xyz - worldPosition.xyz;
    output.lightPos = normalize(output.lightPos);

    return output;
}