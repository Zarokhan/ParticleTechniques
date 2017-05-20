#ifndef _SCENE_H_
#define _SCENE_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "Graphics/Utilities/DebugWindow.h"
#include "Graphics/Direct3DManager.h"
#include "Graphics/Camera.h"
#include "Graphics/WorldObjects/Cube.h"
#include "Graphics/WorldObjects/Mesh.h"
#include "Graphics/Shaders/SuperShader.h"
#include "Graphics/Shaders/TextureShader.h"
#include "Graphics/Light.h"
#include "Graphics/RenderTexture.h"
#include "Graphics\ParticleSystem\ParticleSystem.h"
#include "Graphics\GPUParticleSystem\ParticleSys.h"
#include "Graphics\Shaders\particleshader.h"
#include "InputHandler.h"
#include "DirectX.h"
#include "Benchmark\Benchmark.h"
#include <vector>


/////////////
// GLOBALS //
/////////////

#define WINDOW_SCREEN_WIDTH 1920
#define  WINDOW_SCREEN_HEIGHT 1080
#define SHOW_MOUSE_CURSOR 1
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = false;
const float SCREEN_DEPTH = 500.0f;
const float SCREEN_Z_NEAR = 0.1f;
const float SHADOWMAP_DEPTH = 500.f;
const float SHADOWMAP_Z_NEAR = 10.f;
const int SHADOWMAP_WIDTH = 1024*2;
const int SHADOWMAP_HEIGHT = 1024*2;

//////////////////
// BENCHMARKING //
//////////////////
const float m_warmupTime = 30.0f;
const float m_sampleTime = 30.0f;
const int m_incrementParticles = 100000;
const int m_rounds = 5;

////////////////////////////////////////////////////////////////////////////////
// Class name: Scene
////////////////////////////////////////////////////////////////////////////////
class Scene
{
private:
	struct ModelEntry
	{
		ModelEntry(Model* entry, bool active = true) : m_model(entry), m_active(active) { }

		~ModelEntry()
		{
			delete m_model;
		}

		Model* m_model;
		bool m_active;
	};
	enum ParticleSystemType
	{
		None,
		Instancing,
		StreamOut,
		ComputeShader
	};
	enum BenchmarkPhase {
		Nonet,	// None // Not been activated yet
		Warmup,	// Lets PS run for 30 sec before sampling data
		Sample,
		Done
	};
public:
	Scene();
	Scene(const Scene&);
	~Scene();

	bool Initialize(int, int, HWND, InputHandler*);

	bool Input(const float& dt);
	bool Frame(const float& dt);

	void AddModel(Model* model, bool active = true)
	{
		m_models.push_back(new ModelEntry(model, active));
	}
	bool SwitchParticleSystem(ParticleSystemType particleType);

private:
	bool Render();
	bool RenderScene();
	bool RenderToTexture();
	void UpdateBenchmarkCount(float dt);

private:	// Reference pointer, not an object created by this class
	InputHandler* m_Input;

private:
	HWND m_hwnd; // Win
	Direct3DManager* m_Direct3D;
	Camera* m_Camera;
	Light* m_Light;
	SuperShader* m_SuperShader;
	TextureShader* m_textureShader;
	RenderTexture* m_renderTexture;
	DebugWindow* m_debugWindow;
	Benchmark* m_benchStreamout;
	Benchmark* m_benchInstancing;

	ParticleSystemType m_currentParticleSystem;

	Cube* m_Floor;
	ParticleShader* m_particleshader;
	ParticleSystem* m_particlesystem;
	GPUP::ParticleSys* m_gpuParticleSystem;
	std::vector<ModelEntry*> m_models;

	// Benchmarking
	int m_totalParticles;
	bool m_sampledInstancing;
	int m_sampleRounds;
	float m_timePassed;
	BenchmarkPhase m_bench;

	int m_fps, m_count;
	std::vector<float> m_deltaTimes;
	unsigned long m_startTime;
};

#endif