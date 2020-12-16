#include "PhongRenderPass.h"
#include "graphic.h"
#include "PipelineStateObject.h"


bool PhongRenderPass::Initialize(UploadBatch* batch) {
	
	Game::RootSignature rootSig(3,0);

	rootSig[0].initAsConstantBuffer(0, 0);
	rootSig[1].initAsConstantBuffer(1, 0);
	rootSig[2].initAsConstantBuffer(2, 0);

	if (!gGraphic.CreateRootSignature(rootSigName,&rootSig)) {
		OUTPUT_DEBUG_STRING("fail to create rootsignature for phong render pass\n");
		return false;
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> layout = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,32,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};


	D3D_SHADER_MACRO macro[] = {
		"MAX_LIGHT_STRUCT_NUM",getShaderMaxLightStructNumStr,
		"LIGHT_TYPE_POINT",getShaderLightTypeStr<SHADER_LIGHT_TYPE_POINT>(),
		"LIGHT_TYPE_DIRECIONAL",getShaderLightTypeStr<SHADER_LIGHT_TYPE_DIRECTIONAL>(),
		nullptr,nullptr
	};

	Shader* shader = gShaderManager.loadShader(L"../shader/PhongLighting.hlsl", "VS", "PS",
		rootSigName.c_str(), layout, rootSigName.c_str(),macro);

	if (shader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for phong lighting\n");
		return false;
	}

	Game::GraphicPSORP mPso;
	mPso.LazyBlendDepthRasterizeDefault();
	mPso.SetNodeMask(0);
	mPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	mPso.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);

	if (!gGraphic.CreatePipelineStateObjectRP(shader,&mPso,psoName.c_str())) {
		OUTPUT_DEBUG_STRING("fail to create pso for phong lighting\n");
		return false;
	}

	Game::RootSignature texRootSig(5,1);
	texRootSig[0].initAsConstantBuffer(0, 0);
	texRootSig[1].initAsConstantBuffer(1, 0);
	texRootSig[2].initAsConstantBuffer(2, 0);

	texRootSig[3].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	texRootSig[4].initAsDescriptorTable(1, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	texRootSig.InitializeSampler(0, CD3DX12_STATIC_SAMPLER_DESC(0));

	if (!gGraphic.CreateRootSignature(texRootSigName,&texRootSig)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for phong lighting\n");
		return false;
	}

	D3D_SHADER_MACRO texMacro[] = {
		"MAX_LIGHT_STRUCT_NUM",getShaderMaxLightStructNumStr,
		"LIGHT_TYPE_POINT",getShaderLightTypeStr<SHADER_LIGHT_TYPE_POINT>(),
		"LIGHT_TYPE_DIRECIONAL",getShaderLightTypeStr<SHADER_LIGHT_TYPE_DIRECTIONAL>(),
		"ENABLE_DIFFUSE_MAP","1",
		nullptr,nullptr
	};


	Shader* texShader = gShaderManager.loadShader(L"../shader/PhongLighting.hlsl", "VS", "PS",
		texRootSigName.c_str(), layout, texRootSigName.c_str(),texMacro);

	if (texShader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for phong lighting\n");
		return false;
	}

	Game::GraphicPSORP mTexPso;
	mTexPso.LazyBlendDepthRasterizeDefault();
	mTexPso.SetNodeMask(0);
	mTexPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	mTexPso.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);

	if (!gGraphic.CreatePipelineStateObjectRP(texShader, &mTexPso, texPsoName.c_str())) {
		OUTPUT_DEBUG_STRING("fail to create pipeline state object for phong lighting\n");
		return false;
	}

	ID3D12Device* mDevice = gGraphic.GetDevice();

	lightPass = std::make_unique<ConstantBuffer<LightPass>>(mDevice);
	objectPass = std::make_unique<ConstantBuffer<ObjectPass>>(mDevice,objBufferSize);

	mHeap = std::make_unique<DescriptorHeap>(128);

	Game::RootSignature shadowRootSig(2,0);
	shadowRootSig[0].initAsConstantBuffer(0, 0);
	shadowRootSig[1].initAsConstantBuffer(1, 0);
	if (!gGraphic.CreateRootSignature(shadowRootSigName,&shadowRootSig)) {
		OUTPUT_DEBUG_STRING("fail to create shadow pass root signature for shadow pass");
		return false;
	}

	Shader* shadowPassShader = gShaderManager.loadShader(
		L"../shader/ShadowMap.hlsl", "VS", "PS",
		shadowRootSigName.c_str(), layout, L"shadowPass");
	if (shadowPassShader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for shadow pass\n");
		return false;
	}

	Game::GraphicPSO ShadowPSO;
	ShadowPSO.LazyBlendDepthRasterizeDefault();
	ShadowPSO.SetNodeMask(0);
	ShadowPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	ShadowPSO.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
	ShadowPSO.SetRenderTargetFormat(DXGI_FORMAT_R32_FLOAT);
	ShadowPSO.SetDepthStencilViewFomat(DXGI_FORMAT_D24_UNORM_S8_UINT);

	if (!gGraphic.CreatePipelineStateObject(shadowPassShader,&ShadowPSO,shadowPsoName.c_str())) {
		OUTPUT_DEBUG_STRING("fail to create pipeline state object for shadow pipeline\n");
		return false;
	}

	mDepthLightView = std::make_unique<ConstantBuffer<ShadowLightPass>>(mDevice);
	size_t mWinWidth = gGraphic.GetScreenWidth(), mWinHeight = gGraphic.GetScreenHeight();
	mDepthRTVTex = std::make_unique<Texture>(mWinWidth, mWinHeight,
		TEXTURE_FORMAT_FLOAT,TEXTURE_FLAG_ALLOW_RENDER_TARGET);
	mDepthDSVTex = std::make_unique<Texture>(mWinWidth, mWinHeight,
		TEXTURE_FORMAT_DEPTH_STENCIL, TEXTURE_FLAG_ALLOW_DEPTH_STENCIL);

	mDepthRTVTex->CreateRenderTargetView(mHeap->Allocate(1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	mDepthRTVTex->CreateShaderResourceView(mHeap->Allocate(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	mDepthDSVTex->CreateDepthStencilView(mHeap->Allocate(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
	
	return true;
}

void PhongRenderPass::Render(Graphic* graphic, RENDER_PASS_LAYER layer) {
	if (layer != RENDER_PASS_LAYER_OPAQUE) return;

	if (!objQueue.empty()) {
		graphic->BindPSOAndRootSignature(psoName.c_str(), rootSigName.c_str());
		graphic->BindMainCameraPass();
		graphic->BindConstantBuffer(lightPass->GetADDR(), 2);

		for (auto& ele : objQueue) {
			graphic->BindConstantBuffer(objectPass->GetADDR(ele.objectID), 0);
			if (ele.ibv != nullptr) {
				graphic->Draw(ele.vbv, ele.ibv, ele.start, ele.num);
			}
			else {
				graphic->Draw(ele.vbv, ele.start, ele.num);
			}
		}
	}
	if (!objTexQueue.empty()) {

		graphic->BindPSOAndRootSignature(texPsoName.c_str(), texRootSigName.c_str());
		graphic->BindMainCameraPass();
		graphic->BindConstantBuffer(lightPass->GetADDR(), 2);
		ID3D12DescriptorHeap* heaps[] = { mHeap->GetHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) };
		graphic->BindDescriptorHeap(heaps,_countof(heaps));

		for (auto& ele : objTexQueue) {
			graphic->BindConstantBuffer(objectPass->GetADDR(ele.objectID), 0);
			graphic->BindDescriptorHandle(ele.diffuseMap, 3);
			graphic->BindDescriptorHandle(ele.normalMap, 4);
			if (ele.ibv != nullptr) {
				graphic->Draw(ele.vbv, ele.ibv, ele.start, ele.num);
			}
			else {
				graphic->Draw(ele.vbv, ele.start, ele.num);
			}
		}
	}

	objQueue.clear();
	objTexQueue.clear();
	mHeap->Clear(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void PhongRenderPass::finalize() {
	lightPass.release();
	objectPass.release();
}

void PhongRenderPass::BindLightData(LightData* data,size_t offset, size_t num ) {
	if (offset + num > SHADER_MAX_LIGHT_STRUCT_NUM) return;

	for (size_t i = 0; i != num; i++) {
		//the light numbered 0 must be a directional light
		if (i == mainLightIndex && data[i].type != SHADER_LIGHT_TYPE_DIRECTIONAL) {
			continue;
		}
		lightPass->GetBufferPtr()->lights[offset] = data[i];
	}
}

void PhongRenderPass::BindAmbientLightData(Game::Vector3 color) {
	lightPass->GetBufferPtr()->ambient = Game::Vector4(color, 1.);
}

PhongObjectID PhongRenderPass::AllocateObjectPass() {
	if (avaliableObjectBuffers.empty()) {
		if (allocatedBufferNum == objBufferSize) {
			objBufferSize *= 2;
			ID3D12Device* mDevice = gGraphic.GetDevice();
			objectPass->Resize(mDevice, objBufferSize);
		}
		allocatedBufferNum++;
		return allocatedBufferNum - 1;
	}
	else {
		PhongObjectID rv = avaliableObjectBuffers[avaliableObjectBuffers.size() - 1];
		avaliableObjectBuffers.pop_back();
		return rv;
	}

	return -1;
}

void PhongRenderPass::DeallocateObjectPass(PhongObjectID& id) {
	if (id >= allocatedBufferNum) return;
	if (
		auto iter = std::find(avaliableObjectBuffers.begin(), avaliableObjectBuffers.end(), id);
		iter != avaliableObjectBuffers.end()
		) {
		return;
	}

	avaliableObjectBuffers.push_back(id);
	sizeof(CameraPass);
	id = 0;
}

ObjectPass* PhongRenderPass::GetObjectPass(PhongObjectID id) {
	if (allocatedBufferNum <= id) {
		return nullptr;
	}
	return objectPass->GetBufferPtr(id);
}

void PhongRenderPass::DrawObject(D3D12_VERTEX_BUFFER_VIEW* vbv, size_t start, size_t num,
	PhongObjectID id,PhongMaterialTexture* tex) {
	if (allocatedBufferNum <= id) {
		return;
	}

	ObjectElement oe;
	oe.ibv = nullptr;
	oe.vbv = vbv;
	oe.objectID = id;
	oe.start = start;
	oe.num = num;
	if (tex != nullptr) {
		Descriptor desc = mHeap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, tex->diffuseMap->GetShaderResourceViewCPU());
		oe.diffuseMap = desc.gpuHandle;
		desc = mHeap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, tex->normalMap->GetShaderResourceViewCPU());
		oe.normalMap = desc.gpuHandle;
		objTexQueue.push_back(oe);
	}
	else {
		objQueue.push_back(oe);
	}
}

void PhongRenderPass::DrawObject(D3D12_VERTEX_BUFFER_VIEW* vbv, D3D12_INDEX_BUFFER_VIEW* ibv,
	size_t start, size_t num, PhongObjectID id,PhongMaterialTexture* tex) {
	if (allocatedBufferNum <= id) {
		return;
	}

	ObjectElement oe;
	oe.ibv = ibv;
	oe.vbv = vbv;
	oe.objectID = id;
	oe.start = start;
	oe.num = num;
	if (tex != nullptr) {
		Descriptor desc = mHeap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, tex->diffuseMap->GetShaderResourceViewCPU());
		oe.diffuseMap = desc.gpuHandle; 
		desc = mHeap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, tex->normalMap->GetShaderResourceViewCPU());
		oe.normalMap = desc.gpuHandle;
		objTexQueue.push_back(oe);
	}
	else {
		objQueue.push_back(oe);
	}
}