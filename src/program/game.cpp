#pragma once

#include <iostream>
#include "game.h"
#include "descart_loop.h"
#include "SystemReference.h"
#include "Position.h"
#include "middle_shape_utils.h"
#include "engine_system_names.h"

using namespace middle;

namespace middle{

	void registerSystems(middle::GameState* gameState) {
		auto& systemMap = middle::getSystemMap();

		// register engine systems
		for (auto& name : middle::engineSystemNamesFrameStart) {
			auto& ptr = systemMap[name];
			gameState->engineSystemsFrameStart.push_back(std::move(ptr));
		}
		for (auto& name : middle::engineSystemNamesFrameEnd) {
			gameState->engineSystemsFrameEnd.push_back(std::move(systemMap[name]));
		}
		for (auto& name : middle::engineRendererSystemNames) {
			gameState->engineRendererSystems.push_back(std::move(systemMap[name]));
		}

		// register gameplay systems
		for (auto& pair : systemMap) {
			std::string name = pair.first;
			// check that the system still exists
			auto sysptr = pair.second.get();
			if (sysptr) {
				gameState->gameplaySystems[name] = std::move(pair.second);
			}
		}

		gameState->systemsRegistered = true;
	}


	void physicsUpdate(GameState* gameState) {

		if (gameState->applicationMode == ApplicationMode::GAME_MODE) {

			// TODO for now just uses editor camera
			Shape& activeCamera = getShape(gameState, gameState->activeCameraIndex);
			auto pos = getComponent<components::Position>(activeCamera);
			assert(pos != nullptr);
			Vector3 p = { pos->posX, pos->posY, pos->posZ };
			moveCameraXZ(gameState->editorState.camera, p);
		}

		for (auto& system : gameState->engineSystemsFrameStart) {
			system->update(gameState);
		}

		// run gameplay systems
		loopInstances(gameState, [gameState](int i, Shape& shape) {
			// don't update systems in ghost shapes
			if (isGhostShape(i))
				return;

			auto sysRef = getComponent<components::SystemReference>(shape);
			if (sysRef != nullptr) {
				auto systemName = sysRef->systemName;
				auto& system = gameState->gameplaySystems[systemName];
				system->update(gameState);
			}
			});

		for (auto& system : gameState->engineSystemsFrameEnd) {
			system->update(gameState);
		}


		// Clear input blockers at the end of physics update
		gameState->inputBlockers.clear();

	}

}

extern "C" {

	__declspec(dllexport) void UpdateGame(GameState* gameState)
	{
		if (gameState->closeGame) {
			closeGame(gameState);
			return;
		}

		if (!gameState->systemsRegistered) {
			registerSystems(gameState);
		}

		if (gameState->frameTimeAccumulator >= gameState->frameTime)
		{
			gameState->frameTimeAccumulator -= gameState->frameTime;
			if (gameState->frameTimeAccumulator > gameState->frameTime * 2) {
				gameState->frameTimeAccumulator = 0;
			}
			physicsUpdate(gameState);
		}

		for (auto& renderSystem : gameState->engineRendererSystems) {
			renderSystem->update(gameState);
		}

	}

}

void closeGame(GameState* gameState)
{
	saveEditorState(gameState);
}


