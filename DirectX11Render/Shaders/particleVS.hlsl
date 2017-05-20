
struct VSIn
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR0;
	float3 instancePosition : TEXCOORD1;
    float4 instanceColor : COLOR1;
};

struct GSIn
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
};

GSIn VS_main(VSIn input)
{
	GSIn output;

	output.position = input.position + float4(input.instancePosition, 1.0f);
	output.color = input.instanceColor;
	output.tex = input.tex;

	//input.position.w = 1.0f;
	//input.position.x += input.instancePosition.x;
	//input.position.y += input.instancePosition.y;
	//input.position.z += input.instancePosition.z;
 //   
 //   input.color = input.instanceColor;

	////Make a new matrix
	//matrix billboardMatrix = worldMatrix;
	//
	////place the particle at origo
	//billboardMatrix[3][0] -= input.instancePosition.x;
	//billboardMatrix[3][1] -= input.instancePosition.y;
	//billboardMatrix[3][2] -= input.instancePosition.z;
	//
	////rotate given the inverse cameras rotation
	//billboardMatrix = mul(billboardMatrix, cameraRotationMatrix);
	//
	////place it back at the position
	//billboardMatrix[3][0] += input.instancePosition.x;
	//billboardMatrix[3][1] += input.instancePosition.y;
	//billboardMatrix[3][2] += input.instancePosition.z;

	//output.position = mul(input.position, billboardMatrix);
	//output.position = mul(output.position, viewMatrix);
	//output.position = mul(output.position, projectionMatrix);

	//output.tex = input.tex;
	//output.color = input.color;


	return output;
}