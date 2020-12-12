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

#include "InputBuffer.h"


std::unique_ptr<StaticMesh> box;

struct BoxConstantBuffer {
	Game::Mat4x4 world;
	Game::Mat4x4 transInvWorld;
};

std::unique_ptr<ConstantBuffer<BoxConstantBuffer>>  boxBuffer;
BoxConstantBuffer* boxConstantBufferPtr;

Shader* shader;


SpriteRenderPass* srp;
SpriteID spriteID;

SpriteData data[100];

void upload(){
	std::vector<float> vertices = std::move(GeometryGenerator::Cube(1.,1.,1.));
	box = std::make_unique<StaticMesh>( vertices.size() / 8, reinterpret_cast<MeshVertex*>(vertices.data()));

	boxBuffer = std::make_unique<ConstantBuffer<BoxConstantBuffer>>(gGraphic.GetDevice());
	boxConstantBufferPtr = boxBuffer->GetBufferPtr();

	boxConstantBufferPtr->world = Game::PackTransfrom(Game::Vector3(0., 0., 9), Game::Vector3(0., 0., 0.), Game::Vector3(1., 1., 1.));
	boxConstantBufferPtr->transInvWorld = boxConstantBufferPtr->world.R();
	boxConstantBufferPtr->world = boxConstantBufferPtr->world.T();

	shader = gShaderManager.getShaderByName(L"with_out_light");

	for (int x = 0; x != 10; x++) {
		for (int y = 0; y != 10; y++) {
			data[x + y * 10].Color = Game::Vector4(x / 10., y / 10., 1., 1.);
			data[x + y * 10].Scale = Game::Vector2(.1, .1);
			data[x + y * 10].Position = Game::Vector3((x - 5)/ 11., (y - 5)/ 11., .5);
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
	return true;
}
float r = 0.;
void Application::update() {
	r += 1e-2;

	boxConstantBufferPtr->world = Game::PackTransfrom(Game::Vector3(0., 0., 9), Game::Vector3(r, 0., 0.), Game::Vector3(1., 1., 1.));
	boxConstantBufferPtr->transInvWorld = boxConstantBufferPtr->world.R();
	boxConstantBufferPtr->world = boxConstantBufferPtr->world.T();

	gGraphic.BindShader(shader);
	gGraphic.BindMainCameraPass();
	gGraphic.BindConstantBuffer(boxBuffer->GetBuffer(), 0);
	gGraphic.Draw(box->GetVBV(), 0, box->GetVertexNum());


	for (int x = 0; x != 10; x++) {
		for (int y = 0; y != 10; y++) {
			float r1 = r * sinf(x * 17 + y * 7) * 2.;
			data[x + y * 10].rotation = r1;
		}
	}

	srp->DrawSprite(100, data, spriteID);
}

void Application::finalize() {
	box.release();
}