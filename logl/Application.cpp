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

Texture* face;
std::unique_ptr<StaticMesh<MeshVertexNormal>> planeMesh;

PhongRenderPass* prp;
DeferredRenderPass* drp;

std::unique_ptr<RenderObject> fro,pro;

std::vector<Game::Vector3> bullets;
FPSCamera camera;

#include "Config.h"

bool Application::initialize() {
	
	drp = gGraphic.GetRenderPass<DeferredRenderPass>();

	{
		UploadBatch up = UploadBatch::Begin();
		
		auto[v, i] = GeometryGenerator::Square(20., 20., GEOMETRY_FLAG_NONE);
		planeMesh = std::make_unique<StaticMesh<MeshVertexNormal>>(gGraphic.GetDevice(),
			i.size(),
			i.data(),
			v.size() / getVertexStrideByFloat<MeshVertexNormal>(),
			reinterpret_cast<MeshVertexNormal*>(v.data()),
			&up);

		Texture* fdiff = gTextureManager.loadTexture(L"../asserts/brickwall.jpg",5,L"wall",
			true);
		Texture* fnormal = gTextureManager.loadTexture(L"../asserts/brickwall_normal.jpg",5,
			L"wall_normal", true);

		fdiff->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
		fnormal->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
		SubMeshMaterial mat;
		mat.diffuse = Game::Vector3(1., 1., 1.);
		mat.roughness = 1.;
		mat.specular = Game::Vector3(.3, .3, .3);

		mat.textures[SUBMESH_MATERIAL_TYPE_BUMP] = fnormal;
		mat.textures[SUBMESH_MATERIAL_TYPE_DIFFUSE] = fdiff;
		mat.textures[SUBMESH_MATERIAL_TYPE_SPECULAR] = nullptr;

		mat.matTransformScale = Game::Vector2(20.,20.);

		fro = std::make_unique<RenderObject>(planeMesh->GetMesh(), mat, Game::Vector3(0., -2., 5.), Game::Vector3(), Game::Vector3(1., 1., 1.), "floor");
		Model* model = gModelManager.loadModel("../asserts/spaceship/spaceship.obj", "plane", &up);
		pro = std::make_unique<RenderObject>(model, Game::Vector3(0., 1., 3.), Game::Vector3(0., 0., 0.), Game::Vector3(.1, .1, .1));
		
		up.End();
	}

	gLightManager.SetAmbientLight(Game::Vector3(.1, .1, .1));
	LightData light;
	light.intensity = Game::Vector3(.3, .3, .3);
	light.direction = Game::normalize(Game::Vector3(0., -1., 0.1));
	light.type = SHADER_LIGHT_TYPE_DIRECTIONAL;
	gLightManager.SetMainLightData(light);


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

	static float li = 1.;
	if (gInput.KeyHold(InputBuffer::Q)) {
		li = fmax(li - 1e-2,0.);
	}
	else if (gInput.KeyHold(InputBuffer::E)) {
		li = fmin(li + 1e-2,40.);
	}

	LightData light;
	light.intensity = Game::Vector3(.3, .3, .3) * li;
	light.direction = Game::normalize(Game::Vector3(0., -1., 0.1));
	light.type = SHADER_LIGHT_TYPE_DIRECTIONAL;
	gLightManager.SetMainLightData(light);

	fro->Render(drp);
	pro->Render(drp);
}

void Application::finalize() {
}

void Application::Quit() {
	
}