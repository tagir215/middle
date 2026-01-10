#pragma once

#include <iostream>
#include "game.h"
#include "state_update.h"
#include "descart_loop.h"
#include "SystemReference.h"
#include "Constraint.h"
#include "PhysicsData.h"
#include "Position.h"
#include "Color.h"
#include "Sphere.h"
#include "MouseSelectable.h"
#include "MouseGrabbable.h"
#include "MouseIntersectable.h"

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

	scriptMap["MouseIntersectDetectionSystem"]->update(gameState);
	scriptMap["MouseGrabbingSystem"]->update(gameState);
	scriptMap["MouseSelectionSystem"]->update(gameState);

	// run scripts
	loopInstances(gameState, [gameState](int i, Shape& shape) {
		auto sysRef = getComponent<components::SystemReference>(shape);
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
		auto& shape = getShape(gameState, i);

		auto constraint = getComponent<components::Constraint>(shape);
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
		auto pcomp = getComponent<components::PhysicsData>(shape);
		auto pos = getComponent<components::Position>(shape);
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

void updateRenderData(GameState* gameState) {
	gameState->renderData.clear();
	for (int i = 0; i < gameState->shapes.size(); ++i) {
		auto& shape = gameState->shapes[i];
		auto color = getComponent<components::Color>(shape);
		if (!color)
			continue;


		RenderItem item;
		item.color.r = color->colorR;
		item.color.g = color->colorG;
		item.color.b = color->colorB;
		item.color.a = color->colorA;

		auto sphere = getComponent<components::Sphere>(shape);
		auto selectable = getComponent<components::MouseSelectable>(shape);
		auto intersectable = getComponent<components::MouseIntersectable>(shape);
		if (sphere) {
			item.center = getShapePosition(gameState, i);
			item.type = RenderItemType::SPHERE;
			item.radius = sphere->radius;

			if (selectable && selectable->selected) {
				RenderItem selectItem;
				selectItem.type = RenderItemType::RECTANGLE;
				selectItem.center = getShapePosition(gameState, i);
				selectItem.widht = item.radius * 4;
				selectItem.height = item.radius * 4;
				selectItem.length = item.radius * 4;
				selectItem.color = ColorAlpha(WHITE, 0.4f);
				gameState->renderData.push_back(selectItem);
			}

			if (intersectable && intersectable->intersecting) {
				item.color = WHITE;
			}

			gameState->renderData.push_back(item);
			continue;
		}
		auto constraint = getComponent<components::Constraint>(shape);
		if (constraint) {
			item.type = RenderItemType::LINE;
			item.linePointA = getShapePosition(gameState, constraint->indexA);
			item.linePointB = getShapePosition(gameState, constraint->indexB);
			gameState->renderData.push_back(item);
			continue;
		}

	}
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
		updateRenderData(gameState);
	}

}

void closeGame(GameState* gameState)
{
	saveEditorState(gameState);
}


