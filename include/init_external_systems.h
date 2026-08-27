#pragma once
#include "game_state.h"
#include "middle_gameplay_script_map.h"
namespace middle{

	void initExternalSystems(middle::GameState* gameState) {
		// these are called in middle project
		auto& systemMap = getSystemMap();
		auto inputSystem = std::shared_ptr(std::move(systemMap["InputSystem"]));
		auto renderSystem = std::shared_ptr(std::move(systemMap["RendererSystem"]));
		auto fileDropSystem = std::shared_ptr(std::move(systemMap["FileDropSystem"]));

		gameState->externalPreFrameSystems.push_back(inputSystem);
		gameState->externalPreFrameSystems.push_back(fileDropSystem);
		gameState->externalPostFrameSystems.push_back(renderSystem);
	}
}
