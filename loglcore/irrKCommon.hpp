#pragma once
#include <irrK/irrKlang.h>

#ifndef _IRRK_LINKED
#define _IRRK_LINKED
#pragma comment(lib,"irrKlang.lib")
#endif
using namespace irrklang;

inline ISoundEngine* gSoundEngine = nullptr;

ISoundEngine* GetSoundEngine() {
	if (gSoundEngine == nullptr) {
		gSoundEngine = createIrrKlangDevice();
	}
	return gSoundEngine;
}