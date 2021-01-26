#include "AudioClip.h"
#include "irrKCommon.hpp"

AudioClip::AudioClip(const wchar_t* path, const char* name, void* data,size_t dataSize):
	name(name),path(path){
	ISoundEngine* SE = GetSoundEngine();
	ss = SE->addSoundSourceFromMemory(data, dataSize,name);
	if (ss == nullptr) {
		valid = false;
	}
	else {
		valid = true;
	}
}

void AudioClip::Play(bool loop) {
	ISoundEngine* SE = GetSoundEngine();
	SE->play2D(reinterpret_cast<ISoundSource*>(ss), loop);
}

void AudioClip::Play(bool loop,Game::Vector3 dis) {
	ISoundEngine* SE = GetSoundEngine();
	SE->play3D(reinterpret_cast<ISoundSource*>(ss), 
		vec3df(dis.x, dis.y, dis.z), loop);
}