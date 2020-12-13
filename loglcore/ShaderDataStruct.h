#pragma once

#include "Math.h"

struct CameraPass {
	Game::Mat4x4  viewMat;
	Game::Mat4x4  perspectMat;
	Game::Vector3 cameraPos;
};

enum SHADER_LIGHT_TYPE {
	SHADER_LIGHT_TYPE_POINT = 0,
	SHADER_LIGHT_TYPE_DIRECTIONAL = 1
};

template<SHADER_LIGHT_TYPE type>
inline constexpr const char* getShaderLightTypeStr() {
	if constexpr (type == SHADER_LIGHT_TYPE_POINT) {
		return "0";
	}
	else if constexpr (type == SHADER_LIGHT_TYPE_DIRECTIONAL) {
		return "1";
	}
	else {
		return "";
	}
}

constexpr size_t SHADER_MAX_LIGHT_STRUCT_NUM = 1;
constexpr const char* getShaderMaxLightStructNumStr = "1";

struct LightData {
	Game::Vector3 intensity;
	float  fallStart;
	union{
		Game::Vector3 direction;
		Game::Vector3 position;
	};
	float  fallEnd;
	int type;
	Game::Vector3 trash;

	LightData() {
		fallStart = 0;
		fallEnd = 0;
		type = SHADER_LIGHT_TYPE_POINT;
	}
};

struct LightPass {
	Game::Vector4 ambient;
	LightData lights[SHADER_MAX_LIGHT_STRUCT_NUM];
};

struct Material {
	Game::Vector4 diffuse;
	Game::Vector3 FresnelR0;
	float  Roughness;
};

struct ObjectPass{
	Game::Mat4x4 world;
	Game::Mat4x4 transInvWorld;
	Material     material;
};