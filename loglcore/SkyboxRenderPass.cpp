#include "SkyboxRenderPass.h"

#include "graphic.h"
#include "GeometryGenerator.h"
#include "TextureManager.h"

#include "GenerateMipmapBatch.h"
#include "LightManager.h"

bool SkyboxRenderPass::Initialize(UploadBatch* batch) {
	Game::RootSignature rootSig(3, 1);
	rootSig[0].initAsConstantBuffer(0, 0);
	rootSig[1].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	rootSig[2].initAsConstants(1, 0, 3);
	rootSig.InitializeSampler(0, CD3DX12_STATIC_SAMPLER_DESC(0));

	if (!gGraphic.CreateRootSignature(rootSigName,&rootSig)) {
		OUTPUT_DEBUG_STRING("fail to create rootsignature for sky box render pass\n");
		return false;
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> layout = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
	};

	Shader* shader = gShaderManager.loadShader(L"../shader/Legacy/Skybox.hlsl",
		"VS", "PS", rootSigName, layout, L"skybox", nullptr);
	if (shader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for sky box render pass\n");
		return false;
	}

	Game::GraphicPSORP mPso = Game::GraphicPSORP::Default();
	D3D12_RASTERIZER_DESC rd = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rd.CullMode = D3D12_CULL_MODE_NONE;
	CD3DX12_DEPTH_STENCIL_DESC ds(D3D12_DEFAULT);
	ds.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	ds.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	mPso.SetRasterizerState(rd);
	mPso.SetDepthStencilState(ds);

	if (!gGraphic.CreatePipelineStateObjectRP(shader,&mPso,psoName)) {
		OUTPUT_DEBUG_STRING("fail to create pipeline state object for sky box render pass\n");
		return false;
	}

	std::vector<float> boxMesh = std::move(GeometryGenerator::Cube(1., 1., 1., GEOMETRY_FLAG_DISABLE_NORMAL | GEOMETRY_FLAG_DISABLE_TANGENT
		| GEOMETRY_FLAG_DISABLE_TEXCOORD));
	
	mBox = std::make_unique<StaticMesh<Game::Vector3>>(gGraphic.GetDevice(), boxMesh.size() / 3,
		reinterpret_cast<Game::Vector3*>(boxMesh.data()), batch);

	mHeap = std::make_unique<DescriptorHeap>(1);
	
	skybox = gTextureManager.loadCubeTexture(default_skybox_path, L"_default_skybox", true, batch);
	if (skybox == nullptr) return false;
	skybox->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	skyboxDesc = mHeap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, skybox->GetShaderResourceViewCPU());

	return true;
}


void   SkyboxRenderPass::Render(Graphic* graphic, RENDER_PASS_LAYER layer) {
	if (layer != RENDER_PASS_LAYER_BEFORE_ALL)  return;

	auto ambient = gLightManager.GetAmbientLight();

	graphic->BindPSOAndRootSignature(psoName, rootSigName);
	graphic->BindDescriptorHeap(mHeap->GetHeap());

	graphic->BindMainCameraPass(0);
	graphic->BindDescriptorHandle(skyboxDesc.gpuHandle,1);
	graphic->Bind32bitConstant(2, ambient, 0);

	graphic->Draw(mBox->GetVBV(), 0, mBox->GetVertexNum());
}

void   SkyboxRenderPass::SetSkyBox(Texture* tex) {
	if (tex->GetType() != TEXTURE_TYPE_2DCUBE) return;
	if (tex->GetShaderResourceViewCPU().ptr == 0) {
		tex->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	}
	mHeap->ClearUploadedDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	skyboxDesc = mHeap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, tex->GetShaderResourceViewCPU());

	skybox = tex;
	irradianceMap = nullptr;
}

Texture* SkyboxRenderPass::GetIrradianceMap() {
	if (irradianceMap == nullptr) {
		irradianceMap = std::make_unique<Texture>(skybox->GetWidth(), skybox->GetHeight(), TEXTURE_FORMAT_RGBA,
			TEXTURE_TYPE_2DCUBE, TEXTURE_FLAG_ALLOW_RENDER_TARGET);
		irradianceMap->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
		GenerateMipmapBatch::GenerateIBLIrradience(skybox->GetResource(),
			irradianceMap->GetResource(), skybox->GetShaderResourceViewCPU(),
			D3D12_RESOURCE_STATE_COMMON
		);
	}
	return irradianceMap.get();
}

Texture* SkyboxRenderPass::GetSpecularIBLMap() {
	if (specularMap == nullptr) {
		specularMap = std::make_unique<Texture>(skybox->GetWidth(), skybox->GetHeight(),
			5, TEXTURE_FORMAT_RGBA, TEXTURE_TYPE_2DCUBE,
			TEXTURE_FLAG_ALLOW_RENDER_TARGET);
		specularMap->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
		GenerateMipmapBatch::PrefilterEnvironment(
			skybox->GetResource(),specularMap->GetResource(),
			specularMapMipNum,skybox->GetShaderResourceViewCPU(),
			D3D12_RESOURCE_STATE_COMMON
		);
	}
	return specularMap.get();
}

void   SkyboxRenderPass::finalize() {
	mHeap.release();
	mBox.release();
}