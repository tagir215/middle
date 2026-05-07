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

			const float minDeceleration = 10;
			float oldSpeedY = camera->speedY;;
			float speedMag = std::abs(camera->speedY);
			float yDeceleration = speedMag * 0.3f;
			yDeceleration = yDeceleration > minDeceleration ? yDeceleration : minDeceleration;
			if (speedMag > 0) {
				camera->speedY -= yDeceleration * (camera->speedY / speedMag);
				if (camera->speedY * oldSpeedY <= 0) {
					camera->speedY = 0;
				}
			}
			float mouseWheelMove = gameState->gameInput.mouseWheelMove;
			const float wheelMouseMultiplier = 70;
			camera->speedY += mouseWheelMove * wheelMouseMultiplier;
			camera->speedX = 0;
			camera->speedZ = 0;

			const float panSpan = 3500;
			const float minY = -100;
			float maxY = minY - panSpan;

			Vector3 oldPos = middle::getShapePosition(gameState, shape.id.index);
			float zoomRatio = std::abs(oldPos.y - minY) / panSpan;
			float panSpeed = 50 * zoomRatio;
			const float minPanSpeed = 0.1f;
			if (panSpeed < minPanSpeed) {
				panSpeed = minPanSpeed;
			}

			const float centerOffsetX = 200;
			const float centerOffsetZ = 0;
			const float minX = -400 + centerOffsetX;
			const float maxX = 400 + centerOffsetX;
			const float minZ = -400 + centerOffsetZ;
			const float maxZ = 400 + centerOffsetZ;

			if (gameState->gameInput.panLeft && oldPos.x > minX) {
				camera->speedX = -panSpeed;
			}
			if (gameState->gameInput.panRight && oldPos.x < maxX) {
				camera->speedX = panSpeed;
			}
			if (gameState->gameInput.panUp && oldPos.z < maxZ) {
				camera->speedZ = panSpeed;
			}
			if (gameState->gameInput.panDown && oldPos.z > minZ) {
				camera->speedZ = -panSpeed;
			}


			middle::moveShape(gameState, shape.id.index, { camera->speedX,camera->speedY,camera->speedZ });

			Vector3 newPos = middle::getShapePosition(gameState, shape.id.index);

			float deltaMinY = newPos.y - minY;
			if (deltaMinY > 0) {
				middle::moveShape(gameState, shape.id.index, { camera->speedX,-deltaMinY,camera->speedZ });
				newPos.y = minY;
			}
			float deltaMaxY = newPos.y - maxY;
			if (deltaMaxY < 0) {
				middle::moveShape(gameState, shape.id.index, { camera->speedX,-deltaMaxY,camera->speedZ });
				newPos.y = maxY;
			}

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
