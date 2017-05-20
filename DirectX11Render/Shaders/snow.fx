// Snow.fx by Robin Andersson & Simon Johansson
// Rain.fx by Frank Luna (C) 2011 All Rights Reserved.

//***********************************************
// GLOBALS                                      *
//***********************************************

cbuffer cbPerFrame : register(b0)
{
	float3 gEyePosW;

	// for when the emit position/direction is varying
	float3 gEmitPosW;
	float3 gEmitDirW;

    int gParticlesPerSecond;
    float gParticleAge;

	float gGameTime;
	float gTimeStep;
	float4x4 gViewProj;
};

cbuffer cbFixed : register(b1)
{
	// Net constant acceleration used to accerlate the particles.
    float3 gAccelW = { 0.0f, -1.0f, 0.0f };
};

// Array of textures for texturing the particles.
Texture2D gTexArray;

// Random texture used to generate random numbers in shaders.
Texture2D gRandomTex;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

DepthStencilState DisableDepth
{
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;
};

DepthStencilState NoDepthWrites
{
	DepthEnable = TRUE;
	DepthWriteMask = ZERO;
};


//***********************************************
// HELPER FUNCTIONS                             *
//***********************************************
float3 RandUnitVec3(float offset)
{
	// Use game time plus offset to sample random texture.
	float u = (gGameTime + offset);

	// coordinates in [-1,1]
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;

	// project onto unit sphere
	return normalize(v);
}

float3 RandVec3(float offset)
{
	// Use game time plus offset to sample random texture.
	float u = (gGameTime + offset);

	// coordinates in [-1,1]
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;

		return v;
}
//***********************************************
// STREAM-OUT TECH                              *
//***********************************************

#define PT_EMITTER 0
#define PT_FLARE 1

struct Particle
{
	float3 InitialPosW : POSITION;
	float3 InitialVelW : VELOCITY;
	float2 SizeW       : SIZE;
	float Age          : AGE;
	float Offset       : OFFSET;
	uint Type          : TYPE;
};

Particle StreamOutVS(Particle vin)
{
	return vin;
}

// The stream-out GS is just responsible for emitting 
// new particles and destroying old particles.  The logic
// programed here will generally vary from particle system
// to particle system, as the destroy/spawn rules will be 
// different.
[maxvertexcount(93)]
void StreamOutGS(point Particle gin[1],
	inout PointStream<Particle> ptStream)
{
	gin[0].Age += gTimeStep;

	if (gin[0].Type == PT_EMITTER)
	{
        int i = 0;
		// time to emit a new particle. IMPORTANT: the i < X needs to be 1 less than total new particles or else emitter will be overwritten
        while (gin[0].Age > (1.0f / ((float)gParticlesPerSecond)) && i < 92) 
		{
            float3 vRandom = 100.0f * RandVec3((float) i / 20.0f); //- float3(50, 0, 50);
			vRandom.y = 0.0f;

			Particle p;
			p.InitialPosW = gEmitPosW.xyz + vRandom;
			p.InitialVelW = float3(0.0f, 0.0f, 0.0f);
			p.SizeW = gin[0].SizeW;
			p.Age = 0.0f;
			p.Type = PT_FLARE;
			p.Offset = RandVec3((float)i / 10.0f);

			//if (i == 91)
				//p.SizeW = float3(10.0f, 10.0f, 10.0f);

            ptStream.Append(p);

			// reduce the time to emit
            gin[0].Age -= 1.0f / ((float) gParticlesPerSecond);
            ++i;
        }

		// always keep emitters
		ptStream.Append(gin[0]);
	}
	else
	{
		// Specify conditions to keep particle; this may vary from system to system.
        if (gin[0].Age <= gParticleAge)
		{
			ptStream.Append(gin[0]); // counter may be here
		}
	}
}

GeometryShader gsStreamOut = ConstructGSWithSO(
	CompileShader(gs_5_0, StreamOutGS()),
	"POSITION.xyz; VELOCITY.xyz; SIZE.xy; AGE.x; OFFSET.x; TYPE.x");

technique11 StreamOutTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, StreamOutVS()));
		SetGeometryShader(gsStreamOut);

		// disable pixel shader for stream-out only
		SetPixelShader(NULL);

		// we must also disable the depth buffer for stream-out only
		SetDepthStencilState(DisableDepth, 0);
	}
}

//***********************************************
// DRAW TECH                                    *
//***********************************************

struct VertexOut
{
	float3 PosW  : POSITION;
	float2 SizeW : SIZE;
	uint   Type  : TYPE;
};

VertexOut DrawVS(Particle vin)
{
	VertexOut vout;

	float t = vin.Age;

	// constant acceleration equation
	float offset = vin.Offset % 1;
	float yAmplitude = .5f;
	float zAmplitude = .5f;
	float x = yAmplitude * sin(vin.Age / 1 * offset);
	x += yAmplitude * sin(vin.Age / 2 * offset);
	x += yAmplitude * sin(vin.Age / 3 * offset);
	x += 2.0f * sin(vin.Age / 6 * offset);
	float y = zAmplitude * sin(vin.Age / 0.5f * offset);
	y += zAmplitude * sin(vin.Age / 1.5f * offset);
	y += zAmplitude * sin(vin.Age / 2.5f * offset);
	y += 2.0f * sin(vin.Age / 5.5f * offset);
	float z = zAmplitude * sin(vin.Age / 1.5f * offset);
	z += zAmplitude * sin(vin.Age / 2.5f * offset);
	z += zAmplitude * sin(vin.Age / 3.5f * offset);
	z += 2.0f * sin(vin.Age / 6.5f * offset);
    vout.PosW = t * gAccelW + vin.InitialPosW + float3(x, y, z); // 0.5f*t*t*gAccelW // vin.InitialVelW

	vout.Type = vin.Type;
	vout.SizeW = vin.SizeW;
	return vout;
}

struct GeoOut
{
	float4 PosH  : SV_Position;
	float2 Tex   : TEXCOORD;
	float4 Color : COLOR;
};

// The draw GS just expands points into quads.
[maxvertexcount(4)]
void DrawGS(point VertexOut gin[1],	inout TriangleStream<GeoOut> triStream)
{
	// do not draw emitter particles.
	if (gin[0].Type != PT_EMITTER)
	{
		float sizeX = gin[0].SizeW.x;
		float sizeY = gin[0].SizeW.y;

		float halfWidth = sizeX * 0.5f;

		float3 planeNormal = gin[0].PosW - gEyePosW;
		planeNormal.y = 0.0f;
		planeNormal = normalize(planeNormal);
		
		float3 upVector = float3(0.0f, 1.0f, 0.0f);
		float3 rightVector = normalize(cross(planeNormal, upVector));
			
		rightVector = rightVector * halfWidth;
		upVector = float3(0, sizeY, 0);

		float4 v[4];
		v[0] = float4(gin[0].PosW - rightVector, 1.0f);
		v[1] = float4(gin[0].PosW + rightVector, 1.0f);
		v[2] = float4(gin[0].PosW - rightVector + upVector, 1.0f);
		v[3] = float4(gin[0].PosW + rightVector + upVector, 1.0f);
		// Slant line in acceleration direction.
		//float3 p0 = gin[0].PosW;
		//	float3 p1 = gin[0].PosW + 0.07f*gAccelW;

		GeoOut gOut[4];
		gOut[0].PosH = mul(v[0], gViewProj);
		gOut[1].PosH = mul(v[1], gViewProj);
		gOut[2].PosH = mul(v[2], gViewProj);
		gOut[3].PosH = mul(v[3], gViewProj);

		gOut[0].Tex = float2(0.0f, 1.0f);
		gOut[1].Tex = float2(1.0f, 1.0f);
		gOut[2].Tex = float2(0.0f, 0.0f);
		gOut[3].Tex = float2(1.0f, 0.0f);
		
		gOut[0].Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
		gOut[1].Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
		gOut[2].Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
		gOut[3].Color = float4(1.0f, 1.0f, 1.0f, 1.0f);

		//gOut[0].Color = clamp(float4(1.0f, 1.0f, 1.0f, 1.0f) * distance(gin[0].PosW, gEyePosW) * 0.05f, 0.0f, 1.0f);
		//gOut[1].Color = clamp(float4(1.0f, 1.0f, 1.0f, 1.0f) * distance(gin[0].PosW, gEyePosW) * 0.05f, 0.0f, 1.0f);
		//gOut[2].Color = clamp(float4(1.0f, 1.0f, 1.0f, 1.0f) * distance(gin[0].PosW, gEyePosW) * 0.05f, 0.0f, 1.0f);
		//gOut[3].Color = clamp(float4(1.0f, 1.0f, 1.0f, 1.0f) * distance(gin[0].PosW, gEyePosW) * 0.05f, 0.0f, 1.0f);
		
		for (int i = 0; i < 4; ++i)
		{
			triStream.Append(gOut[i]);
		}
	}
}

float4 DrawPS(GeoOut pin) : SV_TARGET
{
	float4 color = gTexArray.Sample(samLinear, pin.Tex);
	color *= pin.Color;
	return color;
}

technique11 DrawTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, DrawVS()));
		SetGeometryShader(CompileShader(gs_5_0, DrawGS()));
		SetPixelShader(CompileShader(ps_5_0, DrawPS()));

		SetDepthStencilState(NoDepthWrites, 0);
	}
}