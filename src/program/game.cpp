#pragma once

#include <iostream>
#include "game.h"
#include "state_update.h"
#include "descart_loop.h"
#include "SystemReference.h"
#include "Constraint.h"
#include "PhysicsData.h"
#include "Position.h"

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
		auto pos = getComponent<components::Position>(activeCamera.shape);
		assert(pos != nullptr);
		Vector3 p = { pos->posX, pos->posY, pos->posZ };
		moveCameraXZ(gameState->editorState.initCamera, p);
	}

	// run scripts
	loopInstances(gameState, [gameState](int i, ShapeInstance& instance) {
		auto sysRef = getComponent<components::SystemReference>(instance.shape);
		if (sysRef != nullptr) {
			auto scriptName = sysRef->systemName;
			if (gameState->gameplayScripts.find(scriptName) == gameState->gameplayScripts.end())
				return;
			assert(gameState->gameplayScripts.find(scriptName) != gameState->gameplayScripts.end());
			gameState->gameplayScripts[scriptName]->update(gameState);
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

		auto constraint = getComponent<components::Constraint>(instance.shape);
		if (constraint) {
			if (constraint->indexA != UNASSIGNED && constraint->indexB != UNASSIGNED) {
				Constraint c;
				c.indexA = constraint->indexA;
				c.indexB = constraint->indexB;
				c.biasFactor = constraint->biasFactor;
				c.stiffness = constraint->stiffness;
				constraints.push_back(c);
			}
		}
		auto pcomp = getComponent<components::PhysicsData>(instance.shape);
		auto pos = getComponent<components::Position>(instance.shape);
		if (pcomp) {
			assert(pos != nullptr);
			PhysicsBody* body = physicsBodies[i];
			body->infiniteMass = pcomp->infiniteMass;
			body->mass = pcomp->mass;
			body->invMass = pcomp->invMass;
			body->momentOfInertia = pcomp->momentOfInertia;
			body->invMomentOfInertia = pcomp->invMomentOfInertia;
			body->colliderType = ColliderType::CIRC;
			body->linearAcc = { pcomp->accX, pcomp->accY, pcomp->accZ };
			body->linearVel = { pcomp->velX, pcomp->velY, pcomp->velZ };
			body->position = { pos->posX, pos->posY, pos->posZ };
		}
	}

	DescLoop(gameState->frameTime, pairs, constraints, physicsBodies);

}

void closeGame(GameState* gameState)
{
	saveEditorState(gameState);
}


