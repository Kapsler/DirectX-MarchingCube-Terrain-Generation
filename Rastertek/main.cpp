#include "Systemclass.h"
#include <iostream>

int main()
{
	SystemClass* System;
	bool result;

	//Create system obj
	System = new SystemClass();
	if (!System)
	{
		return 0;
	}

	//Initialize System
	result = System->Initialize();
	if(result)
	{
		System->Run();
	}

	//Shutdown and release
	System->Shutdown();
	delete System;
	System = nullptr;

	return 0;
}
