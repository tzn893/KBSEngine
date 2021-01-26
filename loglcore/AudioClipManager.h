#pragma once
#include "AudioClip.h"
#include <unordered_map>

class AudioClipManager {
public:
	AudioClip*   LoadAudioClip(const wchar_t* path,const char* name);
	AudioClip*   GetAudioByName(const char* name);

	AudioClip*   GetAudioByPath(const wchar_t* path);
private:
	std::unordered_map<std::wstring, AudioClip*> clip_path_map;
	std::unordered_map<std::string, std::unique_ptr<AudioClip>> clips;
};

inline AudioClipManager gAudioClipManager;