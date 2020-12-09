#include "GeometryGenerator.h"
#include "Vector.h"
using namespace GeometryGenerator;

static void SetVectors(float* target,Game::Vector3 Pos,Game::Vector3 Nom,Game::Vector2 TexCoord,GEOMETRY_FLAG flag) {
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
}

static size_t GetVertexStride(GEOMETRY_FLAG flag) {
	size_t result = 3;
	if(!(flag & GEOMETRY_FLAG_DISABLE_NORMAL)) result += 3;
	if (!(flag & GEOMETRY_FLAG_DISABLE_TEXCOORD)) result += 2;
	return result;
}

std::vector<float> GeometryGenerator::Cube(float scalex, float scaley, float scalez, GEOMETRY_FLAG flag) {
	size_t vertexStride = GetVertexStride(flag);
	size_t vertexNum = 36;//6 * 2 * 3

	std::vector<float> result(36 * vertexStride);
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
	SetVectors(rarray, VPositions[0], VNormal[4], Texcoord[0], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[4], VNormal[4], Texcoord[4], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[3], VNormal[4], Texcoord[3], flag);
	rarray += vertexStride;
	/*2*/
	SetVectors(rarray, VPositions[0], VNormal[4], Texcoord[0], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[7], VNormal[4], Texcoord[4], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[4], VNormal[4], Texcoord[7], flag);
	rarray += vertexStride;
	/*3*/
	SetVectors(rarray, VPositions[1], VNormal[5], Texcoord[1], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[2], VNormal[5], Texcoord[2], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[5], VNormal[5], Texcoord[5], flag);
	rarray += vertexStride;
	/*4*/
	SetVectors(rarray, VPositions[1], VNormal[5], Texcoord[1], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[5], VNormal[5], Texcoord[5], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[6], VNormal[5], Texcoord[6], flag);
	rarray += vertexStride;
	/*5*/
	SetVectors(rarray, VPositions[0], VNormal[0], Texcoord[0], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[3], VNormal[0], Texcoord[3], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[2], VNormal[0], Texcoord[2], flag);
	rarray += vertexStride;
	/*6*/
	SetVectors(rarray, VPositions[0], VNormal[0], Texcoord[0], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[2], VNormal[0], Texcoord[2], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[1], VNormal[0], Texcoord[1], flag);
	rarray += vertexStride;
	/*7*/
	SetVectors(rarray, VPositions[7], VNormal[1], Texcoord[7], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[5], VNormal[1], Texcoord[5], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[4], VNormal[1], Texcoord[4], flag);
	rarray += vertexStride;
	/*8*/
	SetVectors(rarray, VPositions[7], VNormal[1], Texcoord[7], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[6], VNormal[1], Texcoord[6], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[5], VNormal[1], Texcoord[5], flag);
	rarray += vertexStride;
	/*9*/
	SetVectors(rarray, VPositions[1], VNormal[2], Texcoord[1], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[7], VNormal[2], Texcoord[7], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[0], VNormal[2], Texcoord[0], flag);
	rarray += vertexStride;
	/*10*/
	SetVectors(rarray, VPositions[1], VNormal[2], Texcoord[1], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[6], VNormal[2], Texcoord[6], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[7], VNormal[2], Texcoord[7], flag);
	rarray += vertexStride;
	/*11*/
	SetVectors(rarray, VPositions[3], VNormal[3], Texcoord[3], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[5], VNormal[3], Texcoord[5], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[2], VNormal[3], Texcoord[2], flag);
	rarray += vertexStride;
	/*12*/
	SetVectors(rarray, VPositions[3], VNormal[3], Texcoord[3], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[4], VNormal[3], Texcoord[4], flag);
	rarray += vertexStride;
	SetVectors(rarray, VPositions[5], VNormal[3], Texcoord[5], flag);
	rarray += vertexStride;
	return std::move(result);
}

std::vector<float> Plane(float width, float height, size_t gridu, size_t gridv, GEOMETRY_FLAG flag = GEOMETRY_FLAG_NONE) {
	std::vector<float> result;
	return std::move(result);
}
std::vector<float> Squre(float width, float height, GEOMETRY_FLAG flag = GEOMETRY_FLAG_NONE);
std::vector<float> Sphere(float radius, size_t gridu, size_t gridv, GEOMETRY_FLAG flag = GEOMETRY_FLAG_NONE);