#pragma once

#include <iostream>
#include "game.h"
#include "descart_loop.h"
#include "SystemReference.h"
#include "Position.h"
#include "middle_shape_utils.h"

using namespace middle;

namespace middle{


	void physicsUpdate(GameState* gameState) {

		if (gameState->applicationMode == ApplicationMode::GAME_MODE) {

			// TODO for now just uses editor camera
			Shape& activeCamera = getShape(gameState, gameState->activeCameraIndex);
			auto pos = getComponent<components::Position>(activeCamera);
			assert(pos != nullptr);
			Vector3 p = { pos->posX, pos->posY, pos->posZ };
			moveCameraXZ(gameState->editorState.camera, p);
		}

		for (auto& name : engineSystemNamesFrameStart) {
			systemMap[name]->update(gameState);
		}

		// run gameplay systems
		loopInstances(gameState, [gameState](int i, Shape& shape) {
			auto sysRef = getComponent<components::SystemReference>(shape);
			if (sysRef != nullptr) {
				auto systemName = sysRef->systemName;
				if (gameState->gameplaySystems.find(systemName) == gameState->gameplaySystems.end())
					return;
				assert(gameState->gameplaySystems.find(systemName) != gameState->gameplaySystems.end());
				gameState->gameplaySystems[systemName]->update(gameState);
			}
			});

		for (auto& name : engineSystemNamesFrameEnd) {
			systemMap[name]->update(gameState);
		}

	}

}

extern "C" {

	__declspec(dllexport) void UpdateGame(GameState* gameState)
	{
		if (gameState->closeGame) {
			closeGame(gameState);
			return;
		}

		if (gameState->frameTimeAccumulator >= gameState->frameTime)
		{
			gameState->frameTimeAccumulator -= gameState->frameTime;
			physicsUpdate(gameState);
		}

		for (auto& name : engineRendererSystemNames) {
			systemMap[name]->update(gameState);
		}

	}

}

void closeGame(GameState* gameState)
{
	saveEditorState(gameState);
}


