# DirectX-MarchingCube-Terrain-Generation
Implementing some terrain using the Marching Cube algorithm

The density values are created on the CPU before the first Renderpass.
The Geometry Shader produces up to 18 vertices via the Marchin Cube algorithm.
Pixelshader uses Flatshading and normals as color value.
