#pragma once
#include "RenderPass.h"
#include "Texture.h"


class SkyboxRenderPass : RenderPass {
public:
private:
	Texture* cubeTex;
	const wchar_t* default_skybox_path = L"../asserts/skybox";

};