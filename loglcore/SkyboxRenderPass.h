#pragma once
#include "RenderPass.h"
#include "Texture.h"
#include "DescriptorAllocator.h"
#include "Mesh.h"

class SkyboxRenderPass : public RenderPass {
public:
	virtual size_t GetPriority() { return 5; }

	virtual bool   Initialize(UploadBatch* batch = nullptr) override ;
	virtual void   Render(Graphic* graphic, RENDER_PASS_LAYER layer) override;

	virtual void   finalize() override;

	Texture* GetSkyBox() { return skybox; }
	Texture* GetIrradianceMap();
	Texture* GetSpecularIBLMap();
	void     SetSkyBox(Texture* tex);

	void	 CreateIrradianceMap();
	void	 CreateSpecularIBLMap();

private:
	const wchar_t* default_skybox_path = L"../asserts/skybox/clear";

	Texture* skybox;
	Descriptor skyboxDesc;
	std::unique_ptr<DescriptorHeap> mHeap;
	std::unique_ptr<StaticMesh<Game::Vector3>> mBox;
	
	const wchar_t* rootSigName = L"skybox";
	const wchar_t* psoName = L"skybox";
	std::unique_ptr<Texture> irradianceMap;
	std::unique_ptr<Texture> specularMap;
	static constexpr size_t specularMapMipNum = 5;
};