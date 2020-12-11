#include "Application.h"
#include "../loglcore/graphic.h"
#include "../loglcore/CopyBatch.h"
#include "../loglcore/Mesh.h"
#include "../loglcore/Shader.h"
#include "../loglcore/GeometryGenerator.h"
#include "../loglcore/ConstantBuffer.h"
#include "../loglcore/TextureManager.h"
#include "../loglcore/DescriptorAllocator.h"

std::unique_ptr<StaticMesh> box;

struct BoxConstantBuffer {
	Game::Mat4x4 world;
	Game::Mat4x4 transInvWorld;
};

std::unique_ptr<ConstantBuffer<BoxConstantBuffer>>  boxBuffer;
BoxConstantBuffer* boxConstantBufferPtr;

Shader* shader;



ComPtr<ID3D12Resource> squareRes;
struct SpriteConstant {
	Game::Vector4 color;
	Game::Mat4x4 world;
};
struct ViewConstant{
	Game::Mat4x4 view;
};
D3D12_VERTEX_BUFFER_VIEW squareVBV;
std::unique_ptr<ConstantBuffer<SpriteConstant>> spriteConstant;
std::unique_ptr<ConstantBuffer<ViewConstant>> viewConstant;

Texture* targetSprite;
std::unique_ptr<DescriptorHeap> heap;
Shader* spriteShader;


void upload(){
	std::vector<float> vertices = std::move(GeometryGenerator::Cube(1.,1.,1.));
	box = std::make_unique<StaticMesh>( vertices.size() / 8, reinterpret_cast<MeshVertex*>(vertices.data()));

	boxBuffer = std::make_unique<ConstantBuffer<BoxConstantBuffer>>(gGraphic.GetDevice());
	boxConstantBufferPtr = boxBuffer->GetBufferPtr();

	boxConstantBufferPtr->world = Game::PackTransfrom(Game::Vector3(0., 0., 9), Game::Vector3(0., 0., 0.), Game::Vector3(1., 1., 1.));
	boxConstantBufferPtr->transInvWorld = boxConstantBufferPtr->world.R();
	boxConstantBufferPtr->world = boxConstantBufferPtr->world.T();

	shader = gShaderManager.getShaderByName(L"with_out_light");


	std::vector<float> square = {
		 .5, .5, 1., 1.,
		-.5, .5, 0., 1.,
		 .5,-.5, 1., 0.,
		-.5,-.5, 0., 0.,
		 .5,-.5, 1., 0.,
		-.5, .5, 0., 1.
	};

	ID3D12Device* mDevice = gGraphic.GetDevice();
	mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(square.size() * sizeof(float)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&squareRes)
	);
	heap = std::make_unique<DescriptorHeap>();

	spriteConstant = std::make_unique<ConstantBuffer<SpriteConstant>>(mDevice);
	viewConstant = std::make_unique<ConstantBuffer<ViewConstant>>(mDevice);

	spriteConstant->GetBufferPtr()->world = Game::MatrixScale(Game::Vector3(1.,1.,.5f));
	spriteConstant->GetBufferPtr()->color = Game::Vector4(1.,1.,1.,1.);

	viewConstant->GetBufferPtr()->view = Game::Mat4x4::I();

	void* ptr;
	squareRes->Map(0, nullptr, &ptr);
	memcpy(ptr, square.data(), square.size() * sizeof(float));

	targetSprite = gTextureManager.loadTexture(L"../asserts/junko.png", L"1");
	D3D12_SHADER_RESOURCE_VIEW_DESC srv;


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = targetSprite->GetFormat();
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

	targetSprite->CreateShaderResourceView(heap->Allocate(),srvDesc);
	squareVBV.BufferLocation = squareRes->GetGPUVirtualAddress();
	squareVBV.SizeInBytes = square.size() * sizeof(float);
	squareVBV.StrideInBytes = 16;
	spriteShader = gShaderManager.getShaderByName(L"DrawSprite");
}

bool Application::initialize() {
	upload();
	return true;
}
float r = 0.;
void Application::update() {
	/*r += 1e-2;

	boxConstantBufferPtr->world = Game::PackTransfrom(Game::Vector3(0., 0., 9), Game::Vector3(r, 0., 0.), Game::Vector3(1., 1., 1.));
	boxConstantBufferPtr->transInvWorld = boxConstantBufferPtr->world.R();
	boxConstantBufferPtr->world = boxConstantBufferPtr->world.T();

	gGraphic.BindShader(shader);
	gGraphic.BindMainCameraPass();
	gGraphic.BindConstantBuffer(boxBuffer->GetBuffer(), 0);
	gGraphic.Draw(box->GetVBV(), 0, box->GetVertexNum());*/

	gGraphic.BindShader(spriteShader);
	ID3D12DescriptorHeap* heaps[] = {heap->GetHeap()};
	gGraphic.BindDescriptorHeap(heaps,_countof(heaps));
	gGraphic.BindDescriptorHandle(targetSprite->GetShaderResourceViewGPU(),2);
	gGraphic.BindConstantBuffer(spriteConstant->GetBuffer(), 0);
	gGraphic.BindConstantBuffer(viewConstant->GetBuffer(), 1);
	gGraphic.Draw(&squareVBV, 0, 6);
	

}

void Application::finalize() {
	box.release();
}