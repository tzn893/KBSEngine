#pragma once
#include "Vector.h"

struct Complex{
public:
	Complex(float x = 0.,float y = 0.) {
		complex.x = x;
		complex.y = y;
	}
	Complex(Game::Vector2 vec) {
		complex = vec;
	}
	Complex(const Complex& com) {
		complex = com.complex;
	}

	inline float Re() const { return complex.x; }
	inline float Im() const { return complex.y; }

	inline float len() const { return Game::length(complex); }
	inline Complex dir() const { return Complex(complex / len()); }

	inline Complex operator=(const Complex& other) { complex = other.complex; return *this; }

	inline Complex operator+(float x) const { return Complex(complex.x + x,complex.y); }
	inline Complex operator+(const Complex& other) const { return Complex(complex + other.complex); }

	inline const Complex& operator+=(float x) { *this = *this + x; return *this; }
	inline const Complex& operator+=(const Complex& other) { *this = *this + other; return *this; }

	inline Complex operator-(float x) const { return Complex(complex.x - x, complex.y); }
	inline Complex operator-(const Complex& other) const { return Complex(complex - other.complex); }

	inline const Complex& operator-=(float x) { *this = *this - x; return *this; }
	inline const Complex& operator-=(const Complex& other) { *this = *this - other; return *this; }
	
	inline Complex operator*(float x) const { return Complex(complex.x * x, complex.y *  x); }
	inline Complex operator*(const Complex& other) const { return Complex(Re() * other.Re() - Im() * other.Im(),
		Re() * other.Im() + Im() * other.Re()); }

	inline const Complex& operator*=(float x) { *this = *this * x; return *this; }
	inline const Complex& operator*=(const Complex& other) { *this = *this * other; return *this; }

	inline Complex operator/(float x) const { return Complex(Game::Vector2(complex.x / x, complex.y / x)); }
	inline Complex operator/(const Complex& other) const { return Complex(Re() * other.Re() + Im() * other.Im(),Im() * other.Re() - Re() * other.Im()) /
		(Game::dot(other.complex,other.complex)); }

	inline const Complex& operator/=(float x) { *this = *this / x; return *this; }
	inline const Complex& operator/=(const Complex& other) { *this = *this / other; return *this; }

	static inline Complex exp(const Complex& complex) { return Complex(cosf(complex.Im()), sinf(complex.Im())) * expf(complex.Re()); }
	static inline Complex exp(float re, float im) { return Complex(cosf(im), sinf(im)) * expf(re); }

	inline Complex conj() { return Complex(complex.x, -complex.y); }

private:
	Game::Vector2 complex;
};