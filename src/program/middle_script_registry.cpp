#include "middle_script_registry.h"
#include <unordered_map>

namespace middle {
	std::unordered_map<std::string, GameScript> scriptRegistry;
	void runScript(GameState* gameState, int index)
	{
		std::string sceneName = gameState->sceneNames[gameState->activeScene];
		std::string name = sceneName + std::to_string(index);
		if (scriptRegistry.find(name) != scriptRegistry.end()) {
			scriptRegistry[name](gameState);
		}
	}
	void registerScript(std::string name, GameScript script)
	{
		scriptRegistry[name] = script;
	}
	bool scriptExists(const std::string name)
	{
		return scriptRegistry.find(name) != scriptRegistry.end();
	}
}

