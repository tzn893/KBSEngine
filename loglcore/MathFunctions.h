#pragma once
#include <math.h>

#define PI 3.1415926f

inline float clamp(float upper,float lower,float target) {
	if (target > upper) {
		return upper;
	}
	else if (target < lower ) {
		return lower;
	}
	return target;
}


inline float get_angle(float sinangle, float cosangle, bool radius = true) {

	//avoid the sinangle and cosangle value overflow,because of the accurancy of the floating point system
	//when the sin and cos values are close to 1. or -1.,maybe some overflow will occur(some thing like 1.0000012 etc.)
	//clamp the value of sinangle and cosangle between -1.0~1.0 to avoid this condition
	if (abs(sinangle) < 1e-4) {
		sinangle = 0.;
	}
	if (abs(cosangle) < 1e-4) {
		cosangle = 0.;
	}

	sinangle = clamp(1.,-1.,sinangle);
	cosangle = clamp(1.,-1.,cosangle);
	float angle = asin(sinangle) >= 0. ? acos(cosangle) : 2 * PI - acos(cosangle);
	if (!radius) {
		return angle * 180.f / PI;
	}
	return angle;
}


inline size_t round_up(size_t num,size_t step) {
	
	return (num + step - 1) & ~(step - 1);
}

namespace Game {
	//we can replace this function with better algorithm
	inline float pow(float num,float index) {
		return ::pow(num, index);
	}
	inline float fmax(float num1,float num2) {
		return num1 > num2 ? num1 : num2;
	}
	inline float fmin(float num1,float num2) {
		return num1 < num2 ? num1 : num2;
	}

	inline int imax(int num1,int num2) {
		return num1 > num2 ? num1 : num2;
	}

	inline int imin(int num1,int num2) {
		return num1 < num2 ? num1 : num2;
	}
	inline int forward(int num, int boundary,int step = 1) {
		return (num + step) % boundary;
	}
}