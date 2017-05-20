#ifndef _SYSTEM_H_
#define _SYSTEM_H_

///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
#define WIN32_LEAN_AND_MEAN

//////////////
// INCLUDES //
//////////////
#include <windows.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "InputHandler.h"
#include "Scene.h"
#include "DirectX.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: System
////////////////////////////////////////////////////////////////////////////////
class System
{
public:
	System();
	System(const System&);
	~System();

	bool Initialize();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);	// Handles the windows system messages

private:
	bool Frame(const float& dt);	// Called each loop
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	InputHandler* m_Input;
	Scene* m_Scene;
};


/////////////////////////
// FUNCTION PROTOTYPES //
/////////////////////////
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


/////////////
// GLOBALS //
/////////////
static System* ApplicationHandle = 0;


#endif