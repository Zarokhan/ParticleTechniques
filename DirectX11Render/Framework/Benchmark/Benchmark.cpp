#include "Benchmark.h"

Benchmark::Benchmark()
{
}

Benchmark::~Benchmark()
{
}

void Benchmark::Initialize(const char * frametimeFilename, const char * particlesFilename)
{
	m_frametimeFilename = frametimeFilename;
	m_particlesFilename = particlesFilename;
}

void Benchmark::Bench(float frametime, int particles)
{
	m_frametime.push_back(frametime);
	m_particles.push_back(particles);
}

void Benchmark::SaveRound()
{
	int count = m_frametime.size();
	float total = 0;
	for each (float time in m_frametime)
	{
		total += time;
	}
	float avrage_frametime = total / (float)count;
	m_frametime_str += std::to_string(avrage_frametime) + '\n';
	m_particles_str += std::to_string(m_particles[0]) + '\n';

	m_frametime.clear();
	m_particles.clear();
}

void Benchmark::SaveToFile()
{
	m_stream.open(m_frametimeFilename);
	//std::replace(m_drawcall.begin(), m_drawcall.end(), '.', ',');
	m_stream << m_frametime_str.c_str();
	m_stream.close();
	m_stream.open(m_particlesFilename);
	m_stream << m_particles_str.c_str();
	m_stream.close();
}
