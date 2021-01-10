#pragma once
#include "../loglcore/Model.h"
#include "../loglcore/RenderObject.h"
#include "FPSCamera.h"
#include "../loglcore/RenderPass.h"
#include "../loglcore/Texture.h"
#include "../loglcore/DescriptorAllocator.h"


class PlayerPostEffect : public RenderPass {
public:
	virtual size_t GetPriority() { return 0; }

	virtual void   PreProcess() {}
	virtual bool   Initialize(UploadBatch* batch = nullptr) override;
	virtual void   Render(Graphic* graphic, RENDER_PASS_LAYER layer) {}
	virtual void   PostProcess(ID3D12Resource* renderTarget) override;

	virtual void   finalize() override;

	void SetHealthFactor(float health);
private:
	struct TextureConstantBuffer {
		float width, height;
		float healthFactor;
	};
	std::unique_ptr<Texture> texture;
	std::unique_ptr<DescriptorHeap> mHeap;
	std::unique_ptr<ConstantBuffer<TextureConstantBuffer>> mBuffer;
};

class Player {
public:
	void Update();
	Game::Vector3 GetWorldPosition();
	Game::Vector3 GetWorldRotation();
	Game::Vector3 GetWorldScale();
	
	void SetWorldPosition(Game::Vector3 pos);
	void SetWorldRotation(Game::Vector3 rot);
	void SetWorldScale(Game::Vector3 scale);

	Player(Game::Vector3 Position,Game::Vector3 Scale,
		Model* model);

	bool ShootSignal();
	std::pair<Game::Vector3, Game::Vector3> Bullet();

	void Shooted(float damage);

	float GetHealth() { return health; }
	void SetHealth(float health) { this->health = health; }
private:
	Game::Vector3 vecForward();
	void UpdateTransform();
	void Shoot();

	FPSCamera camera;
	//this camera is used to compute the object's movement
	FPSCamera moveCamera;
	std::unique_ptr<RenderObject> ro;
	std::unique_ptr<PlayerPostEffect> postEffect;

	static constexpr float cameraDis = 2.;
	static constexpr float camSpeed = 10.;
	float camTheta, camBeta;

	Game::Vector2 mousePos;
	float rangeY = 45.;
	float accY = 0.;
	float drawBackSpeed = .01;
	float rotateSpeedY = 90.;
	float rotateSpeedX = 240.;

	float speedSlow = 1.f;
	float speedFast = 5.f;
	float speedLerp = 0.f;
	float speedAcc = 10.f;

	static constexpr float shootCD = .2f;
	bool  shootSignal;
	float shootTimer = 0.f;

	float shootOffset = .3;

	float recoverTimer = 0.;
	static constexpr float recoverTime = 2.;
	float health = 100.;
	static constexpr float recoverSpeed = 50.;
};