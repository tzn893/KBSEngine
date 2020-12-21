#include "../loglcore/d3dcommon.h"
#include "../loglcore/Mesh.h"
#include "../loglcore/ShaderDataStruct.h"

#include "../loglcore/RenderPass.h"
#include "../loglcore/Shader.h"
#include "../loglcore/ConstantBuffer.h"

class FFTWave;

class FFTWaveRenderPass : public RenderPass {
public:

	virtual size_t GetPriority() { return 10; }

	virtual bool   Initialize(UploadBatch* batch = nullptr) override;
	virtual void   Render(Graphic* graphic, RENDER_PASS_LAYER layer) override;

	virtual void   finalize() override;

	void Attach(FFTWave* wave) {
		this->wave = wave;
	}

	void SetWorldTransform(Game::Vector3 position,Game::Vector3 rotation,Game::Vector3 scale);
	LightPass* GetLightPass() { return lightPass->GetBufferPtr(); }
private:
	FFTWave* wave;
	Shader* fftWaveShader;

	struct FFTWaveObjectPass {
		Game::Mat4x4 world;
		Game::Mat4x4 transInvWorld;
	};
	std::unique_ptr<ConstantBuffer<FFTWaveObjectPass>> objectPass;
	std::unique_ptr<ConstantBuffer<LightPass>> lightPass;
};

class FFTWave {
	friend class FFTWaveRenderPass;
public:
	bool Initialize(float width, float height);
	void Update(float deltaTime);

	void SetPosition(Game::Vector3 position) { this->position = position; transformUpdated = true; }
	void SetRotation(Game::Vector3 rotation) { this->rotation = rotation; transformUpdated = true;}
	void SetScale(Game::Vector3 scale) { this->scale = scale; transformUpdated = true;}

	FFTWaveRenderPass* GetRenderPass() { return mRenderPass.get(); }
private:
	static constexpr size_t rowNum = 64;
	std::unique_ptr<DynamicMesh<MeshVertex>> mMesh;
	std::unique_ptr<FFTWaveRenderPass> mRenderPass;

	float currentTime;

	Game::Vector3 position;
	Game::Vector3 rotation;
	Game::Vector3 scale;
	bool transformUpdated = false;
};