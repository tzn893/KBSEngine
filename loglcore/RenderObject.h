#pragma once

#include "Model.h"
#include "PhongRenderPass.h"
#include "DeferredRenderPass.h"
#include "ToonRenderPass.h"

class RenderObject {
public:
	RenderObject(Model* model, Game::Vector3 worldPosition,
		Game::Vector3 worldRotation, Game::Vector3 worldScale,
		const char* name = nullptr) : worldPosition(worldPosition),
		worldScale(worldScale), worldRotation(worldRotation),model(model) {
		if (name != nullptr) this->name = name;
		else {
			this->name = std::string("_unnamed_renderobject") + std::to_string(renderItemIndex++);
		}
		type = RENDER_OBJECT_TYPE_MODEL;
	}
	RenderObject(Mesh mesh,SubMeshMaterial material, Game::Vector3 worldPosition,
		Game::Vector3 worldRotation, Game::Vector3 worldScale,
		const char* name = nullptr):worldPosition(worldPosition),
		worldRotation(worldRotation),worldScale(worldScale){
		type = RENDER_OBJECT_TYPE_MESH;
		if (name != nullptr) this->name = name;
		else {
			this->name = std::string("_unnamed_renderobject") + std::to_string(renderItemIndex++);
		}
		this->mMesh = mesh;
		this->mMaterial = material;
	}
	template<typename RPType>
	void Render(RPType* RenderPass) {
		if constexpr (std::is_same<RPType, PhongRenderPass>::value) {
			switch (type) {
			case RENDER_OBJECT_TYPE_MODEL:
				RenderByPhongPass(RenderPass);
				break;
			case RENDER_OBJECT_TYPE_MESH:
				RenderByPhongPassMesh(RenderPass);
				break;
			}
		}
		else if constexpr (std::is_same<RPType,DeferredRenderPass>::value) {
			switch (type) {
			case RENDER_OBJECT_TYPE_MODEL:
				RenderByDeferredPass(RenderPass);
				break;
			case RENDER_OBJECT_TYPE_MESH:
				RenderByDeferredPassMesh(RenderPass);
				break;
			}
		}
		else if constexpr (std::is_same<RPType,ToonRenderPass>::value) {
			switch (type) {
			case RENDER_OBJECT_TYPE_MODEL:
				RenderByToonPass(RenderPass);
				break;
			case RENDER_OBJECT_TYPE_MESH:
				RenderByToonPassMesh(RenderPass);
				break;
			}
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
	enum RENDER_OBJECT_TYPE {
		RENDER_OBJECT_TYPE_MODEL,
		RENDER_OBJECT_TYPE_MESH
	} type;

	void RenderByPhongPass(PhongRenderPass* rp);
	void RenderByPhongPassMesh(PhongRenderPass* rp);
	void RenderByDeferredPass(DeferredRenderPass* drp);
	void RenderByDeferredPassMesh(DeferredRenderPass* drp);
	void RenderByToonPass(ToonRenderPass* trp);
	void RenderByToonPassMesh(ToonRenderPass* trp);

	union {
		struct {
			Mesh mMesh;
			SubMeshMaterial mMaterial;
		};
		Model* model;
	};
	
	std::string name;
	Game::Vector3 worldPosition;
	Game::Vector3 worldRotation;
	Game::Vector3 worldScale;

	struct {
		bool initialized = false;
		std::vector<PhongObjectID> phongObjectID;
		std::vector<PhongMaterialTexture> phongMaterialTextures;
	} phongRPData;
	struct {
		bool initialized = false;
		std::vector<DeferredRenderPassID> deferredObjectID;
		std::vector<DeferredRenderPassTexture> deferredTextures;
	} deferredRPData;
	struct {
		bool initialized = false;
		std::vector<ToonRenderObjectID> toonObjectID;
		std::vector<ToonPassMaterial> toonMaterials;
	} toonRPData;

	static size_t renderItemIndex;
};

class SkinnedRenderObject {
public:
	SkinnedRenderObject(SkinnedModel* model, Game::Vector3 worldPosition,
		Game::Vector3 worldRotation, Game::Vector3 worldScale,
		const char* name = nullptr);
	
	template<typename RP>
	void Render(RP* rp) {
		if constexpr (std::is_same<RP,DeferredRenderPass>::value) {
			RenderByDeferredPass(rp);
		}
	}

	void Interpolate(float time,const char* boneAnimationName);
	

private:
	void RenderByDeferredPass(DeferredRenderPass* drp);

	std::string name;
	Game::Vector3 worldPosition;
	Game::Vector3 worldRotation;
	Game::Vector3 worldScale;

	std::unique_ptr<ConstantBuffer<Game::Mat4x4>> boneTransformBuffer;

	SkinnedModel* model;
	struct {
		bool initialized;
		std::vector<DeferredRenderPassID> drpID;
		std::vector<DeferredRenderPassTexture> drpMaterial;
	} DeferredRPData;
};