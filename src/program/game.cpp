#pragma once

#include <iostream>
#include "game.h"
#include "state_update.h"
#include "descart_loop.h"

using namespace middle;



__declspec(dllexport) void UpdateGame(GameState* gameState)
{
	if (gameState->closeGame) {
		closeGame(gameState);
		return;
	}

	if (gameState->applicationMode == ApplicationMode::EDITOR_MODE) {
		updateEditor(gameState);
	}

	if (gameState->applicationMode == ApplicationMode::GAME_MODE) {

		// TODO for now just uses editor camera
		ShapeInstance& activeCamera = getShapeInstance(gameState, gameState->activeCameraIndex);
		moveCameraXZ(gameState->editorState.camera, FromDescVec(activeCamera.pData.position));
	}

	// run scripts
	loopInstances(gameState, [gameState](int i, ShapeInstance& instance) {
		if (instance.shape.type == ShapeType::SCRIPT) {
			auto scriptName = instance.shape.name;
			assert(gameState->gameplayScripts.find(scriptName) != gameState->gameplayScripts.end());
			gameState->gameplayScripts[scriptName]->onUpdate(gameState);
		}
		});


	// shape phsyics stuff
	// create pairs

	std::vector<Constraint> constraints;
	std::vector<BodyPair> pairs;
	std::vector<int> grounds;

	if (physicsBodies.size() < MAX_SHAPE_COUNT) {
		physicsBodies.resize(MAX_SHAPE_COUNT);
	}


	for (int i = 0; i < gameState->shapes.size(); ++i) {
		if (!isShapeAlive(gameState, i)) {
			physicsBodies[i] = nullptr;
			continue;
		}
		auto& instance = getShapeInstance(gameState, i);

		if (instance.shape.type == ShapeType::CONSTRAINT) {
			if (instance.shape.constraint.indexA != UNASSIGNED && instance.shape.constraint.indexB != UNASSIGNED)
				constraints.push_back(instance.shape.constraint);
		}
		if (instance.shape.type == ShapeType::SPHERE) {
			for (int j : grounds) {
				BodyPair pair;
				pair.indexA = i;
				pair.indexB = j;
				pairs.push_back(pair);
			}
			physicsBodies[i] = &instance.pData;
		}
	}

	DescLoop(gameState->frameTime, pairs, constraints, physicsBodies);

}

void closeGame(GameState* gameState)
{
	saveEditorState(gameState);
}


