#pragma once
#include <Mesh.h>

class BasicShapes {
public:
	static Astra::Geometry boxGeometry(float wide = 1.0f, float height = 1.0f, float depth = 1.0f);
	static Astra::Geometry SphereGeometry(float radius = 1.0f, uint32_t resolutionX = 10, uint32_t resolutionY = 10);
	static Astra::Geometry CylinderGeometry(float radius = 1.0f, float height = 1.0f, uint32_t resolution = 10);
	static Astra::Geometry ConeGeometry(float radius = 1.0f, float height = 1.0f, uint32_t resolution = 10);
	static Astra::Geometry IcosahedronGeometry(float radius = 1.0f, uint32_t resolution = 0);
	static Astra::Geometry OctahedronGeometry(float size = 1.0f);
	static Astra::Geometry TetrahedronGeometry(float size = 1.0f);
	static Astra::Geometry TorusGeometry(float radius = 10.0f, float tube = 3.0f, uint32_t radResolution = 16, uint32_t tubeResolution = 16);
};