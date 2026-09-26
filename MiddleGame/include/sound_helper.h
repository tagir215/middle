#pragma once
#include "game_state.h"
#include <filesystem>
#include <raylib.h>
#include "midconfig.h"

namespace middleSoundHelpers{
	void loadSoundEffects(std::unordered_map<std::string, Sound>& soundMap)
	{
		namespace fs = std::filesystem;
		std::string folder = std::string(middlePaths::SOUNDS_FOLDER) + "/";
		for (const auto& entry : fs::directory_iterator(folder)) {
			std::string name = entry.path().stem().string();
			Sound sound = LoadSound(entry.path().string().c_str());
			soundMap[name] = sound;
		}
	}

	void playSoundEffects(std::vector<Sound>& sounds) {
		for (Sound& sound : sounds) {
			PlaySound(sound);
		}
		sounds.clear();
	}

}
