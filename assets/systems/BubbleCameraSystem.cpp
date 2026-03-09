#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "middle_component_table.h"
#include "CameraComponent.h"
#include "Position.h"
#include "comp_cache.h"

class BubbleCameraSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* compCache;

	void init(middle::GameState* gameState) {
		compCache = middle::newCompCache(gameState);
		compCache->addType<components::CameraComponent>();
	}

	void update(middle::GameState* gameState) override {

		auto cameraIt = compCache->begin<components::CameraComponent>();

		int size = compCache->getSize();
		for (int i = 0; i < compCache->getSize(); ++i) {
			auto camera = *cameraIt;
			auto& shape = middle::getShape(gameState, compCache->relevantIdVector[i].index);

			const float cameraSpeed = 2;
			if (gameState->gameInput.zoomIn) {
				middle::moveShape(gameState, shape.id.index, { 0,cameraSpeed,0 });
			}
			if (gameState->gameInput.zoomOut) {
				middle::moveShape(gameState, shape.id.index, { 0,-cameraSpeed,0 });
			}
			if (gameState->gameInput.panUp) {
				middle::moveShape(gameState, shape.id.index, { 0,0,cameraSpeed });
			}
			if (gameState->gameInput.panDown) {
				middle::moveShape(gameState, shape.id.index, { 0,0,-cameraSpeed });
			}
			if (gameState->gameInput.panLeft) {
				middle::moveShape(gameState, shape.id.index, { -cameraSpeed, 0,0 });
			}
			if (gameState->gameInput.panRight) {
				middle::moveShape(gameState, shape.id.index, { cameraSpeed, 0,0 });
			}

			Vector3 newPos = middle::getShapePosition(gameState, shape.id.index);
			gameState->activeCamera.position = newPos;
			gameState->activeCamera.target = { camera->targetX, camera->targetY, camera->targetZ };

			static const Vector3 forward = { 0,10000,0 };
			Vector3 target = gameState->activeCamera.position + forward;
			gameState->activeCamera.target = target;
			camera->targetX = target.x;
			camera->targetY = target.y;
			camera->targetZ = target.z;
		}
	}
};

static middle::SystemRegistrar<BubbleCameraSystem> reg("BubbleCameraSystem");
