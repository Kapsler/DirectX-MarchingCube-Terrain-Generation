#pragma once


class GeometryData
{
public:

	GeometryData(unsigned int width, unsigned int height, unsigned int depth);
	~GeometryData();

	void GenerateData();
	void DebugPrint();

	float* m_data;
	unsigned int m_width, m_height, m_depth;
};
