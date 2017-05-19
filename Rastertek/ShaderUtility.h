#pragma once
#include <d3d11shader.h>
#include <fstream>

namespace ShaderUtility
{
	static void OutputShaderErrorMessage(ID3D10Blob* errorMessage, WCHAR* shaderFilename)
	{
		char* compileErrors;
		unsigned long long bufferSize;
		std::ofstream fout;

		compileErrors = static_cast<char*>(errorMessage->GetBufferPointer());
		bufferSize = errorMessage->GetBufferSize();

		printf("Error compiling shader %ls\n\r", shaderFilename);

		fout.open("shader-error.txt");
		for (auto i = 0; i<bufferSize; i++)
		{
			fout << compileErrors;
		}
		fout.close();

		errorMessage->Release();
		errorMessage = nullptr;
	}
}
