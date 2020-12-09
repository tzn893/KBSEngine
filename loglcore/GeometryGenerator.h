#pragma once
#include <vector>

namespace GeometryGenerator {
	enum GEOMETRY_FLAG{
		GEOMETRY_FLAG_NONE = 0,
		GEOMETRY_FLAG_DISABLE_NORMAL = 1,
		GEOMETRY_FLAG_DISABLE_TEXCOORD = 1 << 1
	};
	
	std::vector<float> Cube(float scalex,float scaley,float scalez,GEOMETRY_FLAG flag = GEOMETRY_FLAG_NONE);
	std::vector<float> Plane(float width,float height,size_t gridu,size_t gridv,std::vector<uint16_t>& indices,GEOMETRY_FLAG flag = GEOMETRY_FLAG_NONE);
	std::vector<float> Squre(float width,float height,std::vector<uint16_t>& indices,GEOMETRY_FLAG flag = GEOMETRY_FLAG_NONE);
	std::vector<float> Sphere(float radius, size_t gridu, size_t gridv,std::vector<uint16_t>& indices, GEOMETRY_FLAG flag = GEOMETRY_FLAG_NONE);
}
