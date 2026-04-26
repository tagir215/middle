#pragma once
#include "game_state.h"
#include <filesystem>

namespace middle{
	void loadSoundEffects(GameState* gameState)
	{
		namespace fs = std::filesystem;
		std::vector<std::string>& soundFileNames = gameState->sceneNames;

		std::string folder = "../assets/sounds/";
		for (const auto& entry : fs::directory_iterator(folder)) {
			std::string name = entry.path().stem().string();
			Sound sound = LoadSound(entry.path().string().c_str());
			gameState->soundMap[name] = sound;
		}
	}

	void playSoundEffects(GameState* gameState) {
		while (gameState->soundQueue.size() > 0) {
			PlaySound(gameState->soundQueue.front());
			gameState->soundQueue.pop();
		}
	}

}
