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
#include "../loglcore/LightManager.h"

#include "InputBuffer.h"
#include "Timer.h"
#include "FPSCamera.h"
#include "FFTWave.h"

#include "Player.h"

struct BoxConstantBuffer {

	Game::Mat4x4 world;
	Game::Mat4x4 transInvWorld;
};

SpriteData data[100];
SpriteRenderPass* srp;
Texture* face;

PhongRenderPass* prp;

std::unique_ptr<RenderObject> rp;
std::unique_ptr<Player> player;

FFTWave wave;
extern bool Quit;

void upload(){

	gLightManager.SetAmbientLight(Game::Vector3(.6, .6, .6));
	LightData light;
	light.fallStart = 0.;
	light.fallEnd = 20.;
	light.intensity = Game::Vector3(1.,1.,1.);
	light.direction = Game::normalize(Game::Vector3(0., 0.,1.));
	light.type = SHADER_LIGHT_TYPE_DIRECTIONAL;
	gLightManager.SetMainLightData(light);

}

bool Application::initialize() {
	upload();
	if (!wave.Initialize(512,512)) {
		return false;
	}
	srp = gGraphic.GetRenderPass<SpriteRenderPass>();
	prp = gGraphic.GetRenderPass<PhongRenderPass>();
	
	
	UploadBatch up = UploadBatch::Begin();
	face = gTextureManager.loadTexture(L"../asserts/awesomeface.png", L"face",true,&up);
	face->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	Model* plane = gModelManager.loadModel("../asserts/spaceship/spaceship.obj", "plane",&up);
	rp = std::make_unique<RenderObject>(plane, Game::Vector3(0.,20.,5.),Game::Vector3(0.,0.,0.),Game::Vector3(.1,.1,.1));
	up.End();

	player = std::make_unique<Player>(Game::Vector3(0., 20., 3.), Game::Vector3(.1, .1, .1),plane);

	//fpsCamera.attach(gGraphic.GetMainCamera());
	//fpsCamera.setPosition(Game::Vector3(0., 20., 0.));

	data[0].Color = Game::Vector4(1.,1.,1.,.5);
	data[0].Position = Game::Vector3(0., 0., .5);
	data[0].rotation = 0.;
	data[0].Scale = Game::Vector2(.3,.3);
	
	return true;
}
void Application::update() {
	
	/*if (gInput.KeyHold(InputBuffer::I)) {
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
	}*/
	/*
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
	}*/

	LightData light;
	light.intensity = Game::Vector3(1., 1., 1.);
	light.direction = Game::normalize(Game::Vector3(0., -1., 1.));
	light.type = SHADER_LIGHT_TYPE_DIRECTIONAL;
	gLightManager.SetMainLightData(light);
	
	srp->DrawSpriteTransparent(1, data, face);
	rp->Render(prp);
	wave.Update(gTimer.DeltaTime());

	player->Update();
}

void Application::finalize() {
}

void Application::Quit() {
	
}