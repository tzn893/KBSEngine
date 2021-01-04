#include "GeometryGenerator.h"
#include "Vector.h"
using namespace GeometryGenerator;

static void SetVectors(float* target,Game::Vector3 Pos,Game::Vector3 Nom,Game::Vector3 tangent,Game::Vector2 TexCoord, int flag) {
	target[0] = Pos[0], target[1] = Pos[1], target[2] = Pos[2];
	target += 3;
	if (!(flag & GEOMETRY_FLAG_DISABLE_NORMAL)) {
		target[0] = Nom[0], target[1] = Nom[1], target[2] = Nom[2];
		target += 3;
	}
	if (!(flag & GEOMETRY_FLAG_DISABLE_TEXCOORD)) {
		target[0] = TexCoord[0], target[1] = TexCoord[1];
		target += 2;
	}
	if (!(flag & GEOMETRY_FLAG_DISABLE_TANGENT)) {
		target[0] = tangent[0], target[1] = tangent[1], target[2] = tangent[2];
		target += 3;
	}
}

static size_t GetVertexStride(int flag) {
	size_t result = 3;
	if(!(flag & GEOMETRY_FLAG_DISABLE_NORMAL)) result += 3;
	if(!(flag & GEOMETRY_FLAG_DISABLE_TEXCOORD)) result += 2;
	if(!(flag & GEOMETRY_FLAG_DISABLE_TANGENT)) result += 3;
	return result;
}

std::vector<float> GeometryGenerator::Cube(float scalex, float scaley, float scalez, int flag) {
	size_t vertexStride = GetVertexStride(flag);
	size_t vertexNum = 36;//6 * 2 * 3

	std::vector<float> result(vertexNum * vertexStride);
	float* rarray = result.data();

	Game::Vector3 VPositions[8] = {
		{ scalex, scaley, scalez},
		{ scalex, scaley,-scalez},
		{ scalex,-scaley,-scalez},
		{ scalex,-scaley, scalez},
		{-scalex,-scaley, scalez},
		{-scalex,-scaley,-scalez},
		{-scalex, scaley,-scalez},
		{-scalex, scaley, scalez}
	};
	Game::Vector3 VNormal[6] = {
		{ 1.f, 0.f, 0.f},
		{-1.f, 0.f, 0.f},
		{ 0.f, 1.f, 0.f},
		{ 0.f,-1.f, 0.f},
		{ 0.f, 0.f, 1.f},
		{ 0.f, 0.f,-1.f}
	};
	Game::Vector3 VTangent[6] = {
		{ 0.f, 0.f, 1.f},
		{ 0.f, 0.f,-1.f},
		{ 1.f, 0.f, 0.f},
		{-1.f, 0.f, 0.f},
		{-1.f, 0.f, 0.f},
		{ 1.f, 0.f, 0.f}
	};
	Game::Vector2 Texcoord[8] = {
		{0.f,1.f},
		{1.f,1.f},
		{1.f,0.f},
		{0.f,0.f},
		{1.f,0.f},
		{0.f,0.f},
		{0.f,1.f},
		{1.f,1.f}
	};
	/*1*/
	SetVectors(rarray, VPositions[0], VNormal[4], VTangent[4], Texcoord[4], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[4], VNormal[4], VTangent[4], Texcoord[0], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[3], VNormal[4], VTangent[4], Texcoord[7], flag);
	rarray += vertexStride;
	/*2*/
	SetVectors(rarray, VPositions[0], VNormal[4], VTangent[4], Texcoord[4], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[7], VNormal[4], VTangent[4], Texcoord[3], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[4], VNormal[4], VTangent[4], Texcoord[0], flag);
	rarray += vertexStride;
	/*3*/
	SetVectors(rarray, VPositions[1], VNormal[5], VTangent[5], Texcoord[1], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[2], VNormal[5], VTangent[5], Texcoord[2], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[5], VNormal[5], VTangent[5], Texcoord[5], flag);
	rarray += vertexStride;
	/*4*/
	SetVectors(rarray, VPositions[1], VNormal[5], VTangent[5], Texcoord[1], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[5], VNormal[5], VTangent[5], Texcoord[5], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[6], VNormal[5], VTangent[5], Texcoord[6], flag);
	rarray += vertexStride;
	/*5*/
	SetVectors(rarray, VPositions[0], VNormal[0], VTangent[0], Texcoord[0], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[3], VNormal[0], VTangent[0], Texcoord[3], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[2], VNormal[0], VTangent[0], Texcoord[2], flag);
	rarray += vertexStride;
	/*6*/
	SetVectors(rarray, VPositions[0], VNormal[0], VTangent[0], Texcoord[0], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[2], VNormal[0], VTangent[0], Texcoord[2], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[1], VNormal[0], VTangent[0], Texcoord[1], flag);
	rarray += vertexStride;
	/*7*/
	SetVectors(rarray, VPositions[7], VNormal[1], VTangent[1], Texcoord[7], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[5], VNormal[1], VTangent[1], Texcoord[5], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[4], VNormal[1], VTangent[1], Texcoord[4], flag);
	rarray += vertexStride;
	/*8*/
	SetVectors(rarray, VPositions[7], VNormal[1], VTangent[1], Texcoord[7], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[6], VNormal[1], VTangent[1], Texcoord[6], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[5], VNormal[1], VTangent[1], Texcoord[5], flag);
	rarray += vertexStride;
	/*9*/
	SetVectors(rarray, VPositions[1], VNormal[2], VTangent[2], Texcoord[2], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[7], VNormal[2], VTangent[2], Texcoord[6], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[0], VNormal[2], VTangent[2], Texcoord[1], flag);
	rarray += vertexStride;
	/*10*/
	SetVectors(rarray, VPositions[1], VNormal[2], VTangent[2], Texcoord[2], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[6], VNormal[2], VTangent[2], Texcoord[5], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[7], VNormal[2], VTangent[2], Texcoord[6], flag);
	rarray += vertexStride;
	/*11*/
	SetVectors(rarray, VPositions[3], VNormal[3], VTangent[3], Texcoord[2], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[5], VNormal[3], VTangent[3], Texcoord[0], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[2], VNormal[3], VTangent[3], Texcoord[1], flag);
	rarray += vertexStride;
	/*12*/
	SetVectors(rarray, VPositions[3], VNormal[3], VTangent[3], Texcoord[2], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[4], VNormal[3], VTangent[3], Texcoord[3], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[5], VNormal[3], VTangent[3], Texcoord[0], flag);
	rarray += vertexStride;
	return std::move(result);
}

std::pair<std::vector<float>, std::vector<uint16_t>> GeometryGenerator::Plane(float width, float height, size_t gridu, size_t gridv, int flag) {
	size_t vertexStride = GetVertexStride(flag);
	size_t vertexNum = (gridu + 1)* (gridv + 1);//6 * 2 * 3

	std::vector<float> vdata(vertexNum * vertexStride);
	float* varray = vdata.data();
	
	for (size_t y = 0; y <= gridv; y++) {
		for (size_t x = 0; x <= gridu; x++) {

			Game::Vector3 normal   = Game::Vector3(0., 1., 0.);
			Game::Vector3 tangent  = Game::Vector3(1., 0., 0.);
			Game::Vector2 Uv       = Game::Vector2(
				static_cast<float>(x) / static_cast<float>(gridu),
				static_cast<float>(y) / static_cast<float>(gridv)
			);
			Game::Vector3 position = Game::Vector3((Uv.x - .5) * width, 0., (.5 - Uv.y) * height);
			
			SetVectors(varray, position, normal, tangent, Uv, flag);
			varray += vertexStride;
		}
	}

	std::vector<uint16_t> idata(6 * gridu * gridv);
	uint16_t* iarray = idata.data();

	for (uint16_t x = 0; x < gridu; x++) {
		for (uint16_t y = 0; y < gridv; y++) {
			uint16_t lu = x + y * (gridu + 1), ru = x + 1 + y * (gridu + 1),
				ld = x + (y + 1) * (gridu + 1), rd = x + 1 + (y + 1) * (gridu + 1);

			iarray[0] = lu, iarray[1] = ru, iarray[2] = rd;
			iarray[3] = lu, iarray[4] = rd, iarray[5] = ld;
			iarray += 6;
		}
	}

	return std::move(std::make_pair(std::move(vdata),std::move(idata)));
}

std::pair<std::vector<float>, std::vector<uint16_t>> GeometryGenerator::Square(float width, float height, int flag) {
	return std::move(Plane(width, height, 1, 1, flag));
}
std::pair<std::vector<float>, std::vector<uint16_t>> GeometryGenerator::Sphere(float radius, size_t gridu, size_t gridv, int flag) {
	return std::make_pair(std::vector<float>(),std::vector<uint16_t>());
}