#include "SystemClass.h"
#include <iostream>

SystemClass::SystemClass()
{
	input = nullptr;
	graphics = nullptr;
}

SystemClass::SystemClass(const SystemClass&)
{
}

SystemClass::~SystemClass()
{
}

bool SystemClass::Initialize()
{
	int screenWidth, screenHeight;
	bool result;

	//Initialize width and height
	screenWidth = 0;
	screenHeight = 0;

	//Initialite Windows API
	InitializeWindows(screenWidth, screenHeight);

	//Create Input
	input = new InputClass();
	if(!input)
	{
		return false;
	}

	//Initialize input object
	input->Initialize();

	//Create Graphics
	graphics = new GraphicsClass();
	if(!graphics)
	{
		return false;
	}

	//Initialize Graphics Object
	result = graphics->Initialize(screenWidth, screenHeight, hwnd);
	if(!result)
	{
		return false;
	}

	return true;
}


void SystemClass::Shutdown()
{
	//Release objects
	if(graphics)
	{
		graphics->Shutdown();
		delete graphics;
		graphics = nullptr;
	}

	if(input)
	{
		delete input;
		input = nullptr;
	}

	//Shutdown window
	ShutdownWindows();
}

void SystemClass::Run()
{
	MSG msg;
	bool done, result;

	//Initialize the message structure
	ZeroMemory(&msg, sizeof(MSG));

	//Loop until there is a quit message from the window
	done = false;
	while(!done)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//If exit signal
		if (msg.message == WM_QUIT)
		{
			done = true;
		} else {
			//Otherwise process Frame
			result = Frame();
			if(!result)
			{
				done = true;
			}
		}
	}
}

LRESULT SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch(umsg)
	{
		//Check if keypress
		case WM_KEYDOWN:
		{
			input->KeyDown(static_cast<unsigned int>(wparam));
			return 0;
		}

		//Check if key has been released
		case WM_KEYUP:
		{
			input->KeyUp(static_cast<unsigned int>(wparam));
			return 0;
		}

		//Any ther message send to default message handler
		default:
		{
			return DefWindowProc(hwnd, umsg, wparam, lparam);
		}
		
	}
}

bool SystemClass::Frame()
{
	bool result;

	//Check if escape is pressed
	if (input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	//Do the frame processing for graphics
	result = graphics->Frame(input);
	if(!result)
	{
		return false;
	}

	return true;
}

void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	//Get external pointer
	ApplicationHandle = this;

	//Get instance of application
	hinstance = GetModuleHandle(nullptr);

	//Give application a name
	applicationName = L"Engine";

	//Setup the windows class default settings
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WNDPROC(WndProc);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	//Register window class
	RegisterClassEx(&wc);

	//Determine resolution of desktop
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	//Setup screen settings depending on fullscreen or windowed mode
	if(FULL_SCREEN)
	{
		//If fullscreen, set full size
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = static_cast<unsigned long>(screenWidth);
		dmScreenSettings.dmPelsHeight = static_cast<unsigned long>(screenHeight);
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		//Change to fullscreen
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		//Set position to top left corner
		posX = posY = 0;
	} else {
		//If windowed
		screenWidth = 1600;
		screenHeight = 900;

		//Place Window in middle of screen
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	//Create window with screen settings and get handle
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, applicationName, applicationName, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP, posX, posY, screenWidth, screenHeight, nullptr, nullptr, hinstance, nullptr);

	//Bring the window up and set focus
	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//Hide Cursor
	ShowCursor(false);
}

void SystemClass::ShutdownWindows()
{
	ShowCursor(true);

	//Fix display settings
	if(FULL_SCREEN)
	{
		ChangeDisplaySettings(nullptr, 0);
	}

	//Remove Window
	DestroyWindow(hwnd);
	hwnd = nullptr;

	//Remove Application Instance
	UnregisterClass(applicationName, hinstance);
	hinstance = nullptr;

	ApplicationHandle = nullptr;
}

LRESULT WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch(umessage)
	{
		//Check if window destroyed
		case WM_DESTROY: 
		{
			PostQuitMessage(0);
			return 0;
		}

		//check window closed
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		//All other messages pass to handler
		default:
		{
			return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}
