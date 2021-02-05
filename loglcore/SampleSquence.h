#pragma once
#include "Vector.h"

class SampleSquence {
public:
	SampleSquence() {
		InitializeHalton_2_3(mHalton_2_3x8, _countof(mHalton_2_3x8));
	}

	Game::Vector2 Halton_2_3x8(size_t index) {
		size_t i = index % _countof(mHalton_2_3x8);
		return mHalton_2_3x8[i];
	}
private:
	Game::Vector2 mHalton_2_3x8[8];

	void InitializeHalton_2_3(Game::Vector2* halton,size_t len);
};

inline SampleSquence gSampleSquence;