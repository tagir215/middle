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
#include "LoopSociety.h"
#include "JointEntity.h"
#include "LoopEntity.h"
#include "ConstraintEntity.h"

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
		physicsBodies[i].active = false;
		if (!isShapeAlive(gameState, i)) {
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
				c.targetDistance = constraint->targetDistance;
				c.type = ConstraintType::distance;
				constraints.push_back(c);
			}
		}
		auto pcomp = getComponent<components::PhysicsData>(shape);
		auto pos = getComponent<components::Position>(shape);
		if (pcomp) {
			physicsBodies[i].active = true;
			assert(pos != nullptr);
			if (physicsBodies.size() <= i) {
				physicsBodies.resize(i + 100);
			}
			PhysicsBody& body = physicsBodies[i];
			body.infiniteMass = pcomp->infiniteMass;
			body.mass = pcomp->mass;
			body.invMass = pcomp->invMass;
			body.momentOfInertia = pcomp->momentOfInertia;
			body.invMomentOfInertia = pcomp->invMomentOfInertia;
			body.colliderType = ColliderType::CIRC;
			body.linearAcc = { pcomp->accX, pcomp->accY, pcomp->accZ };
			body.linearVel = { pcomp->velX, pcomp->velY, pcomp->velZ };
			body.linearDamping = pcomp->damY;
			body.position = { pos->posX, pos->posY, pos->posZ };
		}
	}

	DescLoop(gameState->frameTime, pairs, constraints, physicsBodies);

	loopInstances(gameState, [&](int i, Shape& shape) {
		auto pcomp = getComponent<components::PhysicsData>(shape);
		auto pos = getComponent<components::Position>(shape);
		if (pcomp != nullptr && pos != nullptr) {
			PhysicsBody& body = physicsBodies[i];
			pcomp->velX = body.linearVel.x;
			pcomp->velY = body.linearVel.y;
			pcomp->velZ = body.linearVel.z;
			pos->posX = body.position.x;
			pos->posY = body.position.y;
			pos->posZ = body.position.z;
		}
		});
}

void updateRenderData(GameState* gameState) {
	gameState->renderData.clear();
	for (int i = 0; i < gameState->shapes.size(); ++i) {
		auto& shape = gameState->shapes[i];

		if (isEntityOfType(gameState, i, entities::LoopEntity)) {
			auto loop = getComponent<components::LoopSociety>(shape);
			RenderItem loopItem;
			Vector3 centroid = getLoopCentroid(gameState, i);
			loopItem.type = RenderItemType::SPHERE;
			loopItem.center = centroid;
			loopItem.radius = DEF_RADIUS_LOOP_INDICATOR;
			loopItem.color = LOOP_INDICATOR_COLOR;
			gameState->renderData.push_back(loopItem);
		}

		if (isEntityOfType(gameState, i, entities::JointEntity)) {
			auto sphere = getComponent<components::Sphere>(shape);
			auto pos = getComponent<components::Position>(shape);
			auto intersectable = getComponent<components::MouseIntersectable>(shape);
			auto selectable = getComponent<components::MouseSelectable>(shape);
			RenderItem sphereItem;
			sphereItem.type = RenderItemType::SPHERE;
			sphereItem.radius = sphere->radius;
			sphereItem.center = { pos->posX, pos->posY, pos->posZ };
			sphereItem.color = JOINT_COLOR;
			if (intersectable && intersectable->intersecting) {
				sphereItem.color = HOVERED_THING_COLOR;
			}
			gameState->renderData.push_back(sphereItem);

			if (selectable && selectable->selected) {
				RenderItem selectItem;
				selectItem.type = RenderItemType::RECTANGLE;
				selectItem.center = getShapePosition(gameState, i);
				selectItem.widht = sphereItem.radius * 4;
				selectItem.height = sphereItem.radius * 4;
				selectItem.length = sphereItem.radius * 4;
				selectItem.color = ColorAlpha(WHITE, 0.4f);
				gameState->renderData.push_back(selectItem);
			}

			continue;
		}
		if (isEntityOfType(gameState, i, entities::ConstraintEntity)) {
			auto constraint = getComponent<components::Constraint>(shape);
			RenderItem lineItem;
			lineItem.type = RenderItemType::LINE;
			lineItem.linePointA = getShapePosition(gameState, constraint->indexA);
			lineItem.linePointB = getShapePosition(gameState, constraint->indexB);
			gameState->renderData.push_back(lineItem);
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


