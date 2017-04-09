#include "Inputclass.h"

InputClass::InputClass()
{
}

InputClass::InputClass(const InputClass&)
{
}

InputClass::~InputClass()
{
}

void InputClass::Initialize()
{
	int i;

	//Initialize all keys beeing unpressed
	for(i=0; i<256; i++)
	{
		keys[i] = false;
	}
}

void InputClass::KeyDown(unsigned int input)
{
	keys[input] = true;
}

void InputClass::KeyUp(unsigned int input)
{
	keys[input] = false;
}

bool InputClass::IsKeyDown(unsigned int key)
{
	return keys[key];
}

bool InputClass::IsKeyUp(unsigned int key)
{
	return !keys[key];
}