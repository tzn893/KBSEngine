#include "Vector.h"

inline __m128 dot(__m128 lhs, __m128 rhs) {
	__m128 temp1 = _mm_mul_ps(lhs, rhs);
	__m128 temp2 = _mm_shuffle_ps(temp1, temp1, _MM_SHUFFLE(0, 0, 3, 2));
	temp2 = _mm_add_ps(temp1, temp2);
	temp1 = _mm_shuffle_ps(temp2, temp2, _MM_SHUFFLE(0, 0, 0, 1));
	temp1 = _mm_add_ps(temp1, temp2);
	return _mm_shuffle_ps(temp1, temp1, _MM_SHUFFLE(0, 0, 0, 0));
}

inline __m128 cross(__m128 lhs, __m128 rhs) {
	__m128 temp1 = _mm_mul_ps(
		_mm_shuffle_ps(lhs, lhs, _MM_SHUFFLE(0, 0, 2, 1)),
		_mm_shuffle_ps(rhs, rhs, _MM_SHUFFLE(0, 1, 0, 2))
	);

	__m128 temp2 = _mm_mul_ps(
		_mm_shuffle_ps(lhs, lhs, _MM_SHUFFLE(0, 1, 0, 2)),
		_mm_shuffle_ps(rhs, rhs, _MM_SHUFFLE(0, 0, 2, 1))
	);

	return _mm_sub_ps(temp1, temp2);
}

namespace Game {

	Vector3 cross(const Vector3& lhs, const Vector3& rhs) {
		return Vector3(
			cross(
				_pack_vector_to_m128(lhs),
				_pack_vector_to_m128(rhs)
			)
		);
	}

	float _packed_dot(__m128 lhs, __m128 rhs) {
		float buffer[4];
		_mm_storeu_ps(buffer, ::dot(lhs, rhs));
	    return buffer[0];
	}

	Vector4::Vector4(__m128 m) {
		_mm_store_ps(raw, m);
	}

	Vector3::Vector3(__m128 m) {
		float buffer[4];
		_mm_storeu_ps(buffer,m);
		memcpy(raw, buffer, sizeof(float) * 3);
	}

	Vector2::Vector2(__m128 m) {
		float buffer[4];
		_mm_storeu_ps(buffer, m);
		memcpy(raw, buffer, sizeof(float) * 2);
	}
}