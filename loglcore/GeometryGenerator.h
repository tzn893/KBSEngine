#pragma once
#include <vector>

enum GEOMETRY_FLAG {
	GEOMETRY_FLAG_NONE = 0,
	GEOMETRY_FLAG_DISABLE_NORMAL = 1,
	GEOMETRY_FLAG_DISABLE_TEXCOORD = 1 << 1,
	GEOMETRY_FLAG_DISABLE_TANGENT = 1 << 2
};

namespace GeometryGenerator {
	//Cube has no index buffer
	std::vector<float> Cube(float scalex,float scaley,float scalez, int flag = GEOMETRY_FLAG_NONE);
	std::pair<std::vector<float>,std::vector<uint16_t>> Plane(float width,float height,size_t gridu,size_t gridv, int flag = GEOMETRY_FLAG_NONE);
	std::pair<std::vector<float>, std::vector<uint16_t>> Square(float width,float height, int flag = GEOMETRY_FLAG_NONE);
	std::pair<std::vector<float>, std::vector<uint16_t>> Sphere(float radius, size_t faceNum, int flag = GEOMETRY_FLAG_NONE);
}
