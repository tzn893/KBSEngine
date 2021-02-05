#include "SampleSquence.h"

float Halton(size_t p,size_t i) {
	float l = 0., f = 1.;
	while (i > 0) {
		f /= (float)p;
		l += f * (float)(i % p);
		i = i / p;
	}
	return l;
}

void SampleSquence::InitializeHalton_2_3(Game::Vector2* halton, size_t len) {
	for (size_t i = 0; i != len;i++) {
		halton[i] = Game::Vector2(Halton(2, i), Halton(3, i));
	}
}