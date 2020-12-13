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
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
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
	
	CD3DX12_RASTERIZER_DESC rd(D3D12_DEFAULT);
	rd.CullMode = D3D12_CULL_MODE_NONE;
	mPso.SetRasterizerState(rd);

	mPso.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);

	if (!gGraphic.CreatePipelineStateObject(shader,&mPso,psoName.c_str())) {
		OUTPUT_DEBUG_STRING("fail to create pso for phong lighting\n");
		return false;
	}

	ID3D12Device* mDevice = gGraphic.GetDevice();

	lightPass = std::make_unique<ConstantBuffer<LightPass>>(mDevice);
	objectPass = std::make_unique<ConstantBuffer<ObjectPass>>(mDevice,objBufferSize);

	return true;
}

void PhongRenderPass::Render(Graphic* graphic, RENDER_PASS_LAYER layer) {
	if (layer != RENDER_PASS_LAYER_OPAQUE) return;

	if (objQueue.empty()) return;

	graphic->BindPSOAndRootSignature(psoName.c_str(), rootSigName.c_str());
	graphic->BindMainCameraPass();
	graphic->BindConstantBuffer(lightPass->GetADDR(), 2);

	for (auto& ele : objQueue) {
		graphic->BindConstantBuffer(objectPass->GetADDR(ele.objectID), 0);
		if (ele.ibv != nullptr) {
			graphic->Draw(ele.vbv, ele.ibv, ele.start, ele.num);
		}else {
			graphic->Draw(ele.vbv, ele.start, ele.num);
		}
	}

	objQueue.clear();
}

void PhongRenderPass::finalize() {
	lightPass.release();
	objectPass.release();
}

void PhongRenderPass::BindLightData(LightData* data,size_t offset, size_t num ) {
	if (offset + num > SHADER_MAX_LIGHT_STRUCT_NUM) return;

	for (size_t i = 0; i != num; i++) {
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

void PhongRenderPass::DeallocateObjectPass(PhongObjectID id) {
	if (id >= allocatedBufferNum) return;
	if (
		auto iter = std::find(avaliableObjectBuffers.begin(), avaliableObjectBuffers.end(), id);
		iter != avaliableObjectBuffers.end()
		) {
		return;
	}

	avaliableObjectBuffers.push_back(id);
	sizeof(CameraPass);
}

ObjectPass* PhongRenderPass::GetObjectPass(PhongObjectID id) {
	if (allocatedBufferNum <= id) {
		return nullptr;
	}
	return objectPass->GetBufferPtr(id);
}

void PhongRenderPass::DrawObject(D3D12_VERTEX_BUFFER_VIEW* vbv, size_t start, size_t num,
	PhongObjectID id) {
	if (allocatedBufferNum <= id) {
		return;
	}

	ObjectElement oe;
	oe.ibv = nullptr;
	oe.vbv = vbv;
	oe.objectID = id;
	oe.start = start;
	oe.num = num;

	objQueue.push_back(oe);
}

void PhongRenderPass::DrawObject(D3D12_VERTEX_BUFFER_VIEW* vbv, D3D12_INDEX_BUFFER_VIEW* ibv,
	size_t start, size_t num, PhongObjectID id) {
	if (allocatedBufferNum <= id) {
		return;
	}

	ObjectElement oe;
	oe.ibv = ibv;
	oe.vbv = vbv;
	oe.objectID = id;
	oe.start = start;
	oe.num = num;

	objQueue.push_back(oe);
}