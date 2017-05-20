#include "Scene.h"
#include <iostream>
#include <stdlib.h>
#pragma comment(lib, "winmm.lib")
#define PI 3.14159265359

Scene::Scene()
{
	m_renderTexture = 0;
	m_debugWindow = 0;
	m_textureShader = 0;
	m_Direct3D = 0;
	m_Camera = 0;
	m_SuperShader = 0;
	m_Light = 0;
	m_Floor = 0;
	m_particleshader = 0;
	m_particlesystem = 0;
	m_gpuParticleSystem = 0;
	m_benchStreamout = 0;
	m_benchInstancing = 0;

	m_fps = 0;
	m_count = 0;
	m_startTime = timeGetTime();

	m_currentParticleSystem = ParticleSystemType::None;
}

Scene::Scene(const Scene& other)
{
}

Scene::~Scene()
{
	// Release all objects.
	SAFE_DELETE(m_SuperShader);
	SAFE_DELETE(m_Camera);
	SAFE_DELETE(m_Direct3D);
	SAFE_DELETE(m_Light);
	SAFE_DELETE(m_debugWindow);
	SAFE_DELETE(m_textureShader);
	SAFE_DELETE(m_renderTexture);
	SAFE_DELETE(m_particleshader);
	SAFE_DELETE(m_particlesystem);
	SAFE_DELETE(m_gpuParticleSystem);
	SAFE_DELETE(m_benchStreamout);
	SAFE_DELETE(m_benchInstancing);

	// Delete all models in scene
	for (int i = 0; i < m_models.size(); i++)
	{
		if (m_models[i] != 0)
			delete m_models[i];
	}
	m_models.clear();
}

bool Scene::Initialize(int screenWidth, int screenHeight, HWND hwnd, InputHandler* input)
{
	bool result;

	m_Input = input;
	m_hwnd = hwnd;

	// Create the Direct3D object.
	m_Direct3D = new Direct3DManager;
	if (!m_Direct3D)
		return false;

	// Initialize the Direct3D object.
	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_Z_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new Camera;
	if (!m_Camera)
		return false;

	m_Floor = new Cube;
	if (!m_Floor)
		return false;

	result = m_Floor->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), "Assets/Cube.txt", new std::wstring(L"Fieldstone"), new std::wstring(L"png"));
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the floor object.", L"Error", MB_OK);
		return false;
	}

	// Create the texture shader object.
	m_SuperShader = new SuperShader;
	if (!m_SuperShader)
		return false;

	// Initialize the color shader object.
	result = m_SuperShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the super shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the light object.
	m_Light = new Light;
	if (!m_Light)
		return false;

	m_debugWindow = new DebugWindow;
	if (!m_debugWindow)
		return false;

	result = m_debugWindow->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, 100, 100);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the debug window object.", L"Error", MB_OK);
		return false;
	}

	m_textureShader = new TextureShader;
	if (!m_textureShader)
		return false;

	result = m_textureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	m_renderTexture = new RenderTexture;
	if (!m_renderTexture)
		return false;

	result = m_renderTexture->Initialize(m_Direct3D->GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_DEPTH, SCREEN_Z_NEAR);
	if (!result)
		return false;

	// Init particle Shader
	m_particleshader = new ParticleShader;

	if (!m_particleshader)
		return false;

	ASSERT(m_particleshader->Initialize(m_Direct3D->GetDevice(), hwnd));

	// Initialize the light object
	m_Light->SetPosition(300.f, 200.f, 0.f);
	m_Light->SetAmbientColor(0.6f, 0.6f, 0.6f, 1.f);
	m_Light->SetDiffuseColor(1.f, 1.f, 1.f, 1.f);
	m_Light->SetDirection(1.f, 0.f, 0.f);
	m_Light->SetSpecularColor(1.f, 1.f, 1.f, 1.f);
	m_Light->SetSpecularPower(32.f);
	m_Light->SetLookAt(0.f, 0.f, 0.f);
	m_Light->GenerateProjectionMatrix(SHADOWMAP_DEPTH, SHADOWMAP_Z_NEAR);

	// Set the initial position of the camera.
	m_Camera->SetPosition(65.0f, 30.0f, -65.0f);
	m_Camera->AddRotation(-45.0f, 15.0f);

	// Adjust model objects
	m_Floor->SetScale(XMFLOAT3(50.f, 0.5f, 50.f));
	m_Floor->SetPosition(XMFLOAT3(0.f, 0.f, 0.f));
	m_Floor->SetTextureScale(80.f);

	//AddModel(m_ak, true);
	AddModel(m_Floor);

	m_benchStreamout = new Benchmark;
	m_benchStreamout->Initialize("StreamOutFrameTime.txt", "StreamOutParticles.txt");
	m_benchInstancing = new Benchmark;
	m_benchInstancing->Initialize("InstancingFrameTime.txt", "InstancingParticles.txt");


	// Benchmarking tool
	m_bench = BenchmarkPhase::Nonet;
	m_totalParticles = m_incrementParticles;
	m_sampledInstancing = false;
	m_sampleRounds = 0;
	m_timePassed = 0;

	SwitchParticleSystem(ParticleSystemType::Instancing);
	return true;
}

bool Scene::Input(const float& dt)
{
	bool result;

	// Exits input when benchmark is running
	if (m_bench != BenchmarkPhase::Nonet)
		return true;
	if (m_Input->IsKeyPressed(Keys::Enter))
	{
		m_bench = BenchmarkPhase::Warmup;
		SwitchParticleSystem(ParticleSystemType::Instancing);
		m_sampledInstancing = true;
	}

	// Add camera rotation
	m_Camera->AddRotation(m_Input->GetMouseDeltaX() / 10.f, m_Input->GetMouseDeltaY() / 10.f);

	// Camera movement
	if (m_Input->IsKeyDown(Keys::W))
		m_Camera->MoveForward(dt, 1);
	else if (m_Input->IsKeyDown(Keys::S))
		m_Camera->MoveForward(dt, -1);

	// Camera speed boost
	if (m_Input->IsKeyDown(Keys::Shift))
		m_Camera->SetSpeed(50.0f);
	else
		m_Camera->SetSpeed(15.0f);

	// Strafe camera movement
	if (m_Input->IsKeyDown(Keys::D))
		m_Camera->Strafe(dt, 1);
	else if (m_Input->IsKeyDown(Keys::A))
		m_Camera->Strafe(dt, -1);

	if (m_Input->IsKeyPressed(Keys::One))
	{
		ASSERT(SwitchParticleSystem(ParticleSystemType::Instancing));
	}
	else if (m_Input->IsKeyPressed(Keys::Two))
	{
		ASSERT(SwitchParticleSystem(ParticleSystemType::StreamOut));
	}
	else if (m_Input->IsKeyPressed(Keys::Zero))
	{
		ASSERT(SwitchParticleSystem(ParticleSystemType::None));
	}


	return true;
}

bool Scene::SwitchParticleSystem(ParticleSystemType particleType)
{
	
	SAFE_DELETE(m_particlesystem);
	SAFE_DELETE(m_gpuParticleSystem);

	m_currentParticleSystem = particleType;
	switch (particleType)
	{
	case ParticleSystemType::Instancing:
		m_particlesystem = new ParticleSystem;
		if (!m_particlesystem)
			return false;
		ASSERT(m_particlesystem->Initialize(m_Direct3D, L"Assets/snow.png", m_totalParticles));
		m_particlesystem->SetParticlesPerSecond(m_totalParticles / 30);
		break;
	case ParticleSystemType::StreamOut:
		m_gpuParticleSystem = new GPUP::ParticleSys;
		if (!m_gpuParticleSystem)
			return false;

		ASSERT(m_gpuParticleSystem->Initialize(m_Direct3D, m_hwnd, L"Assets/snow.png", L"Assets/NoiseTextureColor2.jpg"));
		m_gpuParticleSystem->SetParticlesPerSecond(m_totalParticles / 30);
		break;
	default:
		break;
	}
	return true;
}

bool Scene::Frame(const float& dt)
{
	bool result;

	// Update benchmark tool
	if (m_bench != BenchmarkPhase::Nonet) {
		m_timePassed += dt;
		switch (m_bench)
		{
		case BenchmarkPhase::Warmup:
			// Warmup ends --> Begin sampling
			if (m_timePassed >= m_warmupTime)
			{
				m_timePassed = 0;
				m_bench = BenchmarkPhase::Sample;
			}
			break;
		case BenchmarkPhase::Sample:
			// sample ended
			if (m_timePassed >= m_sampleTime)
			{
				m_timePassed = 0;
				if (m_sampledInstancing == true)
				{
					m_bench = BenchmarkPhase::Warmup;
					SwitchParticleSystem(ParticleSystemType::StreamOut);
					m_sampledInstancing = false;
				}
				else
				{
					// Next round
					if (m_sampleRounds != m_rounds)
					{
						m_totalParticles += m_incrementParticles;
						m_sampleRounds++;
						m_benchInstancing->SaveRound();
						m_benchStreamout->SaveRound();
						m_bench = BenchmarkPhase::Warmup;
						SwitchParticleSystem(ParticleSystemType::Instancing);
						m_sampledInstancing = true;
					}
					else
					{
						// Done with benchmarking
						m_bench = BenchmarkPhase::Done;
						SwitchParticleSystem(ParticleSystemType::None);
						m_benchInstancing->SaveToFile();
						m_benchStreamout->SaveToFile();
					}
				}
			}
			break;
		}
	}

	// Update PS
	switch (m_currentParticleSystem)
	{
	case ParticleSystemType::Instancing:
		m_particlesystem->Frame(dt, m_Direct3D->GetDeviceContext());
		break;
	case ParticleSystemType::StreamOut:
		m_gpuParticleSystem->Frame(dt);
		break;
	}

	// Update Models
	for (int i = 0; i < m_models.size(); i++)
	{
		Model* model = m_models[i]->m_model;
		bool active = m_models[i]->m_active;
		if (model != 0 && active)
			model->Update(dt);
	}

	// Render the Scene scene.
	result = Render();
	if (!result)
		return false;

	UpdateBenchmarkCount(dt);

	return true;
}
void Scene::UpdateBenchmarkCount(float dt)
{
	int currentParticles = 0;
	int memoryInKb = 0;
	m_count++;
	m_deltaTimes.push_back(dt);
	if (timeGetTime() >= (m_startTime + 1000))
	{
		system("cls");
		switch (m_bench)
		{
		case BenchmarkPhase::Nonet:
			std::cout << "Press Enter to begin benchmarking" << std::endl;
			break;
		case BenchmarkPhase::Warmup:
			std::cout << "Benchmarking warmup, round: " << m_sampleRounds << std::endl;
			break;
		case BenchmarkPhase::Sample:
			std::cout << "Benchmarking sampling, round: " << m_sampleRounds << std::endl;
			break;
		case BenchmarkPhase::Done:
			std::cout << "Benchmarking complete" << std::endl;
			break;
		}

		float average = 0;
		for (int i = 0; i < m_deltaTimes.size(); ++i)
		{
			average += m_deltaTimes[i];
		}
		average = average / m_deltaTimes.size();
		m_deltaTimes.clear();

		std::cout << "FPS: " << m_count << std::endl;
		float ms = average * 1000.0f;
		std::cout << "Drawcall (ms): " << ms << std::endl;
		switch (m_currentParticleSystem)
		{
		case ParticleSystemType::Instancing:
			std::cout << "PS Type: Instancing" << std::endl;
			currentParticles = m_particlesystem->GetCurrentParticleCount();
			if (m_bench == BenchmarkPhase::Sample)
				m_benchInstancing->Bench(ms, m_totalParticles);
			break;
		case ParticleSystemType::StreamOut:
			std::cout << "PS Type: StreamOut" << std::endl;
			currentParticles = m_gpuParticleSystem->GetCurrentParticles();
			if (m_bench == BenchmarkPhase::Sample)
				m_benchStreamout->Bench(ms, m_totalParticles);
			break;
		}
		memoryInKb = (sizeof(ParticleSystem::VertexType) * currentParticles) / 1024;
		std::cout << "Memory: " << memoryInKb << " kb" << std::endl;
		std::cout << "Total particles: " << m_totalParticles << std::endl;
		std::cout << "Current particles: " << currentParticles << std::endl;

		m_fps = m_count;
		m_count = 0;
		m_startTime = timeGetTime();
	}
}

bool Scene::Render()
{
	bool result;

	result = RenderToTexture();
	if (!result)
		return false;

	result = RenderScene();
	if (!result)
		return false;

	return true;
}

bool Scene::RenderScene()
{
	bool result;

	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	m_Camera->Render();
	m_Light->GenerateViewMatrix();

	// Renders all objects in scene
	for (int i = 0; i < m_models.size(); i++)
	{
		Model* model = m_models[i]->m_model;
		bool active = m_models[i]->m_active;

		if (m_models[i] != 0 && active)
		{
			model->Render(m_Direct3D->GetDeviceContext());

			result = m_SuperShader->Render(*m_Direct3D, *m_Camera, *m_Light, *model, m_Light->GetViewMatrix(), m_Light->GetProjectionMatrix(), m_renderTexture->GetShaderResourceView(), m_Light->GetPosition(), XMMatrixIdentity());
			if (!result)
				return false;

			//result = m_textureshader->render(m_direct3d->getdevicecontext(), model->getindexcount(), model->getworldmatrix(), m_light->getviewmatrix(), m_light->getprojectionmatrix());
			//if (!result)
			//	return false;
		}
	}

	m_Direct3D->TurnOnAlphaBlending();
	switch (m_currentParticleSystem)
	{
	case ParticleSystemType::Instancing:
		m_particlesystem->Render(m_Direct3D->GetDeviceContext());
		ASSERT(m_particleshader->Render(m_Direct3D, m_particlesystem, m_Camera));
		break;
	case ParticleSystemType::StreamOut:
		ASSERT(m_gpuParticleSystem->Render(m_Direct3D, m_Camera));
		break;
	default:
		break;
	}
	
	m_Direct3D->TurnOffAlphaBlending();

	m_Direct3D->EndScene();
	return true;
}

bool Scene::RenderToTexture()
{
	bool result;
	XMMATRIX worldMatrix, lightViewMatrix, lightProjectionMatrix, translateMatrix;
	XMFLOAT3 pos, scale, rot;

	m_renderTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());
	m_renderTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);
	m_Light->GenerateViewMatrix();

	lightViewMatrix = m_Light->GetViewMatrix();
	lightProjectionMatrix = m_Light->GetProjectionMatrix();

	// Renders all objects in scene
	for (int i = 0; i < m_models.size(); i++)
	{
		Model* model = m_models[i]->m_model;
		bool active = m_models[i]->m_active;
		if (m_models[i] != 0 && active)
		{
			pos = XMFLOAT3(model->GetPosition());
			rot = XMFLOAT3(model->GetRotation());
			scale = XMFLOAT3(model->GetScale());
			worldMatrix = XMMatrixTranslation(pos.x, pos.y, pos.z) * XMMatrixRotationX(rot.x) * XMMatrixRotationY(rot.y) * XMMatrixRotationZ(rot.z);

			model->Render(m_Direct3D->GetDeviceContext());
			result = m_textureShader->Render(m_Direct3D->GetDeviceContext(), model->GetIndexCount(), model->GetWorldMatrix(), lightViewMatrix, lightProjectionMatrix);
			if (!result)
				return false;
		}
	}

	m_Direct3D->SetBackBufferRenderTarget();
	m_Direct3D->ResetViewport();
	return true;
}
