#include "Bullet.h"
#include "../loglcore/RootSignature.h"
#include "../loglcore/PipelineStateObject.h"
#include "../loglcore/graphic.h"
#include "../loglcore/Shader.h"
#include "../loglcore/GeometryGenerator.h"

#include <algorithm>

bool BulletRenderPass::Initialize(UploadBatch* batch) {
	Game::RootSignature rootSig(2,0);
	rootSig[0].initAsShaderResource(0, 0);
	rootSig[1].initAsConstantBuffer(0, 0);

	if (!gGraphic.CreateRootSignature(L"bullet",&rootSig)) {
		OUTPUT_DEBUG_STRING("fail to create rootsignature for bullet\n");
		return false;
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> layout = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
	};

	Shader* shader = gShaderManager.loadShader(L"../shader/Custom/Bullet.hlsl", "VS", "PS",
		L"bullet", layout, L"bullet", nullptr);
	if (shader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to load shader for bullet\n");
		return false;
	}

	Game::GraphicPSORP pso = Game::GraphicPSORP::Default();
	if (!gGraphic.CreatePipelineStateObjectRP(shader,&pso,L"bullet")) {
		OUTPUT_DEBUG_STRING("fail to load pso for bullet\n");
		return false;
	}

	bulletConstant = std::make_unique<ConstantBuffer<BulletWorld>>(gGraphic.GetDevice(), maxRenderedBullets, true);

	auto[vertices, indices] = GeometryGenerator::Sphere(.1,12, GEOMETRY_FLAG_DISABLE_NORMAL | GEOMETRY_FLAG_DISABLE_TANGENT
		| GEOMETRY_FLAG_DISABLE_TEXCOORD);
	bulletMesh = std::make_unique<StaticMesh<Game::Vector3>>(gGraphic.GetDevice(),
		indices.size(), indices.data(),
		vertices.size() / 3, reinterpret_cast<Game::Vector3*>(vertices.data()),batch
	);
	updatedBulletPosition.clear();
	bulletNum = 0;
	return true;
}


void BulletRenderPass::UpdateBulletPositions(size_t num,Game::Vector3* position) {
	updatedBulletPosition.clear();
	bulletNum = 0;
	if (num == 0) return;
	updatedBulletPosition.insert(updatedBulletPosition.begin(), position, position + num);
	Game::Vector3 cameraPos = gGraphic.GetMainCamera()->getPosition();
	std::sort(updatedBulletPosition.begin(),updatedBulletPosition.end(),
		[&](const Game::Vector3& lhs,const Game::Vector3& rhs) {
			Game::Vector3 l = lhs - cameraPos, r = rhs - cameraPos;
			return dot(l, l) < dot(r, r);
		}
	);
	bulletNum = maxRenderedBullets < num ? maxRenderedBullets : num;
	for (size_t i = 0; i != bulletNum; i++) {
		bulletConstant->GetBufferPtr(i)->Position = Game::Vector4(updatedBulletPosition[i],0.);
	}
}

void BulletRenderPass::Render(Graphic* graphic,RENDER_PASS_LAYER layer) {
	if (layer != RENDER_PASS_LAYER_OPAQUE) {return;}
	if (bulletNum == 0) return;

	graphic->BindPSOAndRootSignature(L"bullet", L"bullet");
	graphic->BindShaderResource(bulletConstant->GetADDR(), 0);
	graphic->BindMainCameraPass(1);
	
	graphic->DrawInstance(bulletMesh->GetVBV(),bulletMesh->GetIBV(), 0, bulletMesh->GetIndexNum(),bulletNum);
	updatedBulletPosition.clear();
	bulletNum = 0;
}

void BulletRenderPass::finalize() {
	bulletConstant.release();
	bulletMesh.release();
}