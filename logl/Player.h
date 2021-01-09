#pragma once
#include "../loglcore/Model.h"
#include "../loglcore/RenderObject.h"
#include "FPSCamera.h"

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
private:
	Game::Vector3 vecForward();
	void UpdateTransform();
	void Shoot();

	FPSCamera camera;
	//this camera is used to compute the object's movement
	FPSCamera moveCamera;
	std::unique_ptr<RenderObject> ro;

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
};