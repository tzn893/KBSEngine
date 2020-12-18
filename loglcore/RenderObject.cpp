#include "RenderObject.h"
#include "graphic.h"


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
			objPass->material.SetMaterialTransform(Game::Vector2(0., 0.), Game::Vector2(1., 1.));

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

RenderObject::~RenderObject() {
	if (phongRPData.initialized) {
		PhongRenderPass* phongRP = gGraphic.GetRenderPass<PhongRenderPass>();
		for (auto& i : phongRPData.phongObjectID) {
			phongRP->DeallocateObjectPass(i);
		}
	}
}