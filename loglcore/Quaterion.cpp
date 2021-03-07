#include "Quaterion.h"
#include "MathFunctions.h"
using namespace Game;

#include <DirectXMath.h>

Quaterion Quaterion::SLerp(const Quaterion& lhs, const Quaterion& rhs, float factor) {
	float cosTheta = fmin(lhs.Dot(rhs),1.);
	float sinTheta = sqrt(1. - cosTheta * cosTheta);
	
	if (sinTheta < 1e-4) {
		Game::Vector4 q = (lhs * (1. - factor) + (rhs * factor));
		return Game::normalize(q);
	}
	
	float theta = get_angle(sinTheta, cosTheta);
	float lsf = sin(theta * (1. - factor)), rsf = sin(theta * factor);

	return Game::Quaterion((lhs * lsf + rhs * rsf) / sinTheta);
}

Game::Mat4x4  Quaterion::RotationMat() const {
	float q4q4m1 = w * w * 2 - 1.;
	float q1q2 = 2 * x * y;
	float q3q4 = 2 * z * w;
	float q1q3 = 2 * x * z;
	float q2q4 = 2 * y * w;
	float q2q3 = 2 * y * z;
	float q1q4 = 2 * x * w;

	float mat[] = {
		2 * x * x + q4q4m1,q1q2 - q3q4       ,q1q3 + q2q4       ,0,
		q1q2 + q3q4		  ,2 * y * y + q4q4m1,q2q3 - q1q4       ,0,
		q1q3 - q2q4		  ,q2q3 + q1q4		 ,2 * z * z + q4q4m1,0,
		0				  ,0				 ,0					,1
	};

	return Mat4x4(mat);
}

Mat4x4 Game::PackTransformQuaterion(const Game::Vector3& Position, const Game::Quaterion& Rotation,
	const Game::Vector3& Scale) {
	Mat4x4 Res = mul(Rotation.RotationMat(), MatrixScale(Scale));
	Res.a[0][3] = Position.x, Res.a[1][3] = Position.y, Res.a[2][3] = Position.z;
	return Res;
}

Quaterion Quaterion::Axis(Game::Vector3 axis,float angle) {
	angle = angle * PI / 360.;
	return Quaterion(Game::normalize(axis) * cosf(angle), sinf(angle));
}