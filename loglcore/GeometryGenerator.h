#pragma once
#include <vector>


namespace GeometryGenerator {
	enum GEOMETRY_FLAG{
		GEOMETRY_FLAG_NONE = 0,
		GEOMETRY_FLAG_DISABLE_NORMAL = 1,
		GEOMETRY_FLAG_DISABLE_TEXCOORD = 1 << 1,
		GEOMETRY_FLAG_DISABLE_TANGENT = 1 << 2
	};
	
	//Cube has no index buffer
	std::vector<float> Cube(float scalex,float scaley,float scalez,GEOMETRY_FLAG flag = GEOMETRY_FLAG_NONE);
	std::pair<std::vector<float>,std::vector<uint16_t>> Plane(float width,float height,size_t gridu,size_t gridv,GEOMETRY_FLAG flag = GEOMETRY_FLAG_NONE);
	std::pair<std::vector<float>, std::vector<uint16_t>> Square(float width,float height,GEOMETRY_FLAG flag = GEOMETRY_FLAG_NONE);
	std::pair<std::vector<float>, std::vector<uint16_t>> Sphere(float radius, size_t gridu, size_t gridv, GEOMETRY_FLAG flag = GEOMETRY_FLAG_NONE);
}
