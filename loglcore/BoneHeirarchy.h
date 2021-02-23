#pragma once
#include <string>
#include <vector>
#include "Math.h"
#include "ConstantBuffer.h"
#include "AnimationClip.h"
#include "Quaterion.h"
#include <map>

struct Bone{
	Bone(const char* name):name(name) {}
	std::string  name;
	size_t       index;
	Game::Mat4x4 offset;
	std::vector<size_t> childIndex;
};

class BoneHeirarchy {
	friend class BoneAnimationClip;
public:
	bool  Pushback(Bone& b, size_t* parent = nullptr);
	
	Bone* Find(const char* name);
	Bone* Find(size_t index);
	
	size_t GetBoneNum() { return bones.size(); }
private:
	void   CreateConstantBuffer();

	std::unique_ptr<ConstantBuffer<Game::Mat4x4>> boneTransforms;
	std::vector<Bone> bones;
	std::map<std::string, size_t> boneToName;
	std::vector<size_t> roots;
};

struct BoneAnimationNode {
	struct Position {
		Game::Vector3 position;
		float time;
	};

	struct Rotation {
		Game::Quaterion rotation;
		float time;
	};

	struct Scale {
		Game::Vector3 scale;
		float time;
	};

	BoneAnimationNode(std::vector<Position>& positions,
		std::vector<Rotation>& rotation,
		std::vector<Scale>& scale);
	BoneAnimationNode() {}

	Game::Mat4x4 Interpolate(float AnimationTick);
private:

	std::vector<Position> positionKeyframes;
	std::vector<Rotation> rotationKeyframes;
	std::vector<Scale> scaleKeyframes;
};

class BoneAnimationClip : public AnimationClip {
public:
	void Play(float time,ANIMATION_PLAY_TYPE type = ANIMATION_PLAY_TYPE_CLIP) override;
	inline D3D12_GPU_VIRTUAL_ADDRESS GetBoneTransformData() {
		return heir->boneTransforms->GetADDR();
	}
	BoneAnimationClip(const std::vector<BoneAnimationNode>& frames, BoneHeirarchy* heir
		,float tickPersecond,float totalTick,const char* name);
	const char* GetName() { return name.c_str(); }
	BoneHeirarchy* GetBoneHeirarchy() { return heir; }
private:
	void TranverseBoneHeirarchy(size_t node,Game::Mat4x4 rootTransform,float AnimationTick);

	std::vector<BoneAnimationNode> keyframes;
	BoneHeirarchy* heir;
	float tickPersecond;
	float totalTick;
	std::string name;
};