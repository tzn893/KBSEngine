#pragma once

#include <xmmintrin.h>
#include <string.h>
#include "Vector.h"

namespace Game {
	struct Mat4x4;
	struct Mat3x3;

	struct Mat2x2 {
		union {
			float a[2][2];
		};

		Mat2x2() { memset(a, 0, sizeof(float) * 4); }
		Mat2x2(const float* a) { memcpy(this->a, a, sizeof(float) * 4); }
		Mat2x2(__m128 m) { _mm_storeu_ps((float*)a,m); }
		Mat2x2(float a00, float a01,float a10, float a11){
			a[0][0] = a00, a[0][1] = a01, a[1][0] = a10, a[1][1] = a11;
		}

		//Inverse and determinate cost a lot,using these two
		//operation too ofthen is not recomended
		float det() const;
		//Inverse and determinate cost a lot,using these two
		//operation too ofthen is not recomended
		Mat2x2 R() const;
		Mat2x2 T() const;
		static Mat2x2 I() { return Mat2x2(1, 0, 0, 1); }
	};

	struct Mat3x3 {
		float a[3][3];

		Mat3x3() { memset(a, 0, sizeof(float) * 9); }
		Mat3x3(const float* f) { memcpy(a, f, sizeof(float) * 9); }
		Mat3x3(float a00,float a01,float a02,
			   float a10,float a11,float a12,
			   float a20,float a21,float a22) {
		
			a[0][0] = a00, a[0][1] = a01, a[0][2] = a02,
			a[1][0] = a10, a[1][1] = a11, a[1][2] = a12,
			a[2][0] = a20, a[2][1] = a21, a[2][2] = a22;
		}
		//Inverse and determinate cost a lot,using these two
		//operation too ofthen is not recomended
		float det() const;

		Mat3x3(const Mat4x4& mat);

		//Inverse and determinate cost a lot,using these two
		//operation too ofthen is not recomended
		Mat3x3 R() const;
		Mat3x3 T() const;

		static Mat3x3 I() {
			static float buffer[] = {
				1,0,0,
				0,1,0,
				0,0,1
			};

			return Mat3x3(buffer);
		}
	};

	struct Mat4x4 {
		float a[4][4];

		Mat4x4() { memset(a, 0, sizeof(float) * 16); }
		Mat4x4(const float* f) {
			/*init(f[0] ,f[1] ,f[2] ,f[3] ,
				 f[4] ,f[5] ,f[6] ,f[7] ,
				 f[8] ,f[9] ,f[10],f[11],
				 f[12],f[13],f[14],f[15]);*/
			memcpy(a,f,sizeof(float) * 16);
		}
		Mat4x4(float a00,float a01,float a02,float a03,
			float a10, float a11, float a12, float a13,
			float a20, float a21, float a22, float a23,
			float a30, float a31, float a32, float a33) {

			init(a00,a01,a02,a03,
				 a10,a11,a12,a13,
				 a20,a21,a22,a23,
				 a30,a31,a32,a33);
		}

		void init(float a00, float a01, float a02, float a03,
			float a10, float a11, float a12, float a13,
			float a20, float a21, float a22, float a23,
			float a30, float a31, float a32, float a33) {

			    a[0][0] = a00, a[0][1] = a01, a[0][2] = a02, a[0][3] = a03,
				a[1][0] = a10, a[1][1] = a11, a[1][2] = a12, a[1][3] = a13,
				a[2][0] = a20, a[2][1] = a21, a[2][2] = a22, a[2][3] = a23,
				a[3][0] = a30, a[3][1] = a31, a[3][2] = a32, a[3][3] = a33;
		}


		//Inverse and determinate cost a lot,using these two
		//operation too ofthen is not recomended
		float det() const;

		//Inverse and determinate cost a lot,using these two
		//operation too ofthen is not recomended
		Mat4x4 R() const;
		Mat4x4 T() const;

		static Mat4x4 I() {
			static float buffer[] = {
				1,0,0,0,
				0,1,0,0,
				0,0,1,0,
				0,0,0,1
			};
			return Mat4x4(buffer);
		}
	};

	Mat4x4 MatrixLookAt(Vector3 pos,Vector3 target,Vector3 up);
	Mat4x4 MatrixProjection(float aspectRatio,float verticalViewAngle,
		float near =  0.1f,float far = 1000.f);

	Mat4x4 MatrixPosition(Vector3 Offset);
	Mat4x4 MatrixRotation(Vector3 axis,float angle);
	Mat4x4 MatrixRotation(Vector3 eulerAngle);

	void UnpackTransfrom(Mat4x4 trans,Vector3& position,Vector3& rotation,Vector3& scaling);
	Mat4x4 PackTransfrom(Vector3 position,Vector3 rotation,Vector3 scaling);

	Mat4x4 MatrixRotateX(float angle);
	Mat4x4 MatrixRotateY(float angle);
	Mat4x4 MatrixRotateZ(float angle);

	Mat4x4 MatrixScale(Vector3 Scaling);

	Mat4x4 mul(const Mat4x4& lhs,const Mat4x4& rhs);
	Mat3x3 mul(const Mat3x3& lhs,const Mat3x3& rhs);
	Mat2x2 mul(const Mat2x2& lhs,const Mat2x2& rhs);

	Vector4 mul(const Mat4x4& lhs,Vector4 vec);
	Vector3 mul(const Mat3x3& lhs,Vector3 rhs);
	Vector2 mul(const Mat2x2& lhs,Vector2 rhs);
}