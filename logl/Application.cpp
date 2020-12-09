#include "Application.h"
#include "../loglcore/graphic.h"
#include "../loglcore/CopyBatch.h"
#include "../loglcore/Mesh.h"
#include "../loglcore/Shader.h"
#include "../loglcore/GeometryGenerator.h"
#include "../loglcore/ConstantBuffer.h"

std::unique_ptr<DynamicMesh> box;

struct BoxConstantBuffer {
	Game::Mat4x4 world;
	Game::Mat4x4 transInvWorld;
};

std::unique_ptr<ConstantBuffer<BoxConstantBuffer>>  boxBuffer;
BoxConstantBuffer* boxConstantBufferPtr;

Shader* shader;


void upload(){

	std::vector<float> vertices = std::move(GeometryGenerator::Cube(1.,1.,1.));
	box = std::make_unique<DynamicMesh>( vertices.size() / 8, reinterpret_cast<MeshVertex*>(vertices.data()));

	boxBuffer = std::make_unique<ConstantBuffer<BoxConstantBuffer>>(gGraphic.GetDevice());
	boxConstantBufferPtr = boxBuffer->GetBufferPtr();

	boxConstantBufferPtr->world = Game::PackTransfrom(Game::Vector3(0., 0., 9), Game::Vector3(0., 0., 0.), Game::Vector3(1., 1., 1.));
	boxConstantBufferPtr->transInvWorld = boxConstantBufferPtr->world.R();
	boxConstantBufferPtr->world = boxConstantBufferPtr->world.T();

	shader = gShaderManager.getShaderByName(L"with_out_light");
}

bool Application::initialize() {
	upload();
	return true;
}
float r = 0.;
void Application::update() {
	r += 1e-2;
	static bool triggered = false;

	MeshVertex v;
	v = { {0.,0.,0.},{0.,0.,0.},{0.,0.} };
	v.Position.x = sinf(r * 10.);

	boxConstantBufferPtr->world = Game::PackTransfrom(Game::Vector3(0., 0., 9), Game::Vector3(r, 0., 0.), Game::Vector3(1., 1., 1.));
	boxConstantBufferPtr->transInvWorld = boxConstantBufferPtr->world.R();
	boxConstantBufferPtr->world = boxConstantBufferPtr->world.T();

	gGraphic.BindShader(shader);
	gGraphic.BindMainCameraPass();
	gGraphic.BindConstantBuffer(boxBuffer->GetBuffer(), 0);
	gGraphic.Draw(box->GetVBV(), 0, box->GetVertexNum());
}

void Application::finalize() {
	box.release();
}