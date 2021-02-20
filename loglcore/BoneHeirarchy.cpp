#include "BoneHeirarchy.h"
#include <algorithm>
#include "graphic.h"

BoneAnimationClip::BoneAnimationClip(const std::vector<BoneAnimationKeyframe>& frames):
	keyframes(frames) {
	std::sort(keyframes.begin(), keyframes.end());
	ID3D12Device* device = gGraphic.GetDevice();
	boneTransforms = std::make_unique<ConstantBuffer<Game::Mat4x4>>(device, frames.size(), true);
	Play(0.);
}