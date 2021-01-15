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
std::unique_ptr<StaticMesh<MeshVertexNormal>> planeMesh;

PhongRenderPass* prp;
DeferredRenderPass* drp;

std::unique_ptr<RenderObject> ro,cro,fro;

std::vector<Game::Vector3> bullets;
FPSCamera camera;

#include "Config.h"

bool Application::initialize() {
	
	srp = gGraphic.GetRenderPass<SpriteRenderPass>();
	//drp = gGraphic.GetRenderPass<DeferredRenderPass>();
	prp = gGraphic.GetRenderPass<PhongRenderPass>();

	UploadBatch up = UploadBatch::Begin();
	face = gTextureManager.loadTexture(L"../asserts/awesomeface.png", L"face",true,&up);
	face->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	Model* plane = gModelManager.loadModel("../asserts/spaceship/spaceship.obj", "plane",&up);
	ro = std::make_unique<RenderObject>(plane, Game::Vector3(0.,0.,3.),Game::Vector3(0.,0.,0.),Game::Vector3(.1,.1,.1));
	Model* suit = gModelManager.loadModel("../asserts/suit/nanosuit.obj", "suit", &up);
	cro = std::make_unique<RenderObject>(suit, Game::Vector3(0.,0.,5.),Game::Vector3(0.,0.,0.),Game::Vector3(.05,.05,.05));

	auto[v, i] = GeometryGenerator::Square(20., 20., GEOMETRY_FLAG_NONE);
	planeMesh = std::make_unique<StaticMesh<MeshVertexNormal>>(gGraphic.GetDevice(),
		i.size(),
		i.data(),
		v.size() / getVertexStrideByFloat<MeshVertexNormal>(),
		reinterpret_cast<MeshVertexNormal*>(v.data()),
		&up);

	Texture* fdiff = gTextureManager.loadTexture(L"../asserts/brickwall.jpg", L"wall",
		true, &up);
	Texture* fnormal = gTextureManager.loadTexture(L"../asserts/brickwall_normal.jpg",
		L"wall_normal", true, &up);

	fdiff->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	fnormal->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	SubMeshMaterial mat;
	mat.diffuse = Game::Vector3(1., 1., 1.);
	mat.roughness = 1.;
	mat.specular = Game::Vector3(.3, .3, .3);
	
	mat.textures[SUBMESH_MATERIAL_TYPE_BUMP] = fnormal;
	mat.textures[SUBMESH_MATERIAL_TYPE_DIFFUSE] = fdiff;
	mat.textures[SUBMESH_MATERIAL_TYPE_SPECULAR] = nullptr;
	
	fro = std::make_unique<RenderObject>(planeMesh->GetMesh(), mat, Game::Vector3(0., -2., 5.), Game::Vector3(), Game::Vector3(1., 1., 1.), "floor");
	up.End();


	gLightManager.SetAmbientLight(Game::Vector3(.1, .1, .1));
	LightData light;
	light.intensity = Game::Vector3(.3, .3, .3);
	light.direction = Game::normalize(Game::Vector3(0., -1., 0.1));
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
	ro->Render(prp);
	cro->Render(prp);
	fro->Render(prp);
}

void Application::finalize() {
}

void Application::Quit() {
	
}