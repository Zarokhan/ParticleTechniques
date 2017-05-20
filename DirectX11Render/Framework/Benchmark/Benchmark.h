#ifndef _Benchmark_H_
#define _Benchmark_H_
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>

class Benchmark
{
public:
	Benchmark();
	~Benchmark();

	void Initialize(const char* frametimeFilename, const char* particlesFilename);
	void Bench(float frametime, int particles);
	void SaveRound();
	void SaveToFile();
private:
	const char* m_frametimeFilename;
	const char* m_particlesFilename;

	std::string m_frametime_str = "";
	std::string m_particles_str = "";

	// for round
	std::vector<float> m_frametime;
	std::vector<int> m_particles;

	std::ofstream m_stream;
};

#endif