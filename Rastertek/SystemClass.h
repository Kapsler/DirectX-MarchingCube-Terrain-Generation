#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "InputClass.h"
#include "GraphicsClass.h"

class SystemClass
{
public:
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();


	LPCWSTR applicationName;
	HINSTANCE hinstance;
	HWND hwnd;

	InputClass* input;
	GraphicsClass* graphics;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static SystemClass* ApplicationHandle = nullptr;