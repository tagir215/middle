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

	}

	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto camera = middle::getComponent<components::CameraComponent>(shape);
			if (!camera)
				return true;

			auto position = middle::getComponent<components::Position>(shape);
			Vector3 pos = { position->posX, position->posY, position->posZ };


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


			Vector3 newPos = middle::getShapePosition(gameState, i);
			gameState->activeCamera.position = { position->posX, position->posY, position->posZ };
			gameState->activeCamera.target = { camera->targetX, camera->targetY, camera->targetZ };

			static const Vector3 forward = { 0,10000,0 };
			Vector3 target = gameState->activeCamera.position + forward;
			gameState->activeCamera.target = target;
			camera->targetX = target.x;
			camera->targetY = target.y;
			camera->targetZ = target.z;
			return true;
			});
	}
};

static middle::SystemRegistrar<BubbleCameraSystem> reg("BubbleCameraSystem");
