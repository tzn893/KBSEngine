#include "FFTWave.h"
#include "../loglcore/graphic.h"
#include "../loglcore/GeometryGenerator.h"
#include "RandomGenerator.h"
#include "../loglcore/ComputeCommand.h"


bool   FFTWaveRenderPass::Initialize(UploadBatch* batch) {
	Game::RootSignature rootSig(4,1);
	rootSig[0].initAsConstantBuffer(0, 0);
	rootSig[1].initAsConstantBuffer(1, 0);
	rootSig[2].initAsConstantBuffer(2, 0);
	rootSig[3].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
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
	mGenConstant = std::make_unique<ConstantBuffer<FFTWaveGenerateConstant>>(mDevice);
	mGenConstant->GetBufferPtr()->length = 1.f;

	mHeap = std::make_unique<DescriptorHeap>(8);
	Descriptor dsc = mHeap->Allocate(3);
	wave->HeightMap->CreateUnorderedAccessView(dsc.Offset(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0));
	wave->GradientX->CreateUnorderedAccessView(dsc.Offset(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1));
	wave->GradientZ->CreateUnorderedAccessView(dsc.Offset(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2));

	wave->HeightMap->CreateShaderResourceView(mHeap->Allocate());
	wave->GradientX->CreateShaderResourceView(mHeap->Allocate());
	wave->GradientZ->CreateShaderResourceView(mHeap->Allocate());

	wave->HConjmapTex->CreateUnorderedAccessView(mHeap->Allocate());
	wave->HmapTex->CreateUnorderedAccessView(mHeap->Allocate());

	mGenConstant = std::make_unique<ConstantBuffer<FFTWaveGenerateConstant>>(mDevice);
	Game::RootSignature waveGenRootSig(4,0);
	waveGenRootSig[0].initAsDescriptorTable(0, 0, 3, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	waveGenRootSig[1].initAsDescriptorTable(3, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	waveGenRootSig[2].initAsDescriptorTable(4, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	waveGenRootSig[3].initAsConstantBuffer(0, 0);

	if (!gGraphic.CreateRootSignature(L"fftwave_gen",&waveGenRootSig)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for fft wave\n");
		return false;
	}

	ComputeShader* CS = gShaderManager.loadComputeShader(L"../shader/Custom/FFTWaveGenerate.hlsl",
		"GenerateHFrequence", L"fftwave_gen", L"fftwave_gen");

	Game::ComputePSO waveGenPso;
	waveGenPso.SetComputePipelineStateFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
	waveGenPso.SetComputePipelineStateNodeMask(0);
	if (!gGraphic.CreateComputePipelineStateObject(CS,&waveGenPso,L"fftwave_gen")) {
		OUTPUT_DEBUG_STRING("fail to create pso for fft wave\n");
		return false;
	}

	Game::RootSignature waveFFTRootSig(2,0);
	waveFFTRootSig[0].initAsConstantBuffer(0, 0);
	waveFFTRootSig[1].initAsDescriptorTable(0, 0, 3, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);

	if (!gGraphic.CreateRootSignature(L"fftwave_fft",&waveFFTRootSig)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for fft wave\n");
		return false;
	}

	ComputeShader* fftCS = gShaderManager.loadComputeShader(L"../shader/Custom/FFT64CS.hlsl",
		"PerformFFT64", L"fftwave_fft", L"fftwave_fft");
	if (fftCS == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for fft wave\n");
		return false;
	}

	Game::ComputePSO waveFFTPso;
	waveFFTPso.SetComputePipelineStateFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
	waveFFTPso.SetComputePipelineStateNodeMask(0);
	
	if (!gGraphic.CreateComputePipelineStateObject(fftCS,&waveFFTPso,L"fftwave_fft")) {
		OUTPUT_DEBUG_STRING("fail to create pipeline state object for fft wave\n");
		return false;
	}
	mGenConstant->GetBufferPtr()->length = 1.;

	return true;
}
void   FFTWaveRenderPass::Render(Graphic* graphic, RENDER_PASS_LAYER layer) {
	if (wave == nullptr) return;
	if (layer != RENDER_PASS_LAYER_OPAQUE) return;

	ComputeCommand cc = ComputeCommand::Begin();

	cc.ResourceTrasition(wave->HeightMap->GetResource(),D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	cc.ResourceTrasition(wave->GradientX->GetResource(), D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	cc.ResourceTrasition(wave->GradientZ->GetResource(), D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	cc.SetCPSOAndRootSignature(L"fftwave_gen",L"fftwave_gen");
	cc.BindConstantBuffer(3, mGenConstant->GetADDR());
	ID3D12DescriptorHeap* heaps[] = { mHeap->GetHeap()};
	cc.BindDescriptorHeap(heaps, _countof(heaps));
	cc.BindDescriptorHandle(0, wave->HeightMap->GetUnorderedAccessViewGPU());
	cc.BindDescriptorHandle(1, wave->HmapTex->GetUnorderedAccessViewGPU());
	cc.BindDescriptorHandle(2, wave->HConjmapTex->GetUnorderedAccessViewGPU());

	cc.Dispatch(4, 4, 1);

	cc.SetCPSOAndRootSignature(L"fftwave_fft", L"fftwave_fft");
	cc.BindConstantBuffer(0, mGenConstant->GetADDR());
	cc.BindDescriptorHandle(1, wave->HeightMap->GetUnorderedAccessViewGPU());

	cc.Dispatch(1, 1, 1);

	cc.ResourceTrasition(wave->HeightMap->GetResource(), 
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,D3D12_RESOURCE_STATE_COMMON);
	cc.ResourceTrasition(wave->GradientX->GetResource(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,D3D12_RESOURCE_STATE_COMMON);
	cc.ResourceTrasition(wave->GradientZ->GetResource(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,D3D12_RESOURCE_STATE_COMMON);

	cc.End();


	graphic->BindPSOAndRootSignature(L"fftwave",L"fftwave");
	graphic->BindConstantBuffer(objectPass->GetADDR(),0);
	graphic->BindMainCameraPass(1);
	graphic->BindConstantBuffer(lightPass->GetADDR(), 2);
	graphic->BindDescriptorHandle(wave->HeightMap->GetShaderResourceViewGPU(), 3);

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
	auto[vertices, indices] = GeometryGenerator::Plane(width, height,
		128,128,GeometryGenerator::GEOMETRY_FLAG_DISABLE_TANGENT);

	mMesh = std::make_unique<DynamicMesh<MeshVertex>>(
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

	SetWaveWind(Game::Vector2(1., 0.));
	SetWaveHeight(.3);
	SetWaveLength(1.);

	/*for (int x = 0; x != rowNum; x++) {
		for (int y = 0; y != rowNum; y++) {
			Hmap[y][x] = h0(x, y);
			HConjmap[y][x] = h0(-x, -y).conj();
		}
	}*/
	Complex* Hmap = new Complex[rowNum * rowNum];
	Complex* HConjmap = new Complex[rowNum * rowNum];

	for (int y = 0; y != rowNum; y++) {
		for (int x = 0; x != rowNum; x++) {
			Hmap[y * rowNum + x] = h0(x, y);
			HConjmap[y * rowNum + x] = h0(-x, -y).conj();
			Complex a = h0(x, y);
		}
	}
	void* hmapd = Hmap;
	void* hconjmapd = HConjmap;

	UploadBatch batch = UploadBatch::Begin();
	HmapTex = std::make_unique<Texture>(rowNum, rowNum, TEXTURE_FORMAT_FLOAT2,
		&hmapd, TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS,D3D12_RESOURCE_STATE_UNORDERED_ACCESS,&batch);
	HConjmapTex = std::make_unique<Texture>(rowNum,rowNum,TEXTURE_FORMAT_FLOAT2,
		&hconjmapd,TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS,D3D12_RESOURCE_STATE_UNORDERED_ACCESS,&batch);
	batch.End();

	HeightMap = std::make_unique<Texture>(rowNum, rowNum, TEXTURE_FORMAT_FLOAT2, TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS);
	GradientX = std::make_unique<Texture>(rowNum, rowNum, TEXTURE_FORMAT_FLOAT2, TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS);
	GradientZ = std::make_unique<Texture>(rowNum, rowNum, TEXTURE_FORMAT_FLOAT2, TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS);

	mRenderPass = std::make_unique<FFTWaveRenderPass>();
	mRenderPass->Attach(this);
	RenderPass* rps[] = { mRenderPass.get() };
	if (!gGraphic.RegisterRenderPasses(rps, _countof(rps))) {
		return false;
	};

	return true;
}

static constexpr float g = 9.8;
/*
#pragma region FFT
Complex W(size_t N, size_t K) {
	return Complex::exp(0., static_cast<float>(K) / static_cast<float>(N) * 2 * PI);
}

uint32_t bitInverse64(uint32_t index) {
	uint32_t rv = 0;
	rv |= (index & 0b1) << 5;
	rv |= (index & 0b10) << 3;
	rv |= (index & 0b100) << 1;
	rv |= (index & 0b1000) >> 1;
	rv |= (index & 0b10000) >> 3;
	rv |= (index & 0b100000) >> 5;
	return rv;
}

class ComplexAccessor {
public:
	virtual Complex& operator[](size_t index) = 0;
};

class VerticalComplexAccessor : public ComplexAccessor {
public:
	virtual Complex& operator[](size_t index) override {
		return (*mComplexArray)[index][columnIndex];
	}
	VerticalComplexAccessor(std::vector<std::vector<Complex>>* mComplexArray,
		size_t columnIndex) :mComplexArray(mComplexArray),
		columnIndex(columnIndex) {}
private:
	std::vector<std::vector<Complex>>* mComplexArray;
	size_t columnIndex;
};

class HorizontalComplexAccessor : public ComplexAccessor {
public:
	virtual Complex& operator[](size_t index) override {
		return (*mComplexArray)[rowIndex][index];
	}
	HorizontalComplexAccessor(std::vector<std::vector<Complex>>* mComplexArray,
		size_t rowIndex) :mComplexArray(mComplexArray), rowIndex(rowIndex) {}
private:
	std::vector<std::vector<Complex>>* mComplexArray;
	size_t rowIndex;
};


void PerformFFTOn64(ComplexAccessor& target, ComplexAccessor& result) {
	std::vector<Complex> input(64), output(64);
	for (uint32_t x = 0; x != 64; x++) {
		input[x] = target[bitInverse64(x)];
	}

	for (uint32_t netWidth = 2; netWidth <= 64; netWidth = netWidth << 1) {
		for (uint32_t groupStart = 0; groupStart != 64; groupStart += netWidth) {
			uint32_t halfWidth = netWidth / 2;
			for (uint32_t index = 0; index != halfWidth; index++) {
				output[index + groupStart] = input[index + groupStart] + W(netWidth, index) * input[index + halfWidth + groupStart];
				output[index + groupStart + halfWidth] = input[index + groupStart] - W(netWidth, index) * input[index + halfWidth + groupStart];
			}
		}
		std::swap(output, input);
	}
	std::swap(output, input);

	for (uint32_t x = 0; x != 64; x++) {
		result[x] = output[x];
	}
}
#pragma endregion
*/

float FFTWave::phillips(uint32_t gridn,uint32_t gridm) {
	Game::Vector2 k = Game::Vector2((static_cast<float>(gridn) * 2. - rowNum)/ length,
		(static_cast<float>(gridm)  * 2. - rowNum) / length);
	float k_len = Game::length(k);
	if (k_len < 1e-4) return 0.;

	float k_len_2 = k_len * k_len, k_len_4 = k_len_2 * k_len_2;
	float k_dot_w = Game::dot(Game::normalize(k), windDir);
	float k_dot_w2 = k_dot_w * k_dot_w;

	constexpr float damping = 1.;
	float l = windSpeed * windSpeed / g ,l2 = l * l;
	return height * expf(- 1.f / (k_len_2 * l2)) / k_len_4 * k_dot_w2;
}

Complex FFTWave::h0(uint32_t gridn,uint32_t gridm) {
	Complex r(gRandomGenerator.Rand2(NormalDistribution()));
	return r * sqrt(phillips(gridn, gridm) / 2.);
}

/*
Complex FFTWave::ht(uint32_t gridn,uint32_t gridm,float time) {
	Complex H0 = Hmap[gridm][gridn], H0Conj = HConjmap[gridm][gridn];
	
	Game::Vector2 k = Game::Vector2((static_cast<float>(gridn) * 2. - rowNum) / length,
		(static_cast<float>(gridm)  * 2. - rowNum) / length);
	float fw0 = sqrt(g * Game::length(k)) * time;

	return H0 * Complex::exp(0, fw0) + H0Conj * Complex::exp(0, -fw0);
}
*/


void FFTWave::Update(float deltaTime) {
	if (transformUpdated) {
		mRenderPass->SetWorldTransform(position, rotation, scale);
		transformUpdated = false;
	}

	/*std::vector<std::vector<Complex>> frequence(64,std::vector<Complex>(64));
	std::vector<std::vector<Complex>> grandient_x(64, std::vector<Complex>(64));
	std::vector<std::vector<Complex>> grandient_z(64, std::vector<Complex>(64));

	//for (size_t ky = 0; ky != 64; ky++) {
	concurrency::parallel_for(0,64,[&](size_t ky){
		for (size_t kx = 0; kx != 64; kx++) {
			frequence[ky][kx] = ht(kx, ky, currentTime);

			Game::Vector2 k = Game::Vector2((static_cast<float>(kx) * 2. - rowNum) / length,
				(static_cast<float>(ky)  * 2. - rowNum) / length);

			grandient_x[ky][kx] = Complex(0., k.x ) * frequence[ky][kx];
			grandient_z[ky][kx] = Complex(0., k.y ) * frequence[ky][kx];
		}
		HorizontalComplexAccessor accessor(&frequence, ky);
		PerformFFTOn64(accessor, accessor);
		HorizontalComplexAccessor gaccessorx(&grandient_x, ky);
		PerformFFTOn64(gaccessorx, gaccessorx);
		HorizontalComplexAccessor gaccessorz(&grandient_z, ky);
		PerformFFTOn64(gaccessorz, gaccessorz);
	});
	//}
	//for (size_t kx = 0; kx != 64; kx++) {
	concurrency::parallel_for(0,64,[&](size_t kx){
		//for (size_t ky = 0; ky != 64; ky++) {
			//if (kx + ky % 2 == 1)
				//frequence[ky][kx] *= -1;
		//}
		VerticalComplexAccessor accessor(&frequence, kx);
		PerformFFTOn64(accessor, accessor);
		VerticalComplexAccessor gaccessorx(&grandient_x, kx);
		PerformFFTOn64(gaccessorx, gaccessorx);
		VerticalComplexAccessor gaccessorz(&grandient_z, kx);
		PerformFFTOn64(gaccessorz, gaccessorz);
		});
	//}

	for (size_t y = 0; y != 64; y++) {
		for (size_t x = 0; x != 64; x++) {
			//do fft wave simulate
			float sign = (x + y) & 1 ? 1 : -1;
			
			mMesh->GetVertex(y * 64 + x)->Position.y = frequence[y][x].Re() * sign;
			mMesh->GetVertex(y * 64 + x)->Normal = Game::normalize(Game::Vector3(
				 - grandient_x[y][x].Re() * sign ,1. ,- grandient_z[y][x].Re()* sign
			));
		}
	}*/
	currentTime += deltaTime * .05;
	mRenderPass->UpdateTime(currentTime);
}