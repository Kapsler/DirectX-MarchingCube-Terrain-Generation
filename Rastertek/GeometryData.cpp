#include "GeometryData.h"
#include <iostream>

GeometryData::GeometryData(unsigned width, unsigned height, unsigned depth)
	: m_width(width), m_height(height), m_depth(depth)
{
	GenerateData();
}

GeometryData::~GeometryData()
{
	delete m_data;
}

void GeometryData::GenerateData()
{
	m_data = new float[m_width*m_depth*m_height];

	unsigned int width_offset = m_width / 4;
	unsigned int height_offset = m_height / 4;
	unsigned int depth_offset = m_depth / 4;

	size_t index = 0;

	for(size_t i = 0u; i < m_depth; ++i)
	{
		for (size_t j = 0u; j < m_height; ++j)
		{
			for (size_t k = 0u; k < m_width; ++k)
			{

				if(
					k >= 0 + width_offset && k <= m_width - width_offset 
					&& j >= 0 + height_offset && j <= m_height - height_offset
					&& i >= 0 + depth_offset && i <= m_depth - depth_offset)
				{
					m_data[index] = 1.0f;
				} else
				{
					m_data[index] = 0.0f;
				}

				index++;

			}
		}
	}


}

void GeometryData::DebugPrint()
{
	char* output = new char[m_width + 1];
	size_t index = 0;

	for (size_t i = 0u; i < m_depth; ++i)
	{
		for (size_t j = 0u; j < m_height; ++j)
		{
			for (size_t k = 0u; k < m_width; ++k)
			{
				if(m_data[index] == 0)
				{
					output[k] = '0';
				} else
				{
					output[k] = '1';
				}

				index++;
			}
			output[m_width] = '\0';
			printf("%s\n", output);
		}

		printf("\n===========================\n\n");
	}
}
