cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer CameraBuffer : register(b1)
{
	float3 cameraPosition;
	float padding;
};

struct GSOutput
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
};

struct GSIn
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
};

[maxvertexcount(4)]
void GS_main(point GSIn input[1], inout TriangleStream< GSOutput > output)
{


	float sizeX = 0.1f;//gin[0].SizeW.x;
	float sizeY = 0.1f;// gin[0].SizeW.y;

	float halfWidth = sizeX * 0.5f;

	float3 planeNormal = input[0].position.xyz - cameraPosition;
	planeNormal.y = 0.0f;
	planeNormal = normalize(planeNormal);

	float3 upVector = float3(0.0f, 1.0f, 0.0f);
	float3 rightVector = normalize(cross(planeNormal, upVector));

	rightVector = rightVector * halfWidth;
	upVector = float3(0, sizeY, 0);

	float4 v[4];
	v[0] = float4(input[0].position.xyz - rightVector, 1.0f);
	v[1] = float4(input[0].position.xyz + rightVector, 1.0f);
	v[2] = float4(input[0].position.xyz - rightVector + upVector, 1.0f);
	v[3] = float4(input[0].position.xyz + rightVector + upVector, 1.0f);
	// Slant line in acceleration direction.
	//float3 p0 = gin[0].PosW;
	//	float3 p1 = gin[0].PosW + 0.07f*gAccelW;

	GSOutput gOut[4];
	gOut[0].position = mul(v[0], worldMatrix);
	gOut[0].position = mul(gOut[0].position, viewMatrix);
	gOut[0].position = mul(gOut[0].position, projectionMatrix);
	gOut[1].position = mul(v[1], worldMatrix);
	gOut[1].position = mul(gOut[1].position, viewMatrix);
	gOut[1].position = mul(gOut[1].position, projectionMatrix);
	gOut[2].position = mul(v[2], worldMatrix);
	gOut[2].position = mul(gOut[2].position, viewMatrix);
	gOut[2].position = mul(gOut[2].position, projectionMatrix);
	gOut[3].position = mul(v[3], worldMatrix);
	gOut[3].position = mul(gOut[3].position, viewMatrix);
	gOut[3].position = mul(gOut[3].position, projectionMatrix);

	gOut[0].tex = float2(0.0f, 1.0f);
	gOut[1].tex = float2(1.0f, 1.0f);
	gOut[2].tex = float2(0.0f, 0.0f);
	gOut[3].tex = float2(1.0f, 0.0f);

	gOut[0].color = input[0].color;
	gOut[1].color = input[0].color;
	gOut[2].color = input[0].color;
	gOut[3].color = input[0].color;

	//gOut[0].color = clamp(input[0].color * distance(input[0].position.xyz, cameraPosition) * 0.05f, 0.0f, 1.0f);
	//gOut[1].color = clamp(input[0].color * distance(input[0].position.xyz, cameraPosition) * 0.05f, 0.0f, 1.0f);
	//gOut[2].color = clamp(input[0].color * distance(input[0].position.xyz, cameraPosition) * 0.05f, 0.0f, 1.0f);
	//gOut[3].color = clamp(input[0].color * distance(input[0].position.xyz, cameraPosition) * 0.05f, 0.0f, 1.0f);

	for (int i = 0; i < 4; ++i)
	{
		output.Append(gOut[i]);
	}
}