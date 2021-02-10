#include "ToonRenderPass.h"
#include "graphic.h"
#include "TextureManager.h"
#include "LightManager.h"

bool ToonRenderPass::Initialize(UploadBatch* up) {
	Game::RootSignature shadingRootSig(5, 1);
	shadingRootSig[0].initAsConstantBuffer(0, 0);
	shadingRootSig[1].initAsConstantBuffer(1, 0);
	shadingRootSig[2].initAsConstantBuffer(2, 0);
	shadingRootSig[3].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	shadingRootSig[4].initAsConstants(3, 0, 3);
	shadingRootSig.InitializeSampler(0, CD3DX12_STATIC_SAMPLER_DESC(0));

	if (!gGraphic.CreateRootSignature(toon_render_pass_shading,
			&shadingRootSig)) {
		OUTPUT_DEBUG_STRING("fail to create pso for toon render pass\n");
		return false;
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> layout = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,32,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};

	Shader* shading = gShaderManager.loadShader(L"../shader/ToonShading.hlsl",
		"VS", "PS", toon_render_pass_shading, layout, toon_render_pass_shading, nullptr);
	if (shading == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for toon render pass\n");
		return false;
	}

	Game::GraphicPSORP mShadingPso = Game::GraphicPSORP::Default();
	if (!gGraphic.CreatePipelineStateObjectRP(shading,&mShadingPso,toon_render_pass_shading)) {
		OUTPUT_DEBUG_STRING("fail to create shader for toon render pass\n");
		return false;
	}


	Game::RootSignature rootSig(4,1);
	rootSig[0].initAsConstantBuffer(0, 0);
	rootSig[1].initAsConstantBuffer(1, 0);
	rootSig[2].initAsConstants(2, 0, 5);
	rootSig[3].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	rootSig.InitializeSampler(0, CD3DX12_STATIC_SAMPLER_DESC(0));
	
	if (!gGraphic.CreateRootSignature(toon_render_pass_outline,&rootSig)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for toon outline\n");
		return false;
	}

	Shader* outline = gShaderManager.loadShader(L"../shader/ToonOutline.hlsl",
		"VS", "PS", toon_render_pass_outline, layout, toon_render_pass_outline,
		nullptr);
	if (outline == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for toon outline\n");
		return false;
	}
	
	Game::GraphicPSORP pso = Game::GraphicPSORP::Default();
	CD3DX12_RASTERIZER_DESC rd(D3D12_DEFAULT);
	rd.CullMode = D3D12_CULL_MODE_FRONT;
	pso.SetRasterizerState(rd);
	
	if (!gGraphic.CreatePipelineStateObjectRP(outline,&pso,toon_render_pass_outline)) {
		OUTPUT_DEBUG_STRING("fail to create pso for toon outline\n");
		return false;
	}

	ID3D12Device* device = gGraphic.GetDevice();
	mObjectPasses = std::make_unique<ConstantBuffer<ObjectPass>>(device, objectBufferSize);
	descriptorHeap = std::make_unique<DescriptorHeap>(objectBufferSize);

	outlineWidth = .005;
	outlineColor = Game::Vector3(.5, .5, .5);
	div.x = 0.;

	return true;
}

void ToonRenderPass::Render(Graphic* graphic, RENDER_PASS_LAYER layer) {
	if (tdoList.empty() || layer != RENDER_PASS_LAYER_OPAQUE) {
		return;
	}

	graphic->BindPSOAndRootSignature(toon_render_pass_shading, toon_render_pass_shading);
	graphic->BindMainCameraPass(1);
	gLightManager.BindLightPass2ConstantBuffer(2);
	graphic->BindDescriptorHeap(descriptorHeap->GetHeap());
	graphic->Bind32bitConstant(4, div, 0);
	for (auto& tdo : tdoList) {
		graphic->BindDescriptorHandle(tdo.diffuse, 3);
		graphic->BindConstantBuffer(mObjectPasses->GetADDR(tdo.objectID), 0);
		if (tdo.ibv == nullptr) {
			graphic->Draw(tdo.vbv, tdo.start, tdo.num);
		}
		else {
			graphic->Draw(tdo.vbv, tdo.ibv, tdo.start, tdo.num);
		}
	}

	graphic->BindPSOAndRootSignature(toon_render_pass_outline, toon_render_pass_outline);
	graphic->BindMainCameraPass(1);
	graphic->Bind32bitConstant(2, outlineColor, 0);
	graphic->Bind32bitConstant(2, outlineWidth, 4);
	graphic->Bind32bitConstant(2, gGraphic.GetHeightWidthRatio(), 3);
	for (auto& tdo : tdoList) {
		graphic->BindConstantBuffer(mObjectPasses->GetADDR(tdo.objectID), 0);
		graphic->BindDescriptorHandle(tdo.diffuse, 3);
		if (tdo.ibv == nullptr) {
			graphic->Draw(tdo.vbv, tdo.start, tdo.num);
		}
		else {
			graphic->Draw(tdo.vbv, tdo.ibv, tdo.start, tdo.num);
		}
	}

	descriptorHeap->ClearUploadedDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	tdoList.clear();
}

void ToonRenderPass::finalize() {
	mObjectPasses = nullptr;
	descriptorHeap = nullptr;
}

ToonRenderObjectID ToonRenderPass::AllocateToonROID() {
	if (!avaliableToonRenderObjects.empty()) {
		ToonRenderObjectID tro = *(avaliableToonRenderObjects.end() - 1);
		avaliableToonRenderObjects.pop_back();
		return tro + 1;
	}
	else if(allocatedObjectNum < objectBufferSize){
		return ++allocatedObjectNum;
	}
	return 0;
}
bool ToonRenderPass::DeallocateToonROID(ToonRenderObjectID& id) {
	id = id - 1;
	if (id < allocatedObjectNum &&
		std::find(avaliableToonRenderObjects.begin(),avaliableToonRenderObjects.end(),id)
		== avaliableToonRenderObjects.end()) {
		avaliableToonRenderObjects.push_back(id);
		id = 0;
		return true;
	}
	return false;
}

ObjectPass* ToonRenderPass::GetObjectPass(ToonRenderObjectID id) {
	id = id - 1;
	if (id < allocatedObjectNum) {
		return mObjectPasses->GetBufferPtr(id);
	}
	return nullptr;
}

void ToonRenderPass::Draw(D3D12_VERTEX_BUFFER_VIEW* vbv, D3D12_INDEX_BUFFER_VIEW* ibv,
	size_t start, size_t num, ToonRenderObjectID id, ToonPassMaterial* tex ) {
	id -= 1;
	if (id >= allocatedObjectNum) return;
	ToonDrawObject tdo;
	tdo.objectID = id;
	tdo.start = start;
	tdo.num = num;
	tdo.vbv = vbv;
	tdo.ibv = ibv;
	
	if (tex == nullptr || tex->diffuse == nullptr) {
		Texture* white = gTextureManager.getWhiteTexture();
		tdo.diffuse = descriptorHeap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			white->GetShaderResourceViewCPU()).gpuHandle;
	}
	else {
		if (tex->diffuse->GetShaderResourceViewCPU().ptr == 0) {
			tex->diffuse->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
		}
		tdo.diffuse = descriptorHeap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			tex->diffuse->GetShaderResourceViewCPU()).gpuHandle;
	}

	tdoList.push_back(tdo);

}