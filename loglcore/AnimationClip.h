#pragma once

enum ANIMATION_PLAY_TYPE {
	ANIMATION_PLAY_TYPE_CLIP, 
	ANIMATION_PLAY_TYPE_LOOP
};


class AnimationClip {
public:
	virtual void Play(float time,ANIMATION_PLAY_TYPE type = ANIMATION_PLAY_TYPE_CLIP) = 0;
};