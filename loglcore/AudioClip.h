#pragma once
#include <Windows.h>
#include <string>
#include "Vector.h"

struct AudioSourceClip;

class AudioClip {
	friend class AudioClipManager;
	friend class AudioListener;
public:
	AudioClip(const wchar_t* path, const char* name, void* data, size_t dataSize);
	void Play(bool loop);
	void Play(bool loop, Game::Vector3 dis);
protected:
	std::wstring path;
	std::string name;
	bool valid;
	void* ss;
};