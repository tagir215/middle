#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "middle_component_table.h"
#include "CameraComponent.h"
#include "Position.h"

class BubbleCameraSystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto camera = middle::getComponent<components::CameraComponent>(shape);
			if (!camera)
				return;

			auto position = middle::getComponent<components::Position>(shape);
			Vector3 pos = { position->posX, position->posY, position->posZ };

			static const Vector3 forward = { 0,1,0 };
			Vector3 target = pos + forward;

			camera->targetX = target.x;
			camera->targetY = target.y;
			camera->targetZ = target.z;

			const float zoomSpeed = 2;
			if (gameState->gameInput.zoomIn) {
				middle::moveShape(gameState, shape.id.index, { 0,zoomSpeed,0 });
			}
			if (gameState->gameInput.zoomOut) {
				middle::moveShape(gameState, shape.id.index, { 0,-zoomSpeed,0 });
			}

			});
	}
};

static middle::SystemRegistrar<BubbleCameraSystem> reg("BubbleCameraSystem");
