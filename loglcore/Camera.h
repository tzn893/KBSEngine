#pragma once

#include "d3dcommon.h"
#include "Math.h"

class Camera {
public:

	void initialize(float fovy,float aspect,float nearPlane,float farPlane,
		Game::Vector3 Pos,Game::Vector3 Rotation) {
		this->fovy = fovy, this->aspect = aspect, this->nearPlane = nearPlane,
			this->farPlane = farPlane;
		updatePerspect();
		this->Position = Pos, this->Rotation = Rotation;
		updateView();
	}
	
	void setFovy(float fovy) {
		this->fovy = fovy;
		updatePerspect();
	}
	void setAspect(float aspect){
		this->aspect = aspect;
		updatePerspect();
	}
	void setNearPlane(float nearPlane) {
		this->nearPlane = nearPlane;
		updatePerspect();
	}
	void setFarPlane(float farPlane) {
		this->farPlane = farPlane;
		updatePerspect();
	}

	void setPosition(Game::Vector3 Position) {
		this->Position = Position;
		updateView();
	}
	void setRotaion(Game::Vector3 Rotation) {
		this->Rotation = Rotation;
		updateView();
	}

	Game::Vector3 getPosition() { return Position; }
	Game::Vector3 getRotation() { return Rotation; }
	Game::Mat4x4 getViewMat() { return view; }
	Game::Mat4x4 getPerspectMat() { return perspect; }

	void SetTransform(Game::Mat4x4 trans) {
		Game::Vector3 trash;
		Game::UnpackTransfrom(trans, Position, Rotation, trash);
		view = trans.R();
	}

private:
	void updateView() {
		view = Game::PackTransfrom(this->Position, this->Rotation, Game::Vector3(1., 1., 1.));
		view = view.R();
	}
	void updatePerspect() {
		perspect = Game::MatrixProjection(aspect, PI * (fovy / 180.), nearPlane, farPlane);
	}

	Game::Vector3 Position;
	Game::Vector3 Rotation;

	Game::Mat4x4 view;

	float fovy,aspect,nearPlane,farPlane;
	Game::Mat4x4 perspect;
};