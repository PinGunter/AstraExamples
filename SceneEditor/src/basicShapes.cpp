#include <basicShapes.h>
#include <glm/gtc/constants.hpp>

/**
* A lot of these geometries are computed following 
* Three.js implementation
* https://github.com/mrdoob/three.js/blob/master/src/geometries/
*/


Astra::Geometry BasicShapes::boxGeometry(float wide, float height, float depth)
{
	Astra::Geometry geo{};
	float x = wide / 2.0f;
	float y = height / 2.0f;
	float z = depth / 2.0f;

	geo.vertices = {
		// v
		{x, -y, z}, // 0 a
		{x, y, z}, // 1 b
		{-x, y, z}, // 2 c
		{-x, -y, z}, // 3 d
		{-x, -y, -z}, // 4 e
		{-x, y, -z}, // 5 f
		{x, y, -z}, // 6 g
		{x, -y, -z}, // 7 h

		// v'
		{x, -y, z}, // 8 a'
		{x, y, z}, // 9 b'
		{-x, y, z}, // 10 c'
		{-x, -y, z}, // 11 d'
		{-x, -y, -z}, // 12 e'
		{-x, y, -z}, // 13 f'
		{x, y, -z}, // 14 g
		{x, -y, -z}, // 15 h'

		//v''
		{x, -y, z}, // 16 a''
		{x, y, z}, // 17 b''
		{-x, y, z}, // 18 c''
		{-x, -y, z}, // 19 d''
		{-x, -y, -z}, // 20 e''
		{-x, y, -z}, // 21 f''
		{x, y, -z}, // 22 g''
		{x, -y, -z}, // 23 h''
	};
	geo.indices = {
		{0, 1, 2},
		{2, 3, 0},

		{9, 6, 5},
		{9, 5, 10},

		{8, 14, 17},
		{8, 7, 14},

		{11, 18, 13},
		{4, 11, 13},
		
		{12, 15, 16},
		{12, 16, 19},

		{20, 21, 22},
		{23, 20, 22}
	};

	geo.normals = {
		glm::vec3(0,0,1),// 0 
		glm::vec3(0,0,1), // 1
		glm::vec3(0,0,1), // 2
		glm::vec3(0,0,1), // 3
		glm::vec3(-1,0,0), // 4


		glm::vec3(0,1,0), // 5
		glm::vec3(0,1,0), // 6

		glm::vec3(1,0,0), // 7
		glm::vec3(1,0,0), // 8

		glm::vec3(0,1,0), // 9
		glm::vec3(0,1,0), // 10

		glm::vec3(-1,0,0), // 11
		glm::vec3(0,-1,0), // 12
		glm::vec3(-1,0,0), // 13
		glm::vec3(1,0,0), // 14
		glm::vec3(0,-1,0), // 15

		glm::vec3(0,-1,0), // 16
		glm::vec3(1,0,0), // 17
		glm::vec3(-1,0,0), // 18
		glm::vec3(0,-1,0), // 19
		glm::vec3(0,0,-1), // 20
		glm::vec3(0,0,-1), // 21
		glm::vec3(0,0,-1), // 22
		glm::vec3(0,0,-1), // 23
	};

	return geo;
}

Astra::Geometry BasicShapes::SphereGeometry(float radius, uint32_t resolutionX, uint32_t resolutionY)
{
	Astra::Geometry geo{};

	uint index = 0;
	std::vector<std::vector<uint>> grid{};

	glm::vec3 vertex{};

	const float phiLength =  glm::pi<float>() * 2.0f ;
	const float thetaLengt = glm::pi<float>();

	resolutionX = glm::max<uint32_t>(3, resolutionX);
	resolutionY = glm::max<uint32_t>(2, resolutionY);

	for (int iy = 0; iy <= resolutionY; iy++) {
		std::vector<uint> row{};

		float v = (float) iy / resolutionY;

		float uOffset = 0.0f;

		if (iy == 0) {
			uOffset = 0.5f / resolutionX;
		}
		else if (iy == resolutionY) {
			uOffset = -0.5 / resolutionX;
		}

		for (int ix = 0; ix <= resolutionX; ix++) {
			float u = (float ) ix / resolutionX;

			vertex.x = -radius * cos(u * phiLength) * sin(v * thetaLengt);
			vertex.y = radius * cos(v * thetaLengt);
			vertex.z = radius * sin(u * phiLength) * sin(v * thetaLengt);

			geo.vertices.push_back(vertex);

			geo.normals.push_back(glm::normalize(vertex));

			row.push_back(index++);
		}
		grid.push_back(row);
	}

	// indices
	for (int iy = 0; iy < resolutionY; iy++) {
		for (int ix = 0; ix < resolutionX; ix++) {
			uint a, b, c, d;

			a = grid[iy][ix + 1];
			b = grid[iy][ix];
			c = grid[iy + 1][ix];
			d = grid[iy + 1][ix + 1];

			if (iy != 0) geo.indices.push_back({ a,b,d });
			if (iy != resolutionY - 1) geo.indices.push_back({ b,c,d });
		}
	}


	return geo;
}

Astra::Geometry BasicShapes::CylinderGeometry(float radius, float height, uint32_t resolution)
{
	return Astra::Geometry();
}

Astra::Geometry BasicShapes::ConeGeometry(float radius, float height, uint32_t resolution)
{
	return Astra::Geometry();
}

Astra::Geometry BasicShapes::IcosahedronGeometry(float radius)
{
	return Astra::Geometry();
}

Astra::Geometry BasicShapes::OctahedronGeometry(float size)
{
	return Astra::Geometry();
}

Astra::Geometry BasicShapes::TetrahedronGeometry(float size)
{
	return Astra::Geometry();
}

Astra::Geometry BasicShapes::TorusGeometry(float radius, float tube, uint32_t radResolution, uint32_t tubeResolution)
{
	Astra::Geometry geo{};

	glm::vec3 center{}, vertex{}, normal{};
	float pi = glm::pi<float>();

	// vertices
	for (int j = 0; j <= radResolution; j++) {
		for (int i = 0; i <= tubeResolution; i++) {
			float u = (float) i / tubeResolution * pi * 2.0f;
			float v = (float) j / radResolution * pi * 2.0f;

			// vertex
			vertex.x = (radius + tube * cos(v)) * cos(u);
			vertex.y = (radius + tube * cos(v)) * sin(u);
			vertex.z = tube * sin(v);

			geo.vertices.push_back(vertex);

			// normal
			center.x = radius * cos(u);
			center.y = radius * sin(u);
			normal = glm::normalize(vertex - center);

			geo.normals.push_back(normal);
		}
	}

	// indices
	for (int j = 1; j <= radResolution; j++) {
		for (int i = 1; i <= tubeResolution; i++) {
			uint a = (tubeResolution + 1) * j + i - 1;
			uint b = (tubeResolution + 1) * (j - 1) + i - 1;
			uint c = (tubeResolution + 1) * (j - 1) + i;
			uint d = (tubeResolution + 1) * j + i;

			geo.indices.push_back({ a, b, d });
			geo.indices.push_back({ b, c, d });
		}
	}

	return geo;
}
