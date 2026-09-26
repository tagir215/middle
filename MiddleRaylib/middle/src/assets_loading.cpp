#include "assets_loading.h"
#include <filesystem>
#include "bubble_paths.h"
#include <iostream>
#include "raylib.h"
#include "midconfig.h"
#include "asset_enums.h"
#include <cassert>

namespace bubbleAssets {

	namespace fs = std::filesystem;

	void getPathsAndNames(std::string dir, std::vector<std::string>& paths, std::vector<std::string>& names) {

		try {
			if (fs::exists(dir) && fs::is_directory(dir)) {
				for (const auto& entry : fs::directory_iterator(dir)) {
					if (fs::is_regular_file(entry.status())) {
						paths.push_back(entry.path().string());
						names.push_back(entry.path().filename().stem().string());
					}
				}
			}
		} catch (const fs::filesystem_error& e) {
			std::cerr << "Error: " << e.what() << '\n';
		}
	}

	middleAssets::TEXTURE textureEnum(const std::string& name) {
		if (name == "background") {
			return middleAssets::TEXTURE::BACKGROUND;
		}
		else if (name == "and_icon") {
			return middleAssets::TEXTURE::AND_ICON;
		}
		else if (name == "gate_icon") {
			return middleAssets::TEXTURE::GATE_ICON;
		}
		assert(false);
	}

	middleAssets::SHADER shaderEnum(const std::string& name) {
		if (name == "bubbleShader") {
			return middleAssets::SHADER::BUBBLE_SHADER;
		}
		assert(false);
	}

	void loadAssets(std::vector<Shader>& shaders, std::vector<Texture>& textures)
	{
		// load textures
		std::vector<std::string>texturePaths;
		std::vector<std::string>filenames;
		const std::string textureFolderPath = std::string(middlePaths::TEXTURES_FOLDER);
		getPathsAndNames(textureFolderPath, texturePaths, filenames);

		for (int i = 0; i<texturePaths.size(); ++i){
			const std::string& path = texturePaths[i];
			const std::string& name = filenames[i];
			Texture texture = LoadTexture(path.c_str());
			int index = textureEnum(name);
			if (textures.size() <= index) {
				textures.resize(index + 10);
			}
			textures[index] = texture;
		}


		// load shaders
		std::vector<std::string> shaderPaths;
		std::vector<std::string> shaderNames;
		const std::string shaderFolderPath = std::string(middlePaths::SHADERS_FOLDER);
		getPathsAndNames(shaderFolderPath, shaderPaths, shaderNames);

		for (int i = 0; i < shaderPaths.size(); ++i) {
			auto& path = shaderPaths[i];
			auto& name = shaderNames[i];
			Shader shader = LoadShader(0, path.c_str());
			int index = shaderEnum(name);
			if (shaders.size() <= index) {
				shaders.resize(index + 10);
			}
			shaders[index] = shader;
		}
	}
	void loadGlobalFont(Font& font)
	{
		// load font TODO move somewhere
		int codepoints[] = {
			// Basic ASCII (32 to 126) for standard numbers and text
			32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
			50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
			68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85,
			86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
			103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
			117, 118, 119, 120, 121, 122, 123, 124, 125, 126,

			// Custom Math Codepoints
			0x00D7,   // Multiplication sign (×)
			0x22C5,   // Dot operator (⋅)
			0x2211,    // Summation operator (∑)
		};
		int codepointCount = sizeof(codepoints) / sizeof(codepoints[0]);
		const int fontUnitFactor = 1024;
		std::string fontPath = std::string(middlePaths::FONTS_FOLDER) + "/math-sans/NotoSansMath-Regular.ttf";
		font = LoadFontEx(fontPath.c_str(), fontUnitFactor, codepoints, codepointCount);
		GenTextureMipmaps(&font.texture);
		SetTextureFilter(font.texture, TEXTURE_FILTER_TRILINEAR);
	}
}
