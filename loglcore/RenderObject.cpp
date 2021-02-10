#include "RenderObject.h"
#include "graphic.h"

size_t RenderObject::renderItemIndex = 0;

void RenderObject::RenderByPhongPass(PhongRenderPass* RenderPass) {
	Game::Mat4x4 world = Game::PackTransfrom(worldPosition, worldRotation, worldScale);
	Game::Mat4x4 transInvWorld = world.R();
	world = world.T();

	if (!phongRPData.initialized) {
		size_t objnum = model->GetSubMeshNum();
		phongRPData.phongObjectID.resize(objnum);
		phongRPData.phongMaterialTextures.resize(model->GetSubMaterialNum());

		model->ForEachSubMesh([&](SubMesh* mesh,Model* target,size_t index){
			PhongObjectID id = RenderPass->AllocateObjectPass();
			phongRPData.phongObjectID[index] = id;
			ObjectPass* objPass = RenderPass->GetObjectPass(id);
			objPass->world = world;
			objPass->transInvWorld = transInvWorld;

			size_t materialIndex = mesh->GetSubMeshMaterialIndex();
			SubMeshMaterial* material = target->GetMaterial(mesh);
			objPass->material.diffuse = Game::Vector4(material->diffuse,1.f);
			objPass->material.FresnelR0 = material->specular;
			objPass->material.Roughness = material->roughness;
			objPass->material.SetMaterialTransform(material->matTransformOffset,material->matTransformScale);

			phongRPData.phongMaterialTextures[materialIndex].diffuseMap = material->textures[SUBMESH_MATERIAL_TYPE_DIFFUSE];
			phongRPData.phongMaterialTextures[materialIndex].normalMap = material->textures[SUBMESH_MATERIAL_TYPE_BUMP];
		});
		phongRPData.initialized = true;
	}
	else {
		model->ForEachSubMesh([&](SubMesh* mesh, Model* target, size_t index) {
			ObjectPass* objPass = RenderPass->GetObjectPass(phongRPData.phongObjectID[index]);
			objPass->world = world;
			objPass->transInvWorld = transInvWorld;

			size_t materialIndex = mesh->GetSubMeshMaterialIndex();
			
			if (mesh->GetIBV() == nullptr) {
				RenderPass->DrawObject(mesh->GetVBV(), 0, mesh->GetVertexNum(), phongRPData.phongObjectID[index],
					&phongRPData.phongMaterialTextures[materialIndex]);
			}
			else{
				RenderPass->DrawObject(mesh->GetVBV(),mesh->GetIBV(), 0, mesh->GetIndexNum(), phongRPData.phongObjectID[index],
					&phongRPData.phongMaterialTextures[materialIndex]);
			}
		});
	}

}

void RenderObject::RenderByPhongPassMesh(PhongRenderPass* RenderPass) {
	Game::Mat4x4 world = Game::PackTransfrom(worldPosition, worldRotation, worldScale);
	Game::Mat4x4 transInvWorld = world.R();
	world = world.T();

	if (!phongRPData.initialized) {
		phongRPData.phongObjectID.resize(1);
		phongRPData.phongMaterialTextures.resize(1);

		size_t id = RenderPass->AllocateObjectPass();
		phongRPData.phongObjectID[0] = id;
		ObjectPass* objPass = RenderPass->GetObjectPass(id);
		objPass->world = world;
		objPass->transInvWorld = world;

		SubMeshMaterial* material = &this->mMaterial;
		objPass->material.diffuse = Game::Vector4(material->diffuse, 1.f);
		objPass->material.FresnelR0 = material->specular;
		objPass->material.Roughness = material->roughness;
		objPass->material.SetMaterialTransform(material->matTransformOffset, material->matTransformScale);

		phongRPData.phongMaterialTextures[0].diffuseMap = material->textures[SUBMESH_MATERIAL_TYPE_DIFFUSE];
		phongRPData.phongMaterialTextures[0].normalMap = material->textures[SUBMESH_MATERIAL_TYPE_BUMP];

		phongRPData.initialized = true;
	}
	else {
		ObjectPass* objPass = RenderPass->GetObjectPass(phongRPData.phongObjectID[0]);
		objPass->world = world;
		objPass->transInvWorld = transInvWorld;

		if (mMesh.ibv == nullptr) {
			RenderPass->DrawObject(mMesh.vbv, mMesh.startIndex, mMesh.indiceNum, phongRPData.phongObjectID[0],
				&phongRPData.phongMaterialTextures[0]);
		}
		else {
			RenderPass->DrawObject(mMesh.vbv, mMesh.ibv, 0, mMesh.indiceNum, phongRPData.phongObjectID[0],
				&phongRPData.phongMaterialTextures[0]);
		}
	}
}

void RenderObject::RenderByDeferredPass(DeferredRenderPass* RP) {
	Game::Mat4x4 world = Game::PackTransfrom(worldPosition, worldRotation, worldScale);
	Game::Mat4x4 transInvWorld = world.R();
	world = world.T();
	if (!deferredRPData.initialized) {
		size_t objnum = model->GetSubMeshNum();
		deferredRPData.deferredObjectID.resize(objnum);
		deferredRPData.deferredTextures.resize(model->GetSubMaterialNum());
		model->ForEachSubMesh([&](SubMesh* mesh, Model* target, size_t index) {
			DeferredRenderPassID id = RP->AllocateObjectPass();
			deferredRPData.deferredObjectID[index] = id;
			ObjectPass* objPass = RP->GetObjectPass(id);
			objPass->world = world;
			objPass->transInvWorld = transInvWorld;

			size_t materialIndex = mesh->GetSubMeshMaterialIndex();
			SubMeshMaterial* material = target->GetMaterial(mesh);
			objPass->material.diffuse = Game::Vector4(material->diffuse, 1.f);
			objPass->material.FresnelR0 = material->specular;
			objPass->material.Roughness = material->roughness;
			objPass->material.Metallic = material->metallic;
			objPass->material.SetMaterialTransform(material->matTransformOffset, material->matTransformScale);
			objPass->material.emission = material->emissionScale;

			deferredRPData.deferredTextures[materialIndex].diffuse = material->textures[SUBMESH_MATERIAL_TYPE_DIFFUSE];
			deferredRPData.deferredTextures[materialIndex].normal = material->textures[SUBMESH_MATERIAL_TYPE_BUMP];
			deferredRPData.deferredTextures[materialIndex].metallic = material->textures[SUBMESH_MATERIAL_TYPE_METALNESS];
			deferredRPData.deferredTextures[materialIndex].roughness = material->textures[SUBEMSH_MATERIAL_TYPE_ROUGHNESS];
			deferredRPData.deferredTextures[materialIndex].emission = material->textures[SUBMESH_MATERIAL_TYPE_EMISSION];
		});
		deferredRPData.initialized = true;
	}
	else {
		model->ForEachSubMesh(
			[&](SubMesh* mesh, Model* target, size_t index) {
				ObjectPass* op = RP->GetObjectPass(deferredRPData.deferredObjectID[index]);
				op->world = world;
				op->transInvWorld = transInvWorld;

				size_t materialIndex = mesh->GetSubMeshMaterialIndex();

				if (mesh->GetIBV() != nullptr) {
					RP->DrawObject(mesh->GetVBV(),mesh->GetIBV(),0,
						mesh->GetIndexNum(),
						deferredRPData.deferredObjectID[index],
						&deferredRPData.deferredTextures[mesh->GetSubMeshMaterialIndex()]
					);
				}
				else {
					RP->DrawObject(mesh->GetVBV(), 0,
						mesh->GetVertexNum(),
						deferredRPData.deferredObjectID[index],
						&deferredRPData.deferredTextures[mesh->GetSubMeshMaterialIndex()]
					);
				}
			}
		);
	}
}

void RenderObject::RenderByDeferredPassMesh(DeferredRenderPass* RP) {
	Game::Mat4x4 world = Game::PackTransfrom(worldPosition, worldRotation, worldScale);
	Game::Mat4x4 transInvWorld = world.R();
	world = world.T();
	if (!deferredRPData.initialized) {
		size_t objnum = model->GetSubMeshNum();
		deferredRPData.deferredObjectID.resize(1);
		deferredRPData.deferredTextures.resize(1);
		
		size_t id = RP->AllocateObjectPass();
		deferredRPData.deferredObjectID[0] = id;
		ObjectPass* objPass = RP->GetObjectPass(id);
		objPass->world = world;
		objPass->transInvWorld = world;

		SubMeshMaterial* material = &this->mMaterial;
		objPass->material.diffuse = Game::Vector4(material->diffuse, 1.f);
		objPass->material.FresnelR0 = material->specular;
		objPass->material.Roughness = material->roughness;
		objPass->material.Metallic = material->metallic;
		objPass->material.SetMaterialTransform(material->matTransformOffset, material->matTransformScale);
		objPass->material.emission = material->emissionScale;

		deferredRPData.deferredTextures[0].diffuse = material->textures[SUBMESH_MATERIAL_TYPE_DIFFUSE];
		deferredRPData.deferredTextures[0].normal = material->textures[SUBMESH_MATERIAL_TYPE_BUMP];
		deferredRPData.deferredTextures[0].metallic = material->textures[SUBMESH_MATERIAL_TYPE_METALNESS];
		deferredRPData.deferredTextures[0].roughness = material->textures[SUBEMSH_MATERIAL_TYPE_ROUGHNESS];
		deferredRPData.deferredTextures[0].emission = material->textures[SUBMESH_MATERIAL_TYPE_EMISSION];

		deferredRPData.initialized = true;
	}
	else {
		ObjectPass* objPass = RP->GetObjectPass(deferredRPData.deferredObjectID[0]);
		objPass->world = world;
		objPass->transInvWorld = transInvWorld;

		if (mMesh.ibv == nullptr) {
			RP->DrawObject(mMesh.vbv, mMesh.startIndex, mMesh.indiceNum, deferredRPData.deferredObjectID[0],
				&deferredRPData.deferredTextures[0]);
		}
		else {
			RP->DrawObject(mMesh.vbv, mMesh.ibv, 0, mMesh.indiceNum, deferredRPData.deferredObjectID[0],
				&deferredRPData.deferredTextures[0]);
		}
	}
}

void RenderObject::RenderByToonPass(ToonRenderPass* trp) {
	Game::Mat4x4 world = Game::PackTransfrom(worldPosition, worldRotation, worldScale);
	Game::Mat4x4 transInvWorld = world.R();
	world = world.T();
	if (!toonRPData.initialized) {
		size_t objnum = model->GetSubMeshNum();
		toonRPData.toonObjectID.resize(objnum);
		toonRPData.toonMaterials.resize(model->GetSubMaterialNum());
		model->ForEachSubMesh([&](SubMesh* mesh, Model* target, size_t index) {
			ToonRenderObjectID id = trp->AllocateToonROID();
			toonRPData.toonObjectID[index] = id;
			ObjectPass* objPass = trp->GetObjectPass(id);
			objPass->world = world;
			objPass->transInvWorld = transInvWorld;

			size_t materialIndex = mesh->GetSubMeshMaterialIndex();
			SubMeshMaterial* material = target->GetMaterial(mesh);
			objPass->material.diffuse = Game::Vector4(material->diffuse, 1.f);
			objPass->material.FresnelR0 = material->specular;
			objPass->material.Roughness = material->roughness;
			objPass->material.Metallic = material->metallic;
			objPass->material.SetMaterialTransform(material->matTransformOffset, material->matTransformScale);
			objPass->material.emission = material->emissionScale;

			toonRPData.toonMaterials[materialIndex].diffuse = material->textures[SUBMESH_MATERIAL_TYPE_DIFFUSE];
		});
		toonRPData.initialized = true;
	}
	else {
		model->ForEachSubMesh(
			[&](SubMesh* mesh, Model* target, size_t index) {
			ObjectPass* op = trp->GetObjectPass(toonRPData.toonObjectID[index]);
			op->world = world;
			op->transInvWorld = transInvWorld;

			size_t materialIndex = mesh->GetSubMeshMaterialIndex();

			if (mesh->GetIBV() != nullptr) {
				trp->Draw(mesh->GetVBV(), mesh->GetIBV(), 0,
					mesh->GetIndexNum(),
					toonRPData.toonObjectID[index],
					&toonRPData.toonMaterials[mesh->GetSubMeshMaterialIndex()]
				);
			}
			else {
				trp->Draw(mesh->GetVBV(), 0,
					mesh->GetVertexNum(),
					toonRPData.toonObjectID[index],
					&toonRPData.toonMaterials[mesh->GetSubMeshMaterialIndex()]
				);
			}
		}
		);
	}
}

void RenderObject::RenderByToonPassMesh(ToonRenderPass* trp) {
	Game::Mat4x4 world = Game::PackTransfrom(worldPosition, worldRotation, worldScale);
	Game::Mat4x4 transInvWorld = world.R();
	world = world.T();
	if (!toonRPData.initialized) {
		size_t objnum = model->GetSubMeshNum();
		toonRPData.toonObjectID.resize(1);
		toonRPData.toonMaterials.resize(1);

		size_t id = trp->AllocateToonROID();
		toonRPData.toonObjectID[0] = id;
		ObjectPass* objPass = trp->GetObjectPass(id);
		objPass->world = world;
		objPass->transInvWorld = world;

		SubMeshMaterial* material = &this->mMaterial;
		objPass->material.diffuse = Game::Vector4(material->diffuse, 1.f);
		objPass->material.FresnelR0 = material->specular;
		objPass->material.Roughness = material->roughness;
		objPass->material.Metallic = material->metallic;
		objPass->material.SetMaterialTransform(material->matTransformOffset, material->matTransformScale);
		objPass->material.emission = material->emissionScale;

		toonRPData.toonMaterials[0].diffuse = material->textures[SUBMESH_MATERIAL_TYPE_DIFFUSE];

		toonRPData.initialized = true;
	}
	else {
		ObjectPass* objPass = trp->GetObjectPass(toonRPData.toonObjectID[0]);
		objPass->world = world;
		objPass->transInvWorld = transInvWorld;

		if (mMesh.ibv == nullptr) {
			trp->Draw(mMesh.vbv, mMesh.startIndex, mMesh.indiceNum, toonRPData.toonObjectID[0],
				&toonRPData.toonMaterials[0]);
		}
		else {
			trp->Draw(mMesh.vbv, mMesh.ibv, 0, mMesh.indiceNum, toonRPData.toonObjectID[0],
				&toonRPData.toonMaterials[0]);
		}
	}
}

RenderObject::~RenderObject() {
	if (phongRPData.initialized) {
		PhongRenderPass* phongRP = gGraphic.GetRenderPass<PhongRenderPass>();
		for (auto& i : phongRPData.phongObjectID) {
			phongRP->DeallocateObjectPass(i);
		}
	}
	if (deferredRPData.initialized) {
		auto drp = gGraphic.GetRenderPass<DeferredRenderPass>();
		for (auto& i : deferredRPData.deferredObjectID) {
			drp->DeallocateObjectPass(i);
		}
	}
	if (toonRPData.initialized) {
		auto trp = gGraphic.GetRenderPass<ToonRenderPass>();
		for (auto& i : toonRPData.toonObjectID) {
			trp->DeallocateToonROID(i);
		}
	}
}