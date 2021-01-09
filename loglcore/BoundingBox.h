#pragma once
#include "Vector.h"

struct BoxAABB {
	BoxAABB(Game::Vector3 v0, Game::Vector3 v1) {
		vmax = Game::Vector3(fmax(v0[0], v1[0]), fmax(v0[1], v1[1]), fmax(v0[2], v1[2]));
		vmin = Game::Vector3(fmin(v0[0], v1[0]), fmin(v0[1], v1[1]), fmin(v0[2], v1[2]));
	}
	BoxAABB(const BoxAABB& box) {
		vmax = box.vmax; vmin = box.vmin;
	}
	BoxAABB() {
		vmax = Game::Vector3(), vmin = Game::Vector3();
	}
	BoxAABB Merge(BoxAABB& other) {
		Game::Vector3 nvmax = Game::Vector3(fmax(vmax[0], other.vmax[0]), fmax(vmax[1], other.vmax[1]), fmax(vmax[2], other.vmax[2]));
		Game::Vector3 nvmin = Game::Vector3(fmin(vmin[0], other.vmin[0]), fmin(vmin[1], other.vmin[1]), fmin(vmin[2], other.vmin[2]));
		return BoxAABB(nvmax, nvmin);
	}
	BoxAABB Merge(Game::Vector3 v) {
		Game::Vector3 nvmax = Game::Vector3(fmax(vmax[0], v[0]), fmax(vmax[1], v[1]), fmax(vmax[2], v[2]));
		Game::Vector3 nvmin = Game::Vector3(fmin(vmin[0], v[0]), fmin(vmin[1], v[1]), fmin(vmin[2], v[2]));
		return BoxAABB(nvmax, nvmin);
	}

	Game::Vector3 vmax, vmin;
};
