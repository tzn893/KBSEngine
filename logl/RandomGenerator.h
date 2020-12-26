#pragma once

#include <stdint.h>
#include <functional>
#include <time.h>
#include "../loglcore/Math.h"

template<size_t Dim>
struct RandomValue {
	using type = std::vector<float>;
};

template<>
struct RandomValue<1> {
	using type = float;
};

template<>
struct RandomValue<2> {
	using type = Game::Vector2;
};

template<>
struct RandomValue<3> {
	using type = Game::Vector3;
};

template<>
struct RandomValue<4> {
	using type = Game::Vector4;
};

template<size_t Dim>
class RandomValueDistribution {
public:
	virtual typename RandomValue<Dim>::type Transform(
		typename RandomValue<Dim>::type value) const = 0;
};


template<size_t Dim>
class UniformDistribution :public RandomValueDistribution<Dim> {
public:
	virtual typename RandomValue<Dim>::type Transform(
			typename RandomValue<Dim>::type value) const override {
		return (value  - .5) * range  + center;
	}

	UniformDistribution(typename RandomValue<Dim>::type lower, typename RandomValue<Dim>::type upper):
		center((upper + lower) * .5),range((upper - lower)){}
private:
	typename RandomValue<Dim>::type center, range;
};



class NormalDistribution : public RandomValueDistribution<2> {
public:
	virtual Game::Vector2 Transform(Game::Vector2 value) const override {
		value.x = clamp(1. - 1e-6, 1e-6, value.x);
		float R = sqrtf(-logf(value.x) * 2.),
			T = 2 * PI * value.y;
		float x = R * cosf(T) * theta[0] + beta[0],
			y = R * sinf(T)  * theta[1] + beta[1];
		return Game::Vector2(x, y);
	}

	NormalDistribution(float theta = 1, float beta = 0) {
		this->theta[0] = theta, this->theta[1] = theta;
		this->beta[0] = beta, this->beta[1] = beta;
	}

	NormalDistribution(float theta1,float beta1,
		 float theta2,float beta2) {
		theta[0] = theta1, theta[1] = theta2;
		beta[0] = beta1, beta[1] = beta2;
	}

private:
	float theta[2], beta[2];
};

class RandomGenerator {
public:

	RandomGenerator() {
		uint32_t timeTick = time(nullptr) % (rand_cycle / 4);
		nLehmer[0] = timeTick;
		nLehmer[1] = timeTick + rand_cycle / 4;
		nLehmer[2] = timeTick + rand_cycle / 2;
		nLehmer[3] = timeTick + rand_cycle / 4 * 3;
	}

	float Rand(const RandomValueDistribution<1>& dis = UniformDistribution<1>(0, 1)) {
		return dis.Transform(Lehmer32<1>());
	}
	float Rand(uint32_t seed,const RandomValueDistribution<1>& dis = UniformDistribution<1>(0,1)) {
		nLehmer[0] = seed;
		return Rand(dis);
	}
	template<typename T,size_t Dim>
	float Rand(const T& seed,std::function<void(const T&,uint32_t*)> seeder,const RandomValueDistribution<Dim>& dis) {
		seeder(seed, nLehmer);
		return Rand(dis);
	}

	Game::Vector2 Rand2(const RandomValueDistribution<2>& dis = UniformDistribution<2>(Game::Vector2(), Game::Vector2(1., 1.))) {
		return dis.Transform(Lehmer32<2>());
	}

	Game::Vector2 Rand2(uint32_t* seeds,const RandomValueDistribution<2>& dis = 
						UniformDistribution<2>(Game::Vector2(0.,0.),Game::Vector2(1.,1.))) {
		nLehmer[0] = seeds[0], nLehmer[1] = seeds[1];
		return dis.Transform(Lehmer32<2>());
	}
	
private:
	static constexpr uint32_t rand_cycle = 1 << 24;
	uint32_t nLehmer[4];

	template<size_t Dim>
	typename RandomValue<Dim>::type Lehmer32() {
		
		typename RandomValue<Dim>::type rv;

		for (size_t i = 0; i != Dim; i++) {
			nLehmer[i] += 0xe120fc15;
			uint64_t tmp;
			tmp = (uint64_t)nLehmer[i] * 0x4a39b70d;
			uint32_t m1 = (tmp >> 32) ^ tmp;
			tmp = (uint64_t)m1 * 0x12fad5c9;
			uint32_t m2 = (tmp >> 32) ^ tmp;

			rv[i] = static_cast<float>(m2 % rand_cycle) / static_cast<float>(rand_cycle);
		}

		return rv;
	}

	template<>
	typename float Lehmer32<1>() {
		nLehmer[0] += 0xe120fc15;
		uint64_t tmp;
		tmp = (uint64_t)nLehmer[0] * 0x4a39b70d;
		uint32_t m1 = (tmp >> 32) ^ tmp;
		tmp = (uint64_t)m1 * 0x12fad5c9;
		uint32_t m2 = (tmp >> 32) ^ tmp;
		
		return static_cast<float>(m2 % rand_cycle) / static_cast<float>(rand_cycle);
	}
};

inline RandomGenerator gRandomGenerator;