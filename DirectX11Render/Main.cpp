#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
//
// Created by Robin Andersson

// Credit to http://www.rastertek.com/tutdx11s2.html & http://www.rastertek.com/tutdx11.html
// Also credit to assimp
//

#include "Framework/System.h"
#include <iostream>

// Entry point of application
int main(int argc, CHAR* argv[])
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	System* system;
	bool result;

	// Create the system object.
	system = new System;
	if (!system)
	{
		return 0;
	}

	// Initialize and run the system object.
	result = system->Initialize();
	if (result)
	{
		system->Run();
	}

	// Shutdown and release the system object.
	delete system;
	system = 0;

	//_CrtDumpMemoryLeaks();
	return 0;
}