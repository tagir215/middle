#pragma once
#include <unordered_map>
#include <raylib.h>
#include <string>

namespace bubbleAssets{
	void loadAssets(std::vector<Shader>& shaders, std::vector<Texture>& textures);
	void loadGlobalFont(Font& font);
}
