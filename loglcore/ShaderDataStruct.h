#pragma once

#include "Math.h"
#include <stdint.h>

struct CameraPass {
	Game::Mat4x4  viewMat;
	Game::Mat4x4  perspectMat;
	Game::Mat4x4  transInvView;
	Game::Vector3 cameraPos;
	float exposure;
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

constexpr size_t SHADER_MAX_LIGHT_STRUCT_NUM = 8;
constexpr const char* getShaderMaxLightStructNumStr = "8";

struct LightData {
	Game::Vector3 intensity;
	float  fallStart;
	union{
		Game::Vector3 direction;
		Game::Vector3 position;
	};
	float  fallEnd;
	uint32_t type;
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
	uint32_t  lightNum;
};

struct Material {
	Game::Vector4 diffuse;
	Game::Vector3 FresnelR0;
	float Roughness;
	Game::Mat4x4 matTransform;
	float Metallic;
	Game::Vector3 emission;

	void SetMaterialTransform(Game::Vector2 offset,Game::Vector2 Scale) {
		float buffer[] = {
			Scale[0] ,0.	   ,0.,0.,
			0.		 ,Scale[1] ,0.,0.,
			offset[0],offset[1],1.,0.,
			0.		 ,0.	   ,0.,1.
		};
		matTransform = Game::Mat4x4(buffer);
	}
};

struct ObjectPass{
	Game::Mat4x4 world;
	Game::Mat4x4 transInvWorld;
	Material     material;
};


struct MeshVertex {
	Game::Vector3 Position;
	Game::Vector3 Normal;
	Game::Vector2 TexCoord;
};

struct MeshVertexNormal {
	Game::Vector3 Position;
	Game::Vector3 Normal;
	Game::Vector2 TexCoord;
	Game::Vector3 Tangent;
};

struct SkinnedNormalVertex {
	MeshVertexNormal data;
	uint32_t  boneIndices[4];
	Game::Vector4   boneWeights;
};


template<typename T>
inline constexpr size_t getVertexStrideByFloat() {
	return sizeof(T) / 4;
}