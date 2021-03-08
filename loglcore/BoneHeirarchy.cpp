#include "BoneHeirarchy.h"
#include <algorithm>
#include "graphic.h"

bool  BoneHeirarchy::Pushback(Bone& b,size_t* parent) {
	if (boneToName.count(b.name)
		&& (parent != nullptr && *parent >= bones.size())) {
		return false;
	}
	size_t index = bones.size();
	b.index = index;
	bones.push_back(b);
	if (parent != nullptr) {
		bones[*parent].childIndex.push_back(b.index);
	}
	else {
		roots.push_back(b.index);
	}
	boneToName[b.name] = index;
	return true;
}

Bone*  BoneHeirarchy::Find(const char* name) {
	if (auto item = boneToName.find(name);item != boneToName.end()) {
		return &bones[item->second];
	}
	return nullptr;
}

Bone*  BoneHeirarchy::Find(size_t index) {
	if (index < bones.size()) {
		return &bones[index];
	}
	return nullptr;
}


BoneAnimationNode::BoneAnimationNode(std::vector<Position>& positions,
	std::vector<Rotation>& rotation,
	std::vector<Scale>& scale) :positionKeyframes(positions),
	rotationKeyframes(rotation),
	scaleKeyframes(scale)
{
	std::sort(positionKeyframes.begin(), positionKeyframes.end(),
		[](const Position& lhs,const Position& rhs) {
			return lhs.time < rhs.time;
		});
	std::sort(rotationKeyframes.begin(), rotationKeyframes.end(),
		[](const Rotation& lhs,const Rotation& rhs) {
			return lhs.time < rhs.time;
		});
	std::sort(scaleKeyframes.begin(), scaleKeyframes.end(),
		[](const Scale& lhs, const Scale& rhs) {
			return lhs.time < rhs.time;
		});
}



Game::Mat4x4 BoneAnimationNode::Interpolate(float AnimationTick) {
	Game::Vector3 position,scale;
	Game::Quaterion rotation;
	if (!positionKeyframes.empty()){
		auto postPos = std::lower_bound(positionKeyframes.begin(), positionKeyframes.end(),
			AnimationTick, [](const Position& lhs, float time) {
			return lhs.time < time;
		});
		if (postPos == positionKeyframes.end()) {
			position = positionKeyframes.rbegin()->position;
		}
		else if (postPos == positionKeyframes.begin()) {
			position = postPos->position;
		}
		else {
			auto prePos = postPos - 1;
			float factor = (AnimationTick - prePos->time) / (postPos->time - prePos->time);
			position = prePos->position * (1. - factor) + postPos->position * factor;
		}
	}
	if (!scaleKeyframes.empty()) {
		auto postScale = std::lower_bound(scaleKeyframes.begin(), scaleKeyframes.end(), AnimationTick,
			[](const Scale& lhs,float rhs) {
				return lhs.time < rhs;
			});
		if (postScale == scaleKeyframes.end()) {
			scale = scaleKeyframes.rbegin()->scale;
		}
		else if (postScale == scaleKeyframes.begin()) {
			scale = postScale->scale;
		}
		else {
			auto preScale = postScale - 1;

			float factor = (AnimationTick - preScale->time) / (postScale->time - preScale->time);
			scale = preScale->scale * (1. - factor) + postScale->scale * factor;
		}
	}
	else {
		scale = Game::Vector3(1., 1., 1.);
	}
	if (!rotationKeyframes.empty()) {
		auto postRotation = std::lower_bound(rotationKeyframes.begin(), rotationKeyframes.end(),
			AnimationTick, [](const Rotation& lhs, float rhs) {
								return lhs.time < rhs;
							}
			);
		if (postRotation == rotationKeyframes.end()) {
			rotation = rotationKeyframes.rbegin()->rotation;
		}
		else if (postRotation == rotationKeyframes.begin()) {
			rotation = postRotation->rotation;
		}
		else {
			auto preRotation = postRotation - 1;

			float factor = (AnimationTick - preRotation->time) / (postRotation->time - preRotation->time);
			rotation = Game::Quaterion::SLerp(preRotation->rotation, postRotation->rotation, factor);
		}
	}
	return Game::PackTransformQuaterion(position,rotation,scale);
}

void BoneAnimationClip::Interpolate(float time,Game::Mat4x4* boneTransformBuffer) {
	float AnimationTick = 0.;
	float timeTick = tickPersecond * time;
	AnimationTick = fmod(timeTick, totalTick);
	
	//tmp.resize(heir->GetBoneNum());
	//tmp1.resize(heir->GetBoneNum());
	for (auto bi : heir->roots) {
		TranverseBoneHeirarchy(bi, Game::Mat4x4::I(),AnimationTick,
			boneTransformBuffer);
	}
}


void BoneAnimationClip::TranverseBoneHeirarchy(size_t index, Game::Mat4x4 parentToRoot,float AnimationTick,
	Game::Mat4x4* boneTransformBuffer) {
	Bone* bone = heir->Find(index);
	if (bone == nullptr) {
		OUTPUT_DEBUG_STRING("fail to find bone while playing animation\n");
		return;
	}
	BoneAnimationNode* boneAniNode = &keyframes[index];
	Game::Mat4x4 boneAniMat = boneAniNode->Interpolate(AnimationTick);
	Game::Mat4x4 boneTransform = Game::mul(parentToRoot, boneAniMat);
	Game::Mat4x4 boneFinalTransform = Game::mul(boneTransform, bone->offset);
	
	boneTransformBuffer[index] = boneFinalTransform.T();
	for (auto& child : bone->childIndex) {
		//the offset of the bone doesn't affect 
		TranverseBoneHeirarchy(child, boneTransform, AnimationTick,boneTransformBuffer);
	}

	//tmp[index] = boneAniMat;
	//tmp1[index] = boneFinalTransform;
}

BoneAnimationClip::BoneAnimationClip(const std::vector<BoneAnimationNode>& frames, BoneHeirarchy* heir
	, float tickPersecond, float totalTick,const char* name) :
		keyframes(frames), heir(heir),name(name),
		tickPersecond(tickPersecond),totalTick(totalTick){
	if (tickPersecond == 0) tickPersecond = 25.f;
}
