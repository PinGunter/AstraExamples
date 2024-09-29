#include <basicShapes.h>

Astra::Geometry BasicShapes::boxGeometry(float wide, float height, float depth)
{
	Astra::Geometry geo{};
	float x = wide / 2.0f;
	float y = height / 2.0f;
	float z = depth / 2.0f;

	geo.vertices = {
		{x, -y, z}, // 0 a
		{x, y, z}, // 1 b
		{-x, y, z}, // 2 c
		{-x, -y, z}, // 3 d
		{-x, -y, -z}, // 4 e
		{-x, y, -z}, // 5 f
		{x, y, -z}, // 6 g
		{x, -y, -z}, // 7 h
	};
	geo.indices = {
		{0, 1, 2},
		{2, 3, 0},
		{1, 6, 5},
		{5, 2, 1},
		{0, 7, 6},
		{6, 1, 0},
		{2, 5, 3},
		{5, 4, 3},
		{0, 3, 7},
		{3, 4, 7},
		{4, 5, 6},
		{6, 7, 4}
	};

	geo.normals = {
		glm::normalize(glm::vec3(1,-1,1)),
		glm::normalize(glm::vec3(1,1,1)),
		glm::normalize(glm::vec3(-1,1,1)),
		glm::normalize(glm::vec3(-1,-1,1)),
		glm::normalize(glm::vec3(-1,-1,-1)),
		glm::normalize(glm::vec3(-1,1,-1)),
		glm::normalize(glm::vec3(1,1,-1)),
		glm::normalize(glm::vec3(1,-1,-1)),
	};

	return geo;
}

Astra::Geometry BasicShapes::SphereGeometry(float radius, uint32_t resolutionX, uint32_t resolutionY)
{
	return Astra::Geometry();
}

Astra::Geometry BasicShapes::CylinderGeometry(float radius, float height, uint32_t resolution)
{
	return Astra::Geometry();
}

Astra::Geometry BasicShapes::ConeGeometry(float radius, float height, uint32_t resolution)
{
	return Astra::Geometry();
}

Astra::Geometry BasicShapes::IcosahedronGeometry(float radius, uint32_t resolution)
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
	return Astra::Geometry();
}
