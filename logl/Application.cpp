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
#include "FPSCamera.h"

std::unique_ptr<StaticMesh<MeshVertexNormal>> box;

struct BoxConstantBuffer {
	Game::Mat4x4 world;
	Game::Mat4x4 transInvWorld;
};


SpriteData data[100];
SpriteRenderPass* srp;
SpriteID spid;

PhongObjectID pid[3];
PhongRenderPass* prp;

PhongMaterialTexture mat;

FPSCamera fpsCamera;

void upload(){
	//auto[vertices, indices] = GeometryGenerator::Plane(5., 5., 64, 64);
	auto vertices = GeometryGenerator::Cube(1., 1., 1.);

	box = std::make_unique<StaticMesh<MeshVertexNormal>>(
		gGraphic.GetDevice(),
		//indices.size(), indices.data(),
		vertices.size() / getVertexStrideByFloat<MeshVertexNormal>(), reinterpret_cast<MeshVertexNormal*>(vertices.data())
	);


	prp = gGraphic.GetRenderPass<PhongRenderPass>();
	pid[0] = prp->AllocateObjectPass();
	pid[1] = prp->AllocateObjectPass();
	pid[2] = prp->AllocateObjectPass();

	ObjectPass* objPass = prp->GetObjectPass(pid[0]);
	objPass->material.diffuse = Game::Vector4(1.,1.,1.,1.);
	objPass->material.FresnelR0 = Game::Vector4(.1, .1, .1);
	objPass->material.Roughness = .8;

	objPass = prp->GetObjectPass(pid[1]);
	objPass->material.diffuse = Game::Vector4(1., .75, .75, 1.);
	objPass->material.FresnelR0 = Game::Vector4(.1, .1, .1);
	objPass->material.Roughness = .8;

	objPass = prp->GetObjectPass(pid[2]);
	objPass->material.diffuse = Game::Vector4(1., .75, .75, 1.);
	objPass->material.FresnelR0 = Game::Vector4(.1, .1, .1);
	objPass->material.Roughness = .8;

	prp->BindAmbientLightData(Game::Vector3(.1, .1, .1));
	LightData light;
	light.fallStart = 0.;
	light.fallEnd = 20.;
	light.intensity = Game::Vector3(1.,1.,1.);
	light.direction = Game::normalize(Game::Vector3(0., 0.,1.));
	light.type = SHADER_LIGHT_TYPE_DIRECTIONAL;
	prp->BindLightData(&light);

	srp = gGraphic.GetRenderPass<SpriteRenderPass>();
}

bool Application::initialize() {
	upload();
	
	Texture* box = gTextureManager.loadTexture(L"../asserts/brickwall.jpg", L"brickwall");
	box->CreateShaderResourceView(Descriptor(gDescriptorAllocator.AllocateDescriptor()));
	Texture* boxNormal = gTextureManager.loadTexture(L"../asserts/brickwall_normal.jpg", L"brickwall_normal");
	boxNormal->CreateShaderResourceView(Descriptor(gDescriptorAllocator.AllocateDescriptor()));
	mat.diffuseMap = box;
	mat.normalMap = boxNormal;

	fpsCamera.attach(gGraphic.GetMainCamera());

	Texture* face = gTextureManager.loadTexture(L"../asserts/awesomeface.png",L"face");
	spid = srp->RegisterTexture(face);
	
	data[0].Color = Game::Vector4(1.,1.,1.,.5);
	data[0].Position = Game::Vector3(0., 0., .5);
	data[0].rotation = 0.;
	data[0].Scale = Game::Vector2(.3,.3);

	return true;
}
float r = 0.,b = 0.;
Game::Vector2 pos;
void Application::update() {
	if (gInput.KeyHold(InputBuffer::I)) {
		r += gTimer.DeltaTime() * 90.;
	}
	else if (gInput.KeyHold(InputBuffer::K)) {
		r -= gTimer.DeltaTime() * 90.;
	}
	if (gInput.KeyHold(InputBuffer::J)) {
		b -= gTimer.DeltaTime() * 90.;
	}
	else if (gInput.KeyHold(InputBuffer::L)) {
		b += gTimer.DeltaTime() * 90.;
	}

	if (gInput.KeyHold(InputBuffer::D)) {
		fpsCamera.strafe(gTimer.DeltaTime() * 2.);
	}
	else if (gInput.KeyHold(InputBuffer::A)) {
		fpsCamera.strafe(-gTimer.DeltaTime() * 2.);
	}
	if (gInput.KeyHold(InputBuffer::W)) {
		fpsCamera.walk(gTimer.DeltaTime() * 2.);
	}
	else if (gInput.KeyHold(InputBuffer::S)) {
		fpsCamera.walk(-gTimer.DeltaTime() * 2.);
	}

	if (gInput.KeyDown(InputBuffer::MOUSE_LEFT)) {
		pos = gInput.MousePosition();
	}else if (gInput.KeyHold(InputBuffer::MOUSE_LEFT)) {
		Game::Vector2 deltaPos = gInput.MousePosition() - pos;
		float speed = 10.;
		fpsCamera.rotateX(deltaPos.x * gTimer.DeltaTime() * speed);
		fpsCamera.rotateY(deltaPos.y * gTimer.DeltaTime() * speed);
		pos = gInput.MousePosition();
	}
	
	ObjectPass* objPass = prp->GetObjectPass(pid[0]);
	objPass->world = Game::PackTransfrom(Game::Vector3(-3., 0.,10.), Game::Vector3(r, b, 0.), Game::Vector3(1., 1., 1.));
	objPass->transInvWorld = objPass->world.R();
	objPass->world = objPass->world.T();

	objPass = prp->GetObjectPass(pid[1]);
	objPass->world = Game::PackTransfrom(Game::Vector3(3., 0., 10.), Game::Vector3(-r, -b, 0.), Game::Vector3(1., 1., 1.));
	objPass->transInvWorld = objPass->world.R();
	objPass->world = objPass->world.T();

	objPass = prp->GetObjectPass(pid[2]);
	objPass->world = Game::PackTransfrom(Game::Vector3(0., 0., 10.), Game::Vector3(-r, -b, 0.), Game::Vector3(1., 1., 1.));
	objPass->transInvWorld = objPass->world.R();
	objPass->world = objPass->world.T();


	prp->DrawObject(box->GetVBV(), 0, box->GetVertexNum(), pid[0], &mat);
	prp->DrawObject(box->GetVBV(), 0, box->GetVertexNum(), pid[1], &mat);
	prp->DrawObject(box->GetVBV(), 0, box->GetVertexNum(), pid[2]);

	srp->DrawSpriteTransparent(1, data, spid);
}

void Application::finalize() {
	box.release();
}