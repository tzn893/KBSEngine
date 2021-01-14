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
#include "logl.h"

struct BoxConstantBuffer {

	Game::Mat4x4 world;
	Game::Mat4x4 transInvWorld;
};

SpriteData sdata[100];
SpriteRenderPass* srp;
Texture* face;

//PhongRenderPass* prp;
DeferredRenderPass* drp;

std::unique_ptr<RenderObject> ro,cro;

std::vector<Game::Vector3> bullets;
FPSCamera camera;

#include "Config.h"

bool Application::initialize() {
	
	srp = gGraphic.GetRenderPass<SpriteRenderPass>();
	drp = gGraphic.GetRenderPass<DeferredRenderPass>();
	
	UploadBatch up = UploadBatch::Begin();
	face = gTextureManager.loadTexture(L"../asserts/awesomeface.png", L"face",true,&up);
	face->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	Model* plane = gModelManager.loadModel("../asserts/spaceship/spaceship.obj", "plane",&up);
	ro = std::make_unique<RenderObject>(plane, Game::Vector3(0.,0.,3.),Game::Vector3(0.,0.,0.),Game::Vector3(.1,.1,.1));
	Model* suit = gModelManager.loadModel("../asserts/suit/nanosuit.obj", "suit", &up);
	cro = std::make_unique<RenderObject>(suit, Game::Vector3(0.,0.,5.),Game::Vector3(0.,0.,0.),Game::Vector3(.05,.05,.05));
	up.End();


	gLightManager.SetAmbientLight(Game::Vector3(3., 3., 3.));
	LightData light;
	light.intensity = Game::Vector3(10., 10., 10.);
	light.direction = Game::normalize(Game::Vector3(0., -1., -1.));
	light.type = SHADER_LIGHT_TYPE_DIRECTIONAL;
	gLightManager.SetMainLightData(light);

	sdata[0].Color = Game::Vector4(1.,1.,1.,.5);
	sdata[0].Position = Game::Vector3(0., 0., .5);
	sdata[0].rotation = 0.;
	sdata[0].Scale = Game::Vector2(.3,.3);

	camera.attach(gGraphic.GetMainCamera());
	
	return true;
}

Game::Vector2 mousePos;

void Application::update() {

	if (gInput.KeyDown(InputBuffer::MOUSE_LEFT)) {
		mousePos = gInput.MousePosition();
	}
	if (gInput.KeyHold(InputBuffer::MOUSE_LEFT)) {
		Game::Vector2 currPos = gInput.MousePosition();
		Game::Vector2 dif = currPos - mousePos;

		camera.rotateX(dif.x * gTimer.DeltaTime() * 30.);
		camera.rotateY(dif.y * gTimer.DeltaTime() * 30.);

		mousePos = currPos;

		constexpr float speed = 1.;
		if (gInput.KeyHold(InputBuffer::W)) {
			camera.walk(speed * gTimer.DeltaTime());
		}
		else if (gInput.KeyHold(InputBuffer::S)) {
			camera.walk(-speed * gTimer.DeltaTime());
		}
		else if (gInput.KeyHold(InputBuffer::A)) {
			camera.strafe(-speed * gTimer.DeltaTime());
		}
		else if (gInput.KeyHold(InputBuffer::D)) {
			camera.strafe(speed * gTimer.DeltaTime());
		}
	}

	if (gInput.KeyDown(InputBuffer::ESCAPE)) {
		ApplicationQuit();
	}


	srp->DrawSpriteTransparent(1, sdata, face);
	ro->Render(drp);
	cro->Render(drp);
}

void Application::finalize() {
}

void Application::Quit() {
	
}