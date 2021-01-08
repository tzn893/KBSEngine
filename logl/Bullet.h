#pragma once
#include "../loglcore/RenderPass.h"
#include "../loglcore/ConstantBuffer.h"
#include "../loglcore/Math.h"
#include "../loglcore/Mesh.h"


class BulletRenderPass : public RenderPass{
public:
	virtual size_t GetPriority() override { return 30; }

	virtual bool   Initialize(UploadBatch* batch = nullptr) override;
	virtual void   Render(Graphic* graphic, RENDER_PASS_LAYER layer) override;
	
	virtual void   finalize() override;

	void UpdateBulletPositions(size_t num,Game::Vector3* Positions);
private:
	struct BulletWorld {
		Game::Vector4 Position;
	};
	size_t maxRenderedBullets = 100;
	std::vector<Game::Vector3> updatedBulletPosition;
	size_t bulletNum = 0;
	std::unique_ptr<ConstantBuffer<BulletWorld>> bulletConstant;
	std::unique_ptr<StaticMesh<Game::Vector3>> bulletMesh;
};