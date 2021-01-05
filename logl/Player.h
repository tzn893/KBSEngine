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
private:
	Game::Vector3 vecForward();
	void UpdateTransform();


	FPSCamera camera;
	//this camera is used to compute the object's movement
	FPSCamera moveCamera;
	std::unique_ptr<RenderObject> ro;

	static constexpr float cameraDis = 2.;
	static constexpr float camSpeed = 10.;
	float camTheta, camBeta;

	Game::Vector2 mousePos;
	float rangeY = 15.;
	float accY = 0.;
	float drawBackSpeed = .01;

};