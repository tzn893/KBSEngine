#include "Quaterion.h"
using namespace Game;

Quaterion SLerp(const Quaterion& lhs, const Quaterion& rhs, float factor) {
	return Quaterion();
}
Game::Mat4x4  Quaterion::RotationMat() const {
	return Mat4x4();
}

Mat4x4 Game::PackTransformQuaterion(const Game::Vector3& Position, const Game::Quaterion& Rotation,
	const Game::Vector3& Scale) {
	Mat4x4 Res = mul(Rotation.RotationMat(), MatrixScale(Scale));
	Res.a[0][3] = Position.x, Res.a[1][3] = Position.y, Res.a[2][3] = Position.z;
	return Res;
}