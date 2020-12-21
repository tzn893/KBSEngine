#include "FFTWave.h"
#include "../loglcore/graphic.h"
#include "../loglcore/GeometryGenerator.h"


bool   FFTWaveRenderPass::Initialize(UploadBatch* batch) {
	Game::RootSignature rootSig(3,0);
	rootSig[0].initAsConstantBuffer(0, 0);
	rootSig[1].initAsConstantBuffer(1, 0);
	rootSig[2].initAsConstantBuffer(2, 0);

	if (!gGraphic.CreateRootSignature(L"fftwave",&rootSig)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for fft wave\n");
		return false;
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
	};

	fftWaveShader = gShaderManager.loadShader(L"../shader/Custom/FFTWave.hlsl",
		"VS", "PS", L"fftwave", inputLayout,
		L"fftwave", nullptr);

	if (fftWaveShader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for fft wave\n");
		return false;
	}

	Game::GraphicPSORP mPso;
	mPso.LazyBlendDepthRasterizeDefault();
	CD3DX12_RASTERIZER_DESC rd(D3D12_DEFAULT);
	rd.FillMode = D3D12_FILL_MODE_WIREFRAME;
	mPso.SetRasterizerState(rd);
	mPso.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
	mPso.SetNodeMask(0);
	mPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	if (!gGraphic.CreatePipelineStateObjectRP(fftWaveShader, &mPso)) {
		OUTPUT_DEBUG_STRING("fail to create pso for fftwave\n");
		return false;
	}

	ID3D12Device* mDevice = gGraphic.GetDevice();
	lightPass = std::make_unique<ConstantBuffer<LightPass>>(mDevice);
	objectPass = std::make_unique<ConstantBuffer<FFTWaveObjectPass>>(mDevice);

	return true;
}
void   FFTWaveRenderPass::Render(Graphic* graphic, RENDER_PASS_LAYER layer) {
	if (wave == nullptr) return;
	if (layer != RENDER_PASS_LAYER_OPAQUE) return;

	graphic->BindPSOAndRootSignature(L"fftwave",L"fftwave");
	graphic->BindConstantBuffer(objectPass->GetADDR(),0);
	graphic->BindMainCameraPass(1);
	graphic->BindConstantBuffer(lightPass->GetADDR(), 2);

	graphic->Draw(wave->mMesh->GetVBV(),
		wave->mMesh->GetIBV(), 0, wave->mMesh->GetIndexNum());
}

void   FFTWaveRenderPass::finalize() {

}

void   FFTWaveRenderPass::SetWorldTransform(Game::Vector3 position, Game::Vector3 rotation, Game::Vector3 scale) {
	Game::Mat4x4 world = Game::PackTransfrom(position, rotation, scale);
	objectPass->GetBufferPtr()->world = world.T();
	objectPass->GetBufferPtr()->transInvWorld = world.R();
}

bool FFTWave::Initialize(float width,float height) {
	auto[vertices, indices] = GeometryGenerator::Square(width, height,GeometryGenerator::GEOMETRY_FLAG_DISABLE_TANGENT);

	mMesh = std::make_unique<DynamicMesh<MeshVertex>>(
			gGraphic.GetDevice(),
			indices.size(),indices.data(),
			vertices.size() / getVertexStrideByFloat<MeshVertex>(),
		    reinterpret_cast<MeshVertex*>(vertices.data())
		);
	mRenderPass = std::make_unique<FFTWaveRenderPass>();
	mRenderPass->Attach(this);
	RenderPass* rps[] = {mRenderPass.get()};
	if (!gGraphic.RegisterRenderPasses(rps, _countof(rps))) {
		return false;
	}
	currentTime = 0.f;

	position = Game::Vector3(0., 0., 0.);
	rotation = Game::Vector3(0., 0., 0.);
	scale = Game::Vector3(1., 1., 1.);
	transformUpdated = true;

	return true;
}
void FFTWave::Update(float deltaTime) {
	if (transformUpdated) {
		mRenderPass->SetWorldTransform(position, rotation, scale);
		transformUpdated = false;
	}

	currentTime += deltaTime;
}