#pragma once
#include "../loglcore/Camera.h"
#include "../loglcore/Math.h"

class FPSCamera
{
public:

	void attach(Camera* camera);

	FPSCamera(Camera* camera = nullptr) {
		attach(camera);
	}

	inline Camera* getSceneCamera() {
		return camera;
	}
	void walk(float distance);
	void strafe(float distance);

	void rotateY(float angle);
	void rotateX(float angle);

	void look(Game::Vector3 Position);

	void setPosition(Game::Vector3 pos);
private:
	void updateAxis();
	void updateAngle();

	void update();

	Camera* camera;

	Game::Vector3 Position;

	Game::Vector3 lookAt;
	Game::Vector3 up;
	Game::Vector3 biAxis;

	float theta = 0.;
	float phi = 0.;
};