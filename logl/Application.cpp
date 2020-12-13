#include "Application.h"
#include "../loglcore/graphic.h"
#include "../loglcore/CopyBatch.h"
#include "../loglcore/Mesh.h"
#include "../loglcore/Shader.h"
#include "../loglcore/GeometryGenerator.h"
#include "../loglcore/ConstantBuffer.h"
#include "../loglcore/TextureManager.h"
#include "../loglcore/DescriptorAllocator.h"

#include "../loglcore/SpriteRenderPass.h"
#include "../loglcore/PhongRenderPass.h"

#include "InputBuffer.h"
#include "Timer.h"

SpriteRenderPass* srp;
SpriteID spriteID;

SpriteData data[100];

PhongRenderPass* prp;
PhongObjectID    pid;
std::unique_ptr<DynamicMesh> pMesh;

void upload(){
	for (int x = 0; x != 10; x++) {
		for (int y = 0; y != 10; y++) {
			data[x + y * 10].Color = Game::Vector4(x / 10., y / 10., 1., .5);
			data[x + y * 10].Scale = Game::Vector2(.1, .1);
			data[x + y * 10].Position = Game::Vector3((x - 5)/ 11., (y - 5)/ 11., .05);
		}
	}
}

bool Application::initialize() {
	upload();
	srp = gGraphic.GetRenderPass<SpriteRenderPass>();
	if (srp == nullptr) {
		return false;
	}

	Texture* tex = gTextureManager.loadTexture(L"../asserts/awesomeface.png", L"face");
	spriteID = srp->RegisterTexture(tex);

	prp = gGraphic.GetRenderPass<PhongRenderPass>();
	if (prp == nullptr) {
		return false;
	}

	pid = prp->AllocateObjectPass();
	std::vector<float> varray = GeometryGenerator::Cube(.5, .5, .5);
	pMesh = std::make_unique<DynamicMesh>(varray.size() * sizeof(float) / sizeof(MeshVertex),reinterpret_cast<MeshVertex*>(varray.data()));

	auto objPass = prp->GetObjectPass(pid);
	objPass->world = Game::Mat4x4::I();
	objPass->transInvWorld = Game::Mat4x4::I();
	objPass->material.diffuse = Game::ConstColor::White;
	objPass->material.FresnelR0 = Game::Vector3(.1, .1, .1);
	objPass->material.Roughness = .4;
	
	prp->BindAmbientLightData(Game::Vector3(.5, .5, .5));
	
	LightData lData;
	lData.type = SHADER_LIGHT_TYPE_DIRECTIONAL;
	lData.direction = Game::normalize(Game::Vector3(-1.,-1.,0.));
	lData.intensity = Game::Vector3(1., 1., 1.);
	lData.fallEnd = 10.f;
	prp->BindLightData(&lData);

	return true;
}

float r = 0.;
void Application::update() {
	r += 90. * gTimer.DeltaTime();

	for (int x = 0; x != 10; x++) {
		for (int y = 0; y != 10; y++) {
			float r1 = r * sinf(x * 17 + y * 7) * 2.;
			data[x + y * 10].rotation = r1;
		}
	}

	srp->DrawSprite(50, data, spriteID);
	srp->DrawSpriteTransparent(50, data + 50, spriteID);

	prp->DrawObject(pMesh->GetVBV(), 0, pMesh->GetVertexNum(), pid);
}

void Application::finalize() {}