#pragma once

#include <iostream>
#include "game.h"
#include "descart_loop.h"
#include "SystemReference.h"
#include "Position.h"

using namespace middle;


void physicsUpdate(GameState* gameState) {
	if (gameState->applicationMode == ApplicationMode::EDITOR_MODE) {
		updateEditor(gameState);
	}

	if (gameState->applicationMode == ApplicationMode::GAME_MODE) {

		// TODO for now just uses editor camera
		Shape& activeCamera = getShape(gameState, gameState->activeCameraIndex);
		auto pos = getComponent<components::Position>(activeCamera);
		assert(pos != nullptr);
		Vector3 p = { pos->posX, pos->posY, pos->posZ };
		moveCameraXZ(gameState->editorState.camera, p);
	}

	systemMap["MouseIntersectDetectionSystem"]->update(gameState);
	systemMap["MouseGrabbingSystem"]->update(gameState);
	systemMap["MouseSelectionSystem"]->update(gameState);

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

	systemMap["MiddlePhysicsSystem"]->update(gameState);

}

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

	systemMap["EditorRenderSetupSystem"]->update(gameState);

}

void closeGame(GameState* gameState)
{
	saveEditorState(gameState);
}


