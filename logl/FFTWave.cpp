#include "FFTWave.h"
#include "../loglcore/graphic.h"
#include "../loglcore/GeometryGenerator.h"
#include "RandomGenerator.h"
#include "../loglcore/ComputeCommand.h"
#include "../loglcore/LightManager.h"

bool   FFTWaveRenderPass::Initialize(UploadBatch* batch) {
	Game::RootSignature rootSig(5,1);
	rootSig[0].initAsConstantBuffer(0, 0);
	rootSig[1].initAsConstantBuffer(1, 0);
	rootSig[2].initAsConstantBuffer(2, 0);
	rootSig[3].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	rootSig[4].initAsDescriptorTable(1, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	rootSig.InitializeSampler(0, CD3DX12_STATIC_SAMPLER_DESC(0));

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
	//rd.FillMode = D3D12_FILL_MODE_WIREFRAME;
	//mPso.SetRasterizerState(rd);
	mPso.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
	mPso.SetNodeMask(0);
	mPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	if (!gGraphic.CreatePipelineStateObjectRP(fftWaveShader, &mPso)) {
		OUTPUT_DEBUG_STRING("fail to create pso for fftwave\n");
		return false;
	}

	ID3D12Device* mDevice = gGraphic.GetDevice();
	objectPass = std::make_unique<ConstantBuffer<FFTWaveObjectPass>>(mDevice);
	objectPass->GetBufferPtr()->DepthColor = Game::Vector4(0.3071377, 0.4703594, 0.5471698,1.);
	objectPass->GetBufferPtr()->SwallowColor = Game::Vector4(0.04992878, 0.1436478, 0.2075472,1.);

	Game::RootSignature fftRootSig(7,0);
	fftRootSig[0].initAsConstantBuffer(0, 0);
	fftRootSig[1].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	fftRootSig[2].initAsDescriptorTable(1, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	fftRootSig[3].initAsDescriptorTable(2, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	fftRootSig[4].initAsDescriptorTable(3, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	fftRootSig[5].initAsDescriptorTable(4, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	fftRootSig[6].initAsDescriptorTable(5, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	

	if (!gGraphic.CreateRootSignature(L"fftwave_cs",&fftRootSig)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for fft wave\n");
		return false;
	}

	auto createShaderForCS = [&](const char* shaderFName,const wchar_t* shaderName)->bool {
		ComputeShader* fftShader = gShaderManager.loadComputeShader(
			L"../shader/Custom/FFTWaveCS.hlsl", shaderFName,
			L"fftwave_cs", shaderName
		);

		if (fftShader == nullptr) {
			OUTPUT_DEBUG_STRINGW((std::wstring(L"In fftwave : fail to create shader ") + shaderName + L"\n").c_str());
			return false;
		}

		Game::ComputePSO cPso = Game::ComputePSO::Default();
		if (!gGraphic.CreateComputePipelineStateObject(fftShader,&cPso)) {
			OUTPUT_DEBUG_STRINGW((std::wstring(L"In fftwave : fail to create pso ") + shaderName + L"\n").c_str());
			return false;
		}

		return true;
	};

	if (!createShaderForCS("GenerateSpectrums", L"GenerateSpectrums")
		|| !createShaderForCS("FFTVertical",L"FFTVertical")
		|| !createShaderForCS("FFTHorizontal",L"FFTHorizontal")
		|| !createShaderForCS("GenerateHeightNormal",L"GenerateHeightNormal")
		) {
		OUTPUT_DEBUG_STRING("fail to initialize fft wave\n");
		return false;
	}

	N = (int)powf(2,(float)NPow);

	waveConstant = std::make_unique<ConstantBuffer<WaveConstant>>(mDevice, NPow);

	Game::Vector2* GaussMap = new Game::Vector2[N * N];
	for (size_t y = 0; y != N; y++) {
		for (size_t x = 0; x != N; x++) {
			GaussMap[x + y * N] = gRandomGenerator.Rand2(NormalDistribution());
		}
	}

	mRandomMap  = std::make_unique<Texture>(N, N,TEXTURE_FORMAT_FLOAT2, 
		reinterpret_cast<void**>(&GaussMap), TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,batch);
	
	mHeightMap = std::make_unique<Texture>(N, N, TEXTURE_FORMAT_FLOAT2,
		TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS),
	mGradientX = std::make_unique<Texture>(N, N, TEXTURE_FORMAT_FLOAT2,
		TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS),
	mGradientZ = std::make_unique<Texture>(N, N, TEXTURE_FORMAT_FLOAT2,
		TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS);

	mSpareTexture = std::make_unique<Texture>(N, N, TEXTURE_FORMAT_FLOAT2,
		TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS);

	mHeap = std::make_unique<DescriptorHeap>(30);

	mHeightMap->CreateShaderResourceView(mHeap->Allocate());
	mHeightMap->CreateUnorderedAccessView(mHeap->Allocate());

	mGradientX->CreateShaderResourceView(mHeap->Allocate());
	mGradientX->CreateUnorderedAccessView(mHeap->Allocate());

	mGradientZ->CreateShaderResourceView(mHeap->Allocate());
	mGradientZ->CreateUnorderedAccessView(mHeap->Allocate());

	mSpareTexture->CreateShaderResourceView(mHeap->Allocate());
	mSpareTexture->CreateUnorderedAccessView(mHeap->Allocate());

	mRandomMap->CreateUnorderedAccessView(mHeap->Allocate());

	randSeed = gRandomGenerator.Rand2(UniformDistribution<2>(1.f, 10.f));

	return true;
}

void   FFTWaveRenderPass::ComputeFFT(std::unique_ptr<Texture>& tex,size_t index,ComputeCommand& cc) {
	cc.BindConstantBuffer(0, waveConstant->GetADDR(index));
	cc.BindDescriptorHandle(5, tex->GetUnorderedAccessViewGPU());
	cc.BindDescriptorHandle(6, mSpareTexture->GetUnorderedAccessViewGPU());
	size_t dispatch = N / 16;
	cc.Dispatch(dispatch, dispatch, 1);

	std::swap(tex, mSpareTexture);
}


void   FFTWaveRenderPass::Render(Graphic* graphic, RENDER_PASS_LAYER layer) {
	if (wave == nullptr) return;
	if (layer != RENDER_PASS_LAYER_OPAQUE) return;

	UpdateWaveConstant();
	size_t dispatch = N / 16;

	{
		ComputeCommand cc = ComputeCommand::Begin();

		ID3D12DescriptorHeap* heaps[] = {mHeap->GetHeap()};
		cc.BindDescriptorHeap(heaps, _countof(heaps));

		cc.SetCPSOAndRootSignature(L"GenerateSpectrums",L"fftwave_cs");
		cc.BindConstantBuffer(0, waveConstant->GetADDR());
		cc.BindDescriptorHandle(1, mHeightMap->GetUnorderedAccessViewGPU());
		cc.BindDescriptorHandle(2, mGradientX->GetUnorderedAccessViewGPU());
		cc.BindDescriptorHandle(3, mGradientZ->GetUnorderedAccessViewGPU());
		cc.BindDescriptorHandle(4, mRandomMap->GetUnorderedAccessViewGPU());
		cc.Dispatch(dispatch, dispatch, 1);


		//start the fft process
		cc.SetCPSOAndRootSignature(L"FFTHorizontal",L"fftwave_cs");
		for (size_t i = 0; i != NPow; i++) {
			ComputeFFT(mHeightMap, i, cc);
			ComputeFFT(mGradientX, i, cc);
			ComputeFFT(mGradientZ, i, cc);
		}
		
		cc.SetCPSOAndRootSignature(L"FFTVertical", L"fftwave_cs");
		for (size_t i = 0; i != NPow; i++) {
			ComputeFFT(mHeightMap, i, cc);
			ComputeFFT(mGradientX, i, cc);
			ComputeFFT(mGradientZ, i, cc);
		}

		cc.SetCPSOAndRootSignature(L"GenerateHeightNormal", L"fftwave_cs");
		cc.BindConstantBuffer(0, waveConstant->GetADDR());
		cc.BindDescriptorHandle(1, mHeightMap->GetUnorderedAccessViewGPU());
		cc.BindDescriptorHandle(2, mGradientX->GetUnorderedAccessViewGPU());
		cc.BindDescriptorHandle(3, mGradientZ->GetUnorderedAccessViewGPU());
		cc.Dispatch(dispatch, dispatch, 1);

		cc.End();
	}

	graphic->BindPSOAndRootSignature(L"fftwave",L"fftwave");
	graphic->BindConstantBuffer(objectPass->GetADDR(),0);
	graphic->BindMainCameraPass(1);
	gLightManager.BindLightPass2ConstantBuffer(2);
	graphic->BindDescriptorHandle(mHeightMap->GetShaderResourceViewGPU(), 3);
	//the gradient z will be used as the output of the normal map
	graphic->BindDescriptorHandle(mGradientX->GetShaderResourceViewGPU(), 4);

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

void	FFTWaveRenderPass::UpdateWaveConstant(){
	for (size_t i = 0; i != NPow; i++) {
		waveConstant->GetBufferPtr(i)->A = A;
		waveConstant->GetBufferPtr(i)->BubblesScale = BubblesScale;
		waveConstant->GetBufferPtr(i)->BubblesThreshold = BubblesThreshold;
		waveConstant->GetBufferPtr(i)->HeightScale = HeightScale;
		waveConstant->GetBufferPtr(i)->Lambda = Lambda;
		waveConstant->GetBufferPtr(i)->N = N;
		waveConstant->GetBufferPtr(i)->Ns = 1 << i;
		waveConstant->GetBufferPtr(i)->OceanLength = OceanLength;
		waveConstant->GetBufferPtr(i)->Time = currentTime;
		waveConstant->GetBufferPtr(i)->WindAndSeed = Game::Vector4(Game::normalize(windDir) * windScale,randSeed);
		waveConstant->GetBufferPtr(i)->Ended = (i == (NPow - 1));
	}
}

bool FFTWave::Initialize(float width,float height) {
	auto[vertices, indices] = GeometryGenerator::Plane(512, 512,
		250,250,GeometryGenerator::GEOMETRY_FLAG_DISABLE_TANGENT);

	mMesh = std::make_unique<StaticMesh<MeshVertex>>(
			gGraphic.GetDevice(),
			indices.size(),indices.data(),
			vertices.size() / getVertexStrideByFloat<MeshVertex>(),
		    reinterpret_cast<MeshVertex*>(vertices.data())
		);
	
	currentTime = 0.f;

	position = Game::Vector3(0., 0., 0.);
	rotation = Game::Vector3(0., 0., 0.);
	scale = Game::Vector3(1., 1., 1.);
	transformUpdated = true;

	mRenderPass = std::make_unique<FFTWaveRenderPass>();
	mRenderPass->Attach(this);

	mRenderPass->SetWaveLength(512);

	RenderPass* rps[] = { mRenderPass.get() };
	if (!gGraphic.RegisterRenderPasses(rps, _countof(rps))) {
		return false;
	};

	return true;
}

static constexpr float g = 9.8;

void FFTWave::Update(float deltaTime) {
	if (transformUpdated) {
		mRenderPass->SetWorldTransform(position, rotation, scale);
		transformUpdated = false;
	}


	currentTime += deltaTime * 2.;
	mRenderPass->UpdateTime(currentTime);
}