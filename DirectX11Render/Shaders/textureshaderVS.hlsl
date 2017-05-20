/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

//////////////
// TYPEDEFS //
//////////////
struct VSIn
{
    float4 position : POSITION;
};

struct PSIn
{
    float4 position : SV_POSITION;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PSIn VS_main(VSIn input)
{
    PSIn output;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    return output;
}