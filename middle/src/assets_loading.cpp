#include "assets_loading.h"
#include <filesystem>
#include "bubble_paths.h"
#include <iostream>
#include "raylib.h"

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

	void loadAssets(middle::GameState* gameState)
	{
		// load textures
		std::vector<std::string>texturePaths;
		std::vector<std::string>filenames;
		const std::string textureFolderPath = bubblePaths::TEXTURES_FOLDER;
		getPathsAndNames(textureFolderPath, texturePaths, filenames);

		for (int i = 0; i<texturePaths.size(); ++i){
			const std::string& path = texturePaths[i];
			const std::string& name = filenames[i];
			middle::TextureContainer container;
			container.texture = LoadTexture(path.c_str());
			gameState->textureMap[name] = container;
		}


		// load shaders
		std::vector<std::string> shaderPaths;
		std::vector<std::string> shaderNames;
		const std::string shaderFolderPath = bubblePaths::SHADERS_FOLDER;
		getPathsAndNames(shaderFolderPath, shaderPaths, shaderNames);

		for (int i = 0; i < shaderPaths.size(); ++i) {
			auto& path = shaderPaths[i];
			auto& name = shaderNames[i];
			Shader shader = LoadShader(0, path.c_str());
			middle::ShaderContainer container;
			container.shader = shader;
			gameState->shaderMap[name] = container;
		}
	}
}
