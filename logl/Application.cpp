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

#include "../loglcore/ModelManager.h"

#include "../loglcore/RenderObject.h"

#include "InputBuffer.h"
#include "Timer.h"
#include "FPSCamera.h"
/*
std::unique_ptr<StaticMesh<MeshVertexNormal>> box;
std::unique_ptr<StaticMesh<MeshVertexNormal>> plane;

struct BoxConstantBuffer {

	Game::Mat4x4 world;
	Game::Mat4x4 transInvWorld;
};

SpriteData data[100];
SpriteRenderPass* srp;
Texture* face;

PhongObjectID pid[3];
PhongRenderPass* prp;

std::unique_ptr<RenderObject> ro;

PhongMaterialTexture mat;
FPSCamera fpsCamera;


void upload(){
	auto vertices = GeometryGenerator::Cube(.5, .5, .5);

	box = std::make_unique<StaticMesh<MeshVertexNormal>>(
		gGraphic.GetDevice(),
		//indices.size(), indices.data(),
		vertices.size() / getVertexStrideByFloat<MeshVertexNormal>(), reinterpret_cast<MeshVertexNormal*>(vertices.data())
	);


	auto[planeV, planeI] = GeometryGenerator::Square(20., 20.);
	plane = std::make_unique<StaticMesh<MeshVertexNormal>>(
		gGraphic.GetDevice(),
		planeI.size(), planeI.data(),
		planeV.size() / getVertexStrideByFloat<MeshVertexNormal>(), reinterpret_cast<MeshVertexNormal*>(planeV.data())
		);

	prp = gGraphic.GetRenderPass<PhongRenderPass>();
	pid[0] = prp->AllocateObjectPass();
	pid[1] = prp->AllocateObjectPass();
	pid[2] = prp->AllocateObjectPass();

	ObjectPass* objPass = prp->GetObjectPass(pid[0]);
	objPass->material.diffuse = Game::Vector4(1.,1.,1.,1.);
	objPass->material.FresnelR0 = Game::Vector4(.1, .1, .1);
	objPass->material.Roughness = .8;
	objPass->material.SetMaterialTransform(Game::Vector2(0., 0.), Game::Vector2(.5, .5));

	objPass = prp->GetObjectPass(pid[1]);
	objPass->material.diffuse = Game::Vector4(1., .75, .75, 1.);
	objPass->material.FresnelR0 = Game::Vector4(.1, .1, .1);
	objPass->material.Roughness = .8;
	objPass->material.SetMaterialTransform(Game::Vector2(0., 0.), Game::Vector2(.5, .5));

	objPass = prp->GetObjectPass(pid[2]);
	objPass->material.diffuse = Game::Vector4(1., .75, .75, 1.);
	objPass->material.FresnelR0 = Game::Vector4(.1, .1, .1);
	objPass->material.Roughness = .8;
	objPass->material.SetMaterialTransform(Game::Vector2(0., 0.), Game::Vector2(5., 5.));
	objPass->world = Game::PackTransfrom(Game::Vector3(0., -2., 10.), Game::Vector3(0., 0., 0.), Game::Vector3(1., 1., 1.));
	objPass->transInvWorld = objPass->world.R();
	objPass->world = objPass->world.T();

	prp->BindAmbientLightData(Game::Vector3(.6, .6, .6));
	LightData light;
	light.fallStart = 0.;
	light.fallEnd = 20.;
	light.intensity = Game::Vector3(1.,1.,1.);
	light.direction = Game::normalize(Game::Vector3(0., 0.,1.));
	light.type = SHADER_LIGHT_TYPE_DIRECTIONAL;
	prp->BindLightData(&light);

}

bool Application::initialize() {
	upload();
	srp = gGraphic.GetRenderPass<SpriteRenderPass>();
	
	UploadBatch up = UploadBatch::Begin();
	Texture* box = gTextureManager.loadTexture(L"../asserts/brickwall.jpg", L"brickwall",true,&up);
	box->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	Texture* boxNormal = gTextureManager.loadTexture(L"../asserts/brickwall_normal.jpg", L"brickwall_normal",true, &up);
	boxNormal->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	Model* model = gModelManager.loadModel("../asserts/suit/nanosuit.obj", "suit",&up);
	ro = std::make_unique<RenderObject>(model, Game::Vector3(0., -1., 5.), Game::Vector3(0., 180., 0.), Game::Vector3(.1, .1, .1));
	face = gTextureManager.loadTexture(L"../asserts/awesomeface.png", L"face",true,&up);
	face->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	up.End();

	mat.diffuseMap = box;
	mat.normalMap = boxNormal;

	fpsCamera.attach(gGraphic.GetMainCamera());

	data[0].Color = Game::Vector4(1.,1.,1.,.5);
	data[0].Position = Game::Vector3(0., 0., .5);
	data[0].rotation = 0.;
	data[0].Scale = Game::Vector2(.3,.3);
	
	
	return true;
}
float r = 0.,b = 0.,g = 0.;
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

	if (gInput.KeyHold(InputBuffer::Q)) {
		g += 4e-3;
	}
	else if (gInput.KeyHold(InputBuffer::E)) {
		g -= 4e-3;
	}

	LightData light;
	light.intensity = Game::Vector3(1., 1., 1.);
	light.direction = Game::normalize(Game::Vector3(0., sinf(g), cosf(g)));
	light.type = SHADER_LIGHT_TYPE_DIRECTIONAL;
	prp->BindLightData(&light);
	
	ObjectPass* objPass = prp->GetObjectPass(pid[0]);
	objPass->world = Game::PackTransfrom(Game::Vector3(3., 0.,10.), Game::Vector3(r, b, 0.), Game::Vector3(1., 1., 1.));
	objPass->transInvWorld = objPass->world.R();
	objPass->world = objPass->world.T();

	objPass = prp->GetObjectPass(pid[1]);
	objPass->world = Game::PackTransfrom(Game::Vector3(0., -1., 10.), Game::Vector3(r, b, 0.), Game::Vector3(1., 2., 1.));
	objPass->transInvWorld = objPass->world.R();
	objPass->world = objPass->world.T();

	prp->DrawObject(box->GetVBV(), 0, box->GetVertexNum(), pid[0], &mat);
	prp->DrawObject(box->GetVBV(), 0, box->GetVertexNum(), pid[1], &mat);
	prp->DrawObject(plane->GetVBV(), plane->GetIBV(), 0, plane->GetIndexNum(), pid[2],&mat);
	
	srp->DrawSpriteTransparent(1, data, face);
	ro->SetWorldRotation(Game::Vector3(0., r, 0.));
	ro->Render(prp);
}

void Application::finalize() {
	box.release();
}
*/

#include "FFTWave.h"

FPSCamera fpsCamera;
Game::Vector2 pos;
FFTWave wave;

bool Application::initialize() {
	fpsCamera.attach(gGraphic.GetMainCamera());
	
	if (!wave.Initialize(96., 80.)) {
		return false;
	}

	LightData light;
	light.fallStart = 0.;
	light.fallEnd = 20.;
	light.intensity = Game::Vector3(1., 1., 1.);
	light.direction = Game::normalize(Game::Vector3(0., 0., 1.));
	light.type = SHADER_LIGHT_TYPE_DIRECTIONAL;

	auto waverp = wave.GetRenderPass();
	waverp->GetLightPass()->lights[0] = light;
	waverp->GetLightPass()->ambient = Game::Vector4(.6, .6, .6, 1.);
	wave.SetPosition(Game::Vector3(0., -1., 10.));
	
	float color[] = {1.,1.,1.,1.};
	gGraphic.SetDefaultClearColor(color);

	return true;
}

void Application::update() {
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
	}
	else if (gInput.KeyHold(InputBuffer::MOUSE_LEFT)) {
		Game::Vector2 deltaPos = gInput.MousePosition() - pos;
		float speed = 5.;
		fpsCamera.rotateX(deltaPos.x * gTimer.DeltaTime() * speed);
		fpsCamera.rotateY(deltaPos.y * gTimer.DeltaTime() * speed);
		pos = gInput.MousePosition();
	}

	wave.Update(gTimer.DeltaTime());
}

void Application::finalize() {

}
