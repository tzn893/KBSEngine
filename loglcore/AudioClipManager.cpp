#include "AudioClipManager.h"
#include <filesystem>

AudioClip* AudioClipManager::GetAudioByName(const char* name) {
	if (auto clip = clips.find(name);clip != clips.end()) {
		return clip->second.get();
	}
	return nullptr;
}

AudioClip* AudioClipManager::GetAudioByPath(const wchar_t* path) {
	if (auto clip = clip_path_map.find(path);clip != clip_path_map.end()) {
		return clip->second;
	}
	return nullptr;
}

bool checkPathVality(const wchar_t* path) {
	std::filesystem::path p(path);
	if (!std::filesystem::exists(p)) {
		return false;
	}
	
	std::wstring pext = p.extension().wstring();
	if (pext == L".wav" || pext == L".ogg" /*|| pext == L".mp3"*/
		|| pext == L".flac" || pext == L".mod" || pext == L".it" ||
		pext == L".s3d" || pext == L".xm") {
		return true;
	}
	return false;
}

AudioClip* AudioClipManager::LoadAudioClip(const wchar_t* path, const char* name) {
	if (auto clip = GetAudioByPath(path);clip != nullptr) {
		return clip;
	}
	if (GetAudioByName(name) != nullptr) {
		return nullptr;
	}
	if (!checkPathVality(path)) {
		return nullptr;
	}
	FILE* fstream;
	_wfopen_s(&fstream, path, L"rb");
	if (!fstream) {
		return false;
	}


	fseek(fstream, 0, SEEK_END);
	size_t filesize = ftell(fstream);
	fseek(fstream, 0, SEEK_SET);

	void* data = malloc(filesize);
	fread(data, filesize, 1, fstream);

	std::unique_ptr<AudioClip> clip = std::make_unique<AudioClip>(path,name,data,filesize);
	free(data);

	if (!clip->valid) {
		clip.release();
		return nullptr;
	}

	AudioClip* clip_ptr = clip.get();
	clips[name] = std::move(clip);
	clip_path_map[path] = clip_ptr;

	return clip_ptr;
}