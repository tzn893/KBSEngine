#include "Matrix.h"
#include "MathFunctions.h"

struct _PackedMat4x4 {
	__m128 m[4];

};

inline _PackedMat4x4 _load_mat(const Game::Mat4x4& mat) {
	_PackedMat4x4 target;
	target.m[0] = _mm_load_ps(mat.a[0]);
	target.m[1] = _mm_load_ps(mat.a[1]);
	target.m[2] = _mm_load_ps(mat.a[2]);
	target.m[3] = _mm_load_ps(mat.a[3]);

	return target;
}

inline _PackedMat4x4 _load_mat(const Game::Mat3x3& mat) {
	_PackedMat4x4 target;
	float buffer0[] = {0,0,0,0};
	float buffer1[] = {0,0,0,1};
	memcpy(buffer0,mat.a[0],sizeof(float) * 3);
	target.m[0] = _mm_loadu_ps(buffer0);
	memcpy(buffer0, mat.a[1], sizeof(float) * 3);
	target.m[1] = _mm_loadu_ps(buffer0);
	memcpy(buffer0, mat.a[2], sizeof(float) * 3);
	target.m[2] = _mm_loadu_ps(buffer0);

	target.m[3] = _mm_loadu_ps(buffer1);
	return target;
}

inline Game::Mat4x4 _store_mat4(_PackedMat4x4 mat) {
	Game::Mat4x4 target;
	_mm_storeu_ps(target.a[0], mat.m[0]);
	_mm_storeu_ps(target.a[1], mat.m[1]);
	_mm_storeu_ps(target.a[2], mat.m[2]);
	_mm_storeu_ps(target.a[3], mat.m[3]);

	return target;
}

inline Game::Mat3x3 _store_mat3(_PackedMat4x4 mat) {
	Game::Mat3x3 target;
	float buffer[4];
	_mm_storeu_ps(buffer,mat.m[0]);
	memcpy(target.a[0], buffer, sizeof(float) * 3);
	_mm_storeu_ps(buffer, mat.m[1]);
	memcpy(target.a[1], buffer, sizeof(float) * 3);
	_mm_storeu_ps(buffer, mat.m[2]);
	memcpy(target.a[2], buffer, sizeof(float) * 3);

	return target;
}

inline _PackedMat4x4 _TransposeMat4x4(_PackedMat4x4& mat) {
	__m128 temp0 = _mm_shuffle_ps(mat.m[0], mat.m[1], _MM_SHUFFLE(1, 0, 1, 0));
	__m128 temp1 = _mm_shuffle_ps(mat.m[2], mat.m[3], _MM_SHUFFLE(1, 0, 1, 0));
	__m128 temp2 = _mm_shuffle_ps(mat.m[0], mat.m[1], _MM_SHUFFLE(3, 2, 3, 2));
	__m128 temp3 = _mm_shuffle_ps(mat.m[2], mat.m[3], _MM_SHUFFLE(3, 2, 3, 2));

	mat.m[0] = _mm_shuffle_ps(temp0, temp1, _MM_SHUFFLE(2, 0, 2, 0));
	mat.m[1] = _mm_shuffle_ps(temp0, temp1, _MM_SHUFFLE(3, 1, 3, 1));
	mat.m[2] = _mm_shuffle_ps(temp2, temp3, _MM_SHUFFLE(2, 0, 2, 0));
	mat.m[3] = _mm_shuffle_ps(temp2, temp3, _MM_SHUFFLE(3, 1, 3, 1));

	return mat;
}

__m128 _DeterminateMat4x4(_PackedMat4x4& mat) {

	//Inorder to indicate how the thing works let's assume:
	//Dxy as the algebraic cofactor of the xth and yth colum of the matrix
	//which means that Dxy's value is the determinate of 
	// | a2x a2y |
	// | a3x a3y |
	//Meanwhile,the Tx stands for the algebraic cofactor where the elements on xth colmn of the
	//matrix is not included
	//For exmaple the T1 stands for the determinate of 
	// | a10 a12 a13 |
	// | a20 a22 a23 |
	// | a30 a32 a33 |


	//First calculate the 2x2 algebraic cofactor of the 4x4 matrix
	//we should pack the result of all the 2x2 algebraic cofactor in this format:
	// vector name : cofactor20 , cofactor21 ,cofactor22
	// x dimension :    D23			 D13		 D12
	// y dimension :    D23			 D03		 D02
	// z dimension :    D13			 D03		 D01
	// w dimension :    D12			 D02		 D01
	//in that format we make it easier to perform the next step

	__m128 temp0 = _mm_shuffle_ps(mat.m[2], mat.m[2], _MM_SHUFFLE(1, 1, 2, 2));
	__m128 temp1 = _mm_shuffle_ps(mat.m[3], mat.m[3], _MM_SHUFFLE(2, 3, 3, 3));

	__m128 temp2 = _mm_shuffle_ps(mat.m[2], mat.m[2], _MM_SHUFFLE(0, 0, 0, 1));
	__m128 temp3 = _mm_shuffle_ps(mat.m[3], mat.m[3], _MM_SHUFFLE(2, 3, 3, 3));

	__m128 temp4 = _mm_shuffle_ps(mat.m[2], mat.m[2], _MM_SHUFFLE(0, 0, 0, 1));
	__m128 temp5 = _mm_shuffle_ps(mat.m[3], mat.m[3], _MM_SHUFFLE(1, 1, 2, 2));

	__m128 cofactor20 = _mm_mul_ps(temp0, temp1);
	__m128 cofactor21 = _mm_mul_ps(temp2, temp3);
	__m128 cofactor22 = _mm_mul_ps(temp4, temp5);


	temp0 = _mm_shuffle_ps(mat.m[2], mat.m[2], _MM_SHUFFLE(2, 3, 3, 3));
	temp1 = _mm_shuffle_ps(mat.m[3], mat.m[3], _MM_SHUFFLE(1, 1, 2, 2));

	temp2 = _mm_shuffle_ps(mat.m[2], mat.m[2], _MM_SHUFFLE(2, 3, 3, 3));
	temp3 = _mm_shuffle_ps(mat.m[3], mat.m[3], _MM_SHUFFLE(0, 0, 0, 1));

	temp4 = _mm_shuffle_ps(mat.m[2], mat.m[2], _MM_SHUFFLE(1, 1, 2, 2));
	temp5 = _mm_shuffle_ps(mat.m[3], mat.m[3], _MM_SHUFFLE(0, 0, 0, 1));

	temp0 = _mm_mul_ps(temp0, temp1);
	cofactor20 = _mm_sub_ps(cofactor20, temp0);
	temp2 = _mm_mul_ps(temp2, temp3);
	cofactor21 = _mm_sub_ps(cofactor21, temp2);
	temp4 = _mm_mul_ps(temp4, temp5);
	cofactor22 = _mm_sub_ps(cofactor22, temp4);

	//calculate the cofactor3x3 of the matrix
	//Then we can pack them in __m128 as
	// x dimension : T0 = a1D23 - a2D13 + a3D12
	// y dimension : T1 = a0D23 - a2D03 + a3D02
	// z dimension : T2 = a0D13 - a1D03 + a3D01
	// w dimension : T3 = a0D12 - a1D02 + a2D01
	temp0 = _mm_shuffle_ps(mat.m[1], mat.m[1], _MM_SHUFFLE(0, 0, 0, 1));
	temp1 = _mm_shuffle_ps(mat.m[1], mat.m[1], _MM_SHUFFLE(1, 1, 2, 2));
	temp2 = _mm_shuffle_ps(mat.m[1], mat.m[1], _MM_SHUFFLE(2, 3, 3, 3));

	temp0 = _mm_mul_ps(temp0,cofactor20);
	temp1 = _mm_mul_ps(temp1,cofactor21);
	temp2 = _mm_mul_ps(temp2,cofactor22);

	temp0 = _mm_sub_ps(temp0,temp1);

	__m128 cofactor3 = _mm_add_ps(temp0,temp2);

	//multiply cofactor3x3 with the first line of the matrix
	temp0 = _mm_mul_ps(cofactor3,mat.m[0]);
	temp1 = _mm_shuffle_ps(temp0, temp0, _MM_SHUFFLE(3, 2, 3, 2));
	temp0 = _mm_add_ps(temp0, temp1);
	temp1 = _mm_shuffle_ps(temp0,temp0,_MM_SHUFFLE(0,0,0,1));
	temp0 = _mm_sub_ps(temp0, temp1);
	
	return _mm_shuffle_ps(temp0,temp0,_MM_SHUFFLE(0,0,0,0));
}

_PackedMat4x4 _InverseMat4x4(_PackedMat4x4& mat, __m128* det = nullptr) {
	__m128 deter = det ? *det : _DeterminateMat4x4(mat);

	mat = _TransposeMat4x4(mat);

	__m128 temp00, temp01;
	__m128 temp10, temp11;
	__m128 temp20, temp21;
	__m128 temp30, temp31;

	__m128 D0, D1, D2;

	//pack vector d0,d1,d2 in format
	//      D0	M2032	M2033	M2132	M2133
	//		D1	M0012	M0013	M0112	M0113
	//		D2	M2031	M2233	M0011	M0213

	temp00 = _mm_shuffle_ps(mat.m[2], mat.m[2], _MM_SHUFFLE(1, 1, 0, 0));
	temp01 = _mm_shuffle_ps(mat.m[3], mat.m[3], _MM_SHUFFLE(3, 2, 3, 2));
	temp10 = _mm_shuffle_ps(mat.m[0], mat.m[0], _MM_SHUFFLE(1, 1, 0, 0));
	temp11 = _mm_shuffle_ps(mat.m[1], mat.m[1], _MM_SHUFFLE(3, 2, 3, 2));
	temp20 = _mm_shuffle_ps(mat.m[2], mat.m[0], _MM_SHUFFLE(2, 0, 2, 0));
	temp21 = _mm_shuffle_ps(mat.m[3], mat.m[1], _MM_SHUFFLE(3, 1, 3, 1));

	D0 = _mm_mul_ps(temp00, temp01);
	D1 = _mm_mul_ps(temp10, temp11);
	D2 = _mm_mul_ps(temp20, temp21);

	temp00 = _mm_shuffle_ps(mat.m[2], mat.m[2], _MM_SHUFFLE(3, 2, 3, 2));
	temp01 = _mm_shuffle_ps(mat.m[3], mat.m[3], _MM_SHUFFLE(1, 1, 0, 0));
	temp10 = _mm_shuffle_ps(mat.m[0], mat.m[0], _MM_SHUFFLE(3, 2, 3, 2));
	temp11 = _mm_shuffle_ps(mat.m[1], mat.m[1], _MM_SHUFFLE(1, 1, 0, 0));
	temp20 = _mm_shuffle_ps(mat.m[2], mat.m[0], _MM_SHUFFLE(3, 1, 3, 1));
	temp21 = _mm_shuffle_ps(mat.m[3], mat.m[1], _MM_SHUFFLE(2, 0, 2, 0));

	temp00 = _mm_mul_ps(temp00, temp01);
	temp10 = _mm_mul_ps(temp10, temp11);
	temp20 = _mm_mul_ps(temp20, temp21);

	D0 = _mm_sub_ps(D0, temp00);
	D1 = _mm_sub_ps(D1, temp10);
	D2 = _mm_sub_ps(D2, temp20);


	/*
		| +00 -01 +02 -03 |
		| -10 +11 -12 +13 |
		| +20 -21 +22 -23 |
		| -30 +31 -32 +33 |


		caculate 3x3 determinates in every row

	*/

	temp31 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 1, 3, 1));

	temp00 = _mm_shuffle_ps(mat.m[1], mat.m[1], _MM_SHUFFLE(1, 0, 2, 1));
	temp01 = _mm_shuffle_ps(temp31, D0, _MM_SHUFFLE(0, 3, 0, 2));
	temp10 = _mm_shuffle_ps(mat.m[0], mat.m[0], _MM_SHUFFLE(0, 1, 0, 2));
	temp11 = _mm_shuffle_ps(temp31, D0, _MM_SHUFFLE(2, 1, 2, 1));

	temp31 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 3, 3, 1));

	temp20 = _mm_shuffle_ps(mat.m[3], mat.m[3], _MM_SHUFFLE(1, 0, 2, 1));
	temp21 = _mm_shuffle_ps(temp31, D1, _MM_SHUFFLE(0, 3, 0, 2));
	temp30 = _mm_shuffle_ps(mat.m[2], mat.m[2], _MM_SHUFFLE(0, 1, 0, 2));
	temp31 = _mm_shuffle_ps(temp31, D1, _MM_SHUFFLE(2, 1, 2, 1));

	__m128 row0 = _mm_mul_ps(temp00, temp01);
	__m128 row1 = _mm_mul_ps(temp10, temp11);
	__m128 row2 = _mm_mul_ps(temp20, temp21);
	__m128 row3 = _mm_mul_ps(temp30, temp31);

	temp31 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(0, 0, 1, 0));

	temp00 = _mm_shuffle_ps(mat.m[1], mat.m[1], _MM_SHUFFLE(2, 1, 3, 2));
	temp01 = _mm_shuffle_ps(D0, temp31, _MM_SHUFFLE(2, 1, 0, 3));
	temp10 = _mm_shuffle_ps(mat.m[0], mat.m[0], _MM_SHUFFLE(1, 3, 2, 3));
	temp11 = _mm_shuffle_ps(D0, temp31, _MM_SHUFFLE(0, 2, 1, 2));

	temp31 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(2, 2, 1, 0));

	temp20 = _mm_shuffle_ps(mat.m[3], mat.m[3], _MM_SHUFFLE(2, 1, 3, 2));
	temp21 = _mm_shuffle_ps(D1, temp31, _MM_SHUFFLE(2, 1, 0, 3));
	temp30 = _mm_shuffle_ps(mat.m[2], mat.m[2], _MM_SHUFFLE(1, 3, 2, 3));
	temp31 = _mm_shuffle_ps(D1, temp31, _MM_SHUFFLE(0, 2, 1, 2));

	temp00 = _mm_mul_ps(temp00, temp01);
	temp10 = _mm_mul_ps(temp10, temp11);
	temp20 = _mm_mul_ps(temp20, temp21);
	temp30 = _mm_mul_ps(temp30, temp31);

	row0 = _mm_sub_ps(row0, temp00);
	row1 = _mm_sub_ps(row1, temp10);
	row2 = _mm_sub_ps(row2, temp20);
	row3 = _mm_sub_ps(row3, temp30);

	temp01 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 0, 2, 2));
	temp00 = _mm_shuffle_ps(mat.m[1], mat.m[1], _MM_SHUFFLE(0, 3, 0, 3));
	temp01 = _mm_shuffle_ps(temp01, temp01, _MM_SHUFFLE(0, 2, 3, 0));

	temp11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 0, 3, 0));
	temp10 = _mm_shuffle_ps(mat.m[0], mat.m[0], _MM_SHUFFLE(2, 0, 3, 1));
	temp11 = _mm_shuffle_ps(temp11, temp11, _MM_SHUFFLE(2, 1, 0, 3));

	temp21 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 2, 2, 2));
	temp20 = _mm_shuffle_ps(mat.m[3], mat.m[3], _MM_SHUFFLE(0, 3, 0, 3));
	temp21 = _mm_shuffle_ps(temp21, temp21, _MM_SHUFFLE(0, 2, 3, 0));

	temp31 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 2, 3, 0));
	temp30 = _mm_shuffle_ps(mat.m[2], mat.m[2], _MM_SHUFFLE(2, 0, 3, 1));
	temp31 = _mm_shuffle_ps(temp31, temp31, _MM_SHUFFLE(2, 1, 0, 3));

	temp00 = _mm_mul_ps(temp00, temp01);
	temp10 = _mm_mul_ps(temp10, temp11);
	temp20 = _mm_mul_ps(temp20, temp21);
	temp30 = _mm_mul_ps(temp30, temp31);


	//In some position we need to substract in some position we need to add
	//So we need to caculate all the sub and add result and shuffle them
	temp01 = _mm_sub_ps(row0, temp00);
	temp00 = _mm_add_ps(row0, temp00);
	temp11 = _mm_add_ps(row1, temp10);
	temp10 = _mm_sub_ps(row1, temp10);
	temp21 = _mm_sub_ps(row2, temp20);
	temp20 = _mm_add_ps(row2, temp20);
	temp31 = _mm_add_ps(row3, temp30);
	temp30 = _mm_sub_ps(row3, temp30);

	temp00 = _mm_shuffle_ps(temp00, temp01, _MM_SHUFFLE(3, 1, 2, 0));
	row0 = _mm_shuffle_ps(temp00, temp00, _MM_SHUFFLE(3, 1, 2, 0));

	temp10 = _mm_shuffle_ps(temp10, temp11, _MM_SHUFFLE(3, 1, 2, 0));
	row1 = _mm_shuffle_ps(temp10, temp10, _MM_SHUFFLE(3, 1, 2, 0));

	temp20 = _mm_shuffle_ps(temp20, temp21, _MM_SHUFFLE(3, 1, 2, 0));
	row2 = _mm_shuffle_ps(temp20, temp20, _MM_SHUFFLE(3, 1, 2, 0));

	temp30 = _mm_shuffle_ps(temp30, temp31, _MM_SHUFFLE(3, 1, 2, 0));
	row3 = _mm_shuffle_ps(temp30, temp30, _MM_SHUFFLE(3, 1, 2, 0));

	_PackedMat4x4 result;

	__m128 one = _mm_set_ps(1, 1, 1, 1);
	__m128 invDet = _mm_div_ps(one, deter);

	result.m[0] = _mm_mul_ps(row0, invDet);
	result.m[1] = _mm_mul_ps(row1, invDet);
	result.m[2] = _mm_mul_ps(row2, invDet);
	result.m[3] = _mm_mul_ps(row3, invDet);

	return result;
}



_PackedMat4x4 _MulMat4x4(_PackedMat4x4& lhs, _PackedMat4x4& rhs) {
	//for function multiply we seperate out four dimensions of the 
	//one row in 4 vectors,multiply them individually with the same
	//row of the other matrix and add the results up.Then we get one
	//row of the result.
#define _GET_MAT_DIMENSION_(mat,row,dim) _mm_shuffle_ps(mat.m[row],mat.m[row],_MM_SHUFFLE(dim,dim,dim,dim))

	_PackedMat4x4 result;

	__m128 temp0, temp1, temp2, temp3;

#define _MUL_ROW_(row) temp0 = _mm_shuffle_ps(lhs.m[row],lhs.m[row],_MM_SHUFFLE(0,0,0,0));\
	temp0 = _mm_mul_ps(temp0,rhs.m[0]);\
	temp1 = _mm_shuffle_ps(lhs.m[row],lhs.m[row],_MM_SHUFFLE(1,1,1,1));\
	temp1 = _mm_mul_ps(temp1,rhs.m[1]);\
	temp2 = _mm_shuffle_ps(lhs.m[row],lhs.m[row],_MM_SHUFFLE(2,2,2,2));\
	temp2 = _mm_mul_ps(temp2,rhs.m[2]);\
	temp3 = _mm_shuffle_ps(lhs.m[row],lhs.m[row],_MM_SHUFFLE(3,3,3,3));\
	temp3 = _mm_mul_ps(temp3,rhs.m[3]);\
	\
	temp0 = _mm_add_ps(temp0,temp1);\
	temp0 = _mm_add_ps(temp0,temp2);\
	result.m[row] = _mm_add_ps(temp0,temp3);

	//_MUL_ROW_(0);
	temp0 = _mm_shuffle_ps(lhs.m[0], lhs.m[0], _MM_SHUFFLE(0, 0, 0, 0));
	temp0 = _mm_mul_ps(temp0, rhs.m[0]);
	temp1 = _mm_shuffle_ps(lhs.m[0], lhs.m[0], _MM_SHUFFLE(1, 1, 1, 1));
	temp1 = _mm_mul_ps(temp1, rhs.m[1]);
	temp2 = _mm_shuffle_ps(lhs.m[0], lhs.m[0], _MM_SHUFFLE(2, 2, 2, 2));
	temp2 = _mm_mul_ps(temp2, rhs.m[2]);
	temp3 = _mm_shuffle_ps(lhs.m[0], lhs.m[0], _MM_SHUFFLE(3, 3, 3, 3));
	temp3 = _mm_mul_ps(temp3, rhs.m[3]);
	temp0 = _mm_add_ps(temp0, temp1);
	temp0 = _mm_add_ps(temp0, temp2);
	result.m[0] = _mm_add_ps(temp0, temp3);
	_MUL_ROW_(1);
	_MUL_ROW_(2);
	_MUL_ROW_(3);

	return result;
}

__m128 _Multi4x4(_PackedMat4x4& mat,__m128 vec) {
	__m128 temp0, temp1;

#define SUM_VEC(v)\
	temp0 = _mm_shuffle_ps(v,v,_MM_SHUFFLE(1,0,1,0));\
	temp1 = _mm_shuffle_ps(v,v,_MM_SHUFFLE(3,2,3,2));\
	temp0 = _mm_add_ps(temp0,temp1);\
	v = _mm_add_ps(_mm_shuffle_ps(temp0,temp0,_MM_SHUFFLE(0,0,0,0)),_mm_shuffle_ps(temp0,temp0,_MM_SHUFFLE(1,1,1,1)));

	__m128 v0 = _mm_mul_ps(mat.m[0],vec);
	SUM_VEC(v0);
	__m128 v1 = _mm_mul_ps(mat.m[1],vec);
	SUM_VEC(v1);
	__m128 v2 = _mm_mul_ps(mat.m[2],vec);
	SUM_VEC(v2);
	__m128 v3 = _mm_mul_ps(mat.m[3],vec);
	SUM_VEC(v3);

	v0 = _mm_shuffle_ps(v0, v1, _MM_SHUFFLE(0, 0, 0, 0));
	v0 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(2, 0, 2, 0));
	v2 = _mm_shuffle_ps(v2, v3, _MM_SHUFFLE(0, 0, 0, 0));
	v2 = _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(2, 0, 2, 0));

	return _mm_shuffle_ps(v0, v2, _MM_SHUFFLE(3, 2, 1, 0));
}

namespace Game {
	Mat2x2 Mat2x2::T() const {
		return Mat2x2(a[0][0],a[0][1],a[1][0],a[1][1]);
	}

	Mat4x4 Mat4x4::T() const {
		_PackedMat4x4 mat = _load_mat(*this);

		mat = _TransposeMat4x4(mat);

		return _store_mat4(mat);
	}

	Mat3x3 Mat3x3::T() const {
		_PackedMat4x4 mat = _load_mat(*this);

		mat = _TransposeMat4x4(mat);

		return _store_mat3(mat);
	}

	float Mat2x2::det() const {
		return a[0][0] * a[1][1] - a[1][0] * a[0][1];
	}

	float Mat3x3::det() const {
		_PackedMat4x4 mat = _load_mat(*this);

		float buffer[4];
		_mm_storeu_ps(buffer, _DeterminateMat4x4(mat));

		return buffer[0];
	}

	float Mat4x4::det() const {
		_PackedMat4x4 mat = _load_mat(*this);

		float buffer[4];
		_mm_storeu_ps(buffer, _DeterminateMat4x4(mat));

		return buffer[0];
	}

	Mat4x4 Mat4x4::R() const {
		_PackedMat4x4 mat = _load_mat(*this);

		return _store_mat4(_InverseMat4x4(mat));
	}

	Mat3x3 Mat3x3::R() const {
		_PackedMat4x4 mat = _load_mat(*this);

		return _store_mat3(_InverseMat4x4(mat));
	}

	Mat2x2 Mat2x2::R() const {
		float det = this->det();
		return Mat2x2(a[1][1] / det, -a[1][0] / det,
			-a[0][1] / det, a[0][0] / det);
	}


	Vector4 mul(const Mat4x4& mat, Vector4 vec) {
		_PackedMat4x4 _mat = _load_mat(mat);
		__m128 result = _Multi4x4(_mat, _pack_vector_to_m128(vec));

		return Vector4(result);
	}

	Vector3 mul(const Mat3x3& mat, Vector3 vec) {
		_PackedMat4x4 _mat = _load_mat(mat);
		__m128 result = _Multi4x4(_mat, _pack_vector_to_m128(vec));

		return Vector3(result);
	}

	Vector2 mul(const Mat2x2& mat, Vector2 vec) {
		return Vector2(
			vec[0] * mat.a[0][0] + mat.a[0][1] * vec[1],
			vec[0] * mat.a[1][0] + mat.a[1][1] * vec[1]
		);
	}

	Mat4x4 mul(const Mat4x4& lhs,const Mat4x4& rhs) {
		_PackedMat4x4 _lhs = _load_mat(lhs), _rhs = _load_mat(rhs);

		return _store_mat4(_MulMat4x4(_lhs, _rhs));
	}

	Mat3x3 mul(const Mat3x3& lhs,const Mat3x3& rhs) {
		_PackedMat4x4 _lhs = _load_mat(lhs), _rhs = _load_mat(rhs);

		return _store_mat3(_MulMat4x4(_lhs, _rhs));
	}

	Mat2x2 mul(const Mat2x2& lhs,const Mat2x2& rhs) {
		return Mat2x2(
			lhs.a[0][0] * rhs.a[0][0] + lhs.a[0][1] * rhs.a[1][0],
			lhs.a[0][0] * rhs.a[0][1] + lhs.a[0][1] * rhs.a[1][1],
			lhs.a[1][0] * rhs.a[0][0] + lhs.a[1][1] * rhs.a[1][0],
			lhs.a[1][0] * rhs.a[0][1] + lhs.a[1][1] * rhs.a[1][1]
		);
	}

	Mat4x4 MatrixLookAt(Vector3 pos, Vector3 target, Vector3 up) {
		Vector3 x = normalize(target - pos);
		Vector3 y = normalize(cross(x, up));
		Vector3 z = cross(y, x);

		float buffer[16] = {0};
		memcpy(buffer, x.raw, sizeof(float) * 3);
		memcpy(buffer + 4, y.raw, sizeof(float) * 3);
		memcpy(buffer + 8, z.raw, sizeof(float) * 3);

		buffer[15] = 1.;

		return Mat4x4(buffer);
	}

	Mat4x4 MatrixPosition(Vector3 offset) {
		float buffer[16] = {
			1,0,0,offset[0],
			0,1,0,offset[1],
			0,0,1,offset[2],
			0,0,0,1
		};

		return Mat4x4(buffer);
	}

	Mat4x4 MatrixRotation(Vector3 axis,float angle) {
		axis = normalize(axis);

		float s = sin(angle), c = cos(angle),
			& x = axis[0],& y = axis[1],&z = axis[2],oc = 1 - c;

		float buffer[16] = {
			x*x*oc + c  , x*y*oc + s*z , x*z*oc - y*s , 0 , 
			x*y*oc - s*z, y*y*oc + c   , y*z*oc + x*s , 0 ,
			x*z*oc + s*y, y*z*oc - s*x , z*z*oc + c   , 0 ,
			0           , 0            , 0            , 1 
		};

		return Mat4x4(buffer);
	}

	Mat4x4 MatrixRotateX(float angle) {
		float s = sin(angle), c = cos(angle);

		float buffer[16] = {
			1, 0, 0,0,
			0, c, s,0,
			0,-s, c,0,
			0, 0, 0,1
		};

		return Mat4x4(buffer);
	}

	Mat4x4 MatrixRotateY(float angle) {
		float s = sin(angle), c = cos(angle);

		float buffer[16] = {
			c,0, -s,0,
			0,1,  0,0,
		    s,0,  c,0,
			0,0,  0,1
		};

		return Mat4x4(buffer);
	}

	Mat4x4 MatrixRotateZ(float angle) {
		float s = sin(angle), c = cos(angle);

		float buffer[16] = {
			 c, s,0,0,
		    -s, c,0,0,
			 0, 0,1,0,
			 0, 0,0,1
		};

		return Mat4x4(buffer);
	}


	Mat4x4 MatrixScale(Vector3 scaling) {
		float buffer[16] = {
			scaling[0],0,0,0,
			0,scaling[1],0,0,
			0,0,scaling[2],0,
			0,0,0,1
		};

		return Mat4x4(buffer);
	}

	Mat4x4 MatrixProjection(float aspectRatio, float verticalViewAngle,
		float near, float far) {
		
		float a = tan(verticalViewAngle / 2);

		float nearCov = far / (far - near);

		float buffer[16] = {
			1 / (a * aspectRatio) , 0     , 0       , 0 ,
			0                     , 1 / a , 0       , 0 ,
			0                     ,     0 , nearCov , -nearCov * near ,
			0                     ,     0 , 1       , 0
		};

		return Mat4x4(buffer);
	}

	Mat4x4 MatrixRotation(Vector3 eulerAngle) {
		//rotation order axis x,axis y,axis z
		float cr = cos(eulerAngle.x), sr = sin(eulerAngle.x);
		float cb = cos(eulerAngle.y), sb = sin(eulerAngle.y);
		float ca = cos(eulerAngle.z), sa = sin(eulerAngle.z);

		
		
		float buffer[16] = {
			ca * cb, ca * sb * sr - sa * cr, ca * sb * cr + sa * sr, 0,
			sa * cb, sa * sb * sr + ca * cr, sa * sb * cr - ca * sr, 0,
		   -sb     , cb * sr               , cb * cr               , 0,
		    0      , 0                     , 0                     , 1
		};

		return Mat4x4(buffer);
	}

	void UnpackTransfrom(Mat4x4 trans,Vector3& position,Vector3& rotation,Vector3& scale) {

		float scal_z = sqrt(trans.a[2][0] * trans.a[2][0] + trans.a[2][1] * trans.a[2][1] + trans.a[2][2] * trans.a[2][2]);
		float sb = - trans.a[2][0] / scal_z;
		float cb = sqrt(1 - sb * sb);

		float  cr, sr;
		float ca_scal_x = trans.a[0][0] / cb;
		float sa_scal_y = trans.a[1][0] / cb;
		sr = trans.a[2][1] / (cb * scal_z);
		cr = trans.a[2][2] / (cb * scal_z);

		float ca_scal_y;float sa_scal_x;
		if (abs(cr) > 1e-3) {
			sa_scal_x = (-trans.a[0][1] + ca_scal_x * sb * sr) / cr;
			ca_scal_y = ( trans.a[1][1] - sa_scal_y * sb * sr) / cr;
		}
		else {
			ca_scal_y = (-trans.a[1][2] + sa_scal_y * sb * cr) / sr;
			sa_scal_x = ( trans.a[0][2] - ca_scal_x * sb * cr) / sr;
		}

		float scal_y = sqrt(ca_scal_y * ca_scal_y + sa_scal_y * sa_scal_y);
		float scal_x = sqrt(ca_scal_x * ca_scal_x + sa_scal_x * sa_scal_x);

		float ca = ca_scal_x / scal_x, sa = sa_scal_x / scal_x;

		float a01 = ca_scal_x * sb * sr - sa * cr, a02 = ca_scal_x * sb * cr + sa * sr;
		float a11 = sa_scal_y * sb * sr + ca * cr, a12 = sa_scal_y * sb * cr - ca * sr;

		if (abs(a01 - trans.a[0][1]) > 1e-3 || 
			abs(a11 - trans.a[1][1]) > 1e-3 ||
			abs(a02 - trans.a[0][2]) > 1e-3 ||
			abs(a12 - trans.a[1][2]) > 1e-3) {
			cb = -cb;
			ca = trans.a[0][0] / (cb * scal_x);
			sa = trans.a[1][0] / (cb * scal_y);
			cr = trans.a[2][1] / (cb * scal_z);
			cr = trans.a[2][2] / (cb * scal_z);
		}

		rotation = Vector3(get_angle(sr, cr), get_angle(sb, cb), get_angle(sa, ca));
		position = Vector3(trans.a[0][3], trans.a[1][3], trans.a[2][3]);
		scale = Vector3(scal_x, scal_y, scal_z);
	}


	Mat4x4 PackTransfrom(Vector3 position,Vector3 rotation,Vector3 scaling) {
		return mul(MatrixPosition(position),mul(MatrixRotation(rotation),MatrixScale(scaling)));
	}

	Mat3x3::Mat3x3(const Mat4x4& mat) {
		memcpy(a, mat.a, sizeof(float) * 3);
		memcpy(a + 3, mat.a + 4, sizeof(float) * 3);
		memcpy(a + 6, mat.a + 8, sizeof(float) * 3);
	}
}