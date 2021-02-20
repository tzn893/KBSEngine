#pragma once
#include <string>
#include <vector>
#include "Math.h"
#include "ConstantBuffer.h"
#include "AnimationClip.h"
#include <map>

struct Bone{
	Bone(const char* name):name(name) {}
	std::string  name;
	size_t       index;
	Game::Mat4x4 offset;
	std::vector<size_t> childIndex;
};

struct BoneAnimationKeyframeClip {
	Game::Vector3 position;
	Game::Vector3 scale;
	Game::Vector4 rotation;
};

//the num of the key frame clip equals the num of 
//bones on the bone heirarchy
struct BoneAnimationKeyframe {
	std::vector<BoneAnimationKeyframeClip> clips;
	float start, end;

	bool operator<(const BoneAnimationKeyframe& frame) {
		return start < frame.start;
	}
};


class BoneAnimationClip : public AnimationClip {
public:
	void Play(float time) override;
	D3D12_GPU_VIRTUAL_ADDRESS GetBoneTransformData() {
		return boneTransforms->GetADDR();
	}
	BoneAnimationClip(const std::vector<BoneAnimationKeyframe>& frames);
private:
	std::unique_ptr<ConstantBuffer<Game::Mat4x4>> boneTransforms;
	std::vector<BoneAnimationKeyframe> keyframes;
};

class BoneHeirarchy {
public:
	void  Pushback(Bone b,size_t* parentIndex = nullptr);
	Bone* Find(const char* name);
	Bone* Find(size_t index);
	BoneAnimationClip* GetAnimation() { return boneAnimation.get(); }
private:
	std::vector<Bone> bones;
	std::map<std::string, size_t> boneToName;
	std::unique_ptr<BoneAnimationClip> boneAnimation;
};