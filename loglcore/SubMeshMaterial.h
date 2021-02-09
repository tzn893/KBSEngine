#pragma once
#include "ShaderDataStruct.h"
#include "Texture.h"

enum SUBMESH_MATERIAL_TYPE {
	SUBMESH_MATERIAL_TYPE_DIFFUSE = 0,
	SUBMESH_MATERIAL_TYPE_SPECULAR = 1,
	SUBMESH_MATERIAL_TYPE_BUMP = 2,
	SUBMESH_MATERIAL_TYPE_EMISSION = 3,
	SUBMESH_MATERIAL_TYPE_METALNESS = 4,
	SUBEMSH_MATERIAL_TYPE_ROUGHNESS = 5,
	SUBMESH_MATERIAL_TYPE_NUM = 6
};

struct SubMeshMaterial {
	Texture* textures[SUBMESH_MATERIAL_TYPE_NUM];
	Game::Vector3 diffuse;
	float roughness;
	float metallic;
	Game::Vector3 emissionScale;
	Game::Vector3 specular;
	Game::Vector2 matTransformOffset;
	Game::Vector2 matTransformScale;

	SubMeshMaterial() {
		for (int i = 0; i != _countof(textures); i++)
			textures[i] = nullptr;
		diffuse = Game::Vector3(1.,1.,1.);
		roughness = 1.;
		metallic = .3;
		specular = Game::Vector3();
		matTransformOffset = Game::Vector2();
		matTransformScale = Game::Vector2(1., 1.);
	}

};