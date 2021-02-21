#pragma once
#include "Vector.h"
#include "Matrix.h"

namespace Game {
	class Quaterion : public Vector4 {
	public:
		Quaterion() :Vector4() {}
		Quaterion(float x, float y, float z, float w) :Vector4(x, y, z, w) {}
		Quaterion(float* f) :Vector4(f) {}

		//don't call this function to construct the vector.
		Quaterion(__m128 m) : Vector4(m) {}

		Quaterion(const Vector3& vec, float w) :Vector4(vec, w) {}
		Quaterion(const Vector2& vec, float z, float w) :Vector4(vec, z, w) {}
		Quaterion(const Vector2& vec1, const Vector2& vec2) : Vector4(vec1, vec2) {}

		Quaterion(const Vector4& vec) { 
			memcpy(raw, vec.raw, sizeof(float) * 4);
		}

		static Quaterion Axis(Game::Vector3 axis, float angle);

		Quaterion Conj() const {
			return Quaterion(-x, -y, -z, w);
		}
		float     Dot(const Quaterion& other)const {
			return Game::dot((Game::Vector4)*this,(Game::Vector4)other);
		}

		static Quaterion SLerp(const Quaterion& lhs, const Quaterion& rhs, float factor);
		Game::Mat4x4  RotationMat() const;
	};
	Game::Mat4x4 PackTransformQuaterion(const Game::Vector3& Position,const Game::Quaterion& Rotation,
		const Game::Vector3& Scale);
}