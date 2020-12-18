#pragma once

#include "Model.h"
#include "PhongRenderPass.h"


class RenderObject {
public:
	RenderObject(Model* model, Game::Vector3 worldPosition,
		Game::Vector3 worldRotation, Game::Vector3 worldScale,
		const char* name = nullptr) : worldPosition(worldPosition),
		worldScale(worldScale), worldRotation(worldRotation),model(model) {
		if (name != nullptr) this->name = name;
		else {
			static size_t index = 0;
			this->name = std::string("_unnamed_renderobject") + std::to_string(index++);
		}
	}

	template<typename RPType>
	void Render(RPType* RenderPass) {
		if constexpr (std::is_same<RPType,PhongRenderPass>::value) {
			RenderByPhongPass(RenderPass);
		}
		else {
			//otherwise we do nothing
		}
	}

	Game::Vector3 GetWorldPosition() { return worldPosition; }
	Game::Vector3 GetWorldRotation() { return worldRotation; }
	Game::Vector3 GetWorldScale() { return worldScale; }

	void SetWorldPosition(Game::Vector3 position) { worldPosition = position; }
	void SetWorldRotation(Game::Vector3 rotation) { worldRotation = rotation; }
	void SetWorldScale(Game::Vector3 scale) { worldScale = scale; }

	~RenderObject();
private:
	void RenderByPhongPass(PhongRenderPass* rp);

	Model* model;
	std::string name;
	Game::Vector3 worldPosition;
	Game::Vector3 worldRotation;
	Game::Vector3 worldScale;

	struct {
		bool initialized = false;
		std::vector<PhongObjectID> phongObjectID;
		std::vector<PhongMaterialTexture> phongMaterialTextures;
	} phongRPData;
	
};