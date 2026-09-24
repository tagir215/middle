#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "CameraComponent.h"
#include "GlobalTransform.h"
#include "middle_shape_utils.h"

class CameraSystem : public middle::MiddleGameplaySystem {
public:
	CameraSystem() {
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
		systemModeType = middle::SystemModeType::ENGINE;
	}

	components::CompCache* cameraCache;

	void init(middle::GameState* gameState) {
		cameraCache = middle::newCompCache(gameState, systemName);
		cameraCache->addType<components::CameraComponent>();
	}

	void update(middle::GameState* gameState) override {

		if (gameState->middleState.applicationMode == middle::ApplicationMode::EDITOR_MODE) {
			auto& input = gameState->middleState.input;
			// camera controls
			const float maxCameraSpeed = 60;
			float mouseCamRatio = input.mousePos.y / gameState->middleState.screenHeight;
			const float cameraSpeed = mouseCamRatio * mouseCamRatio * mouseCamRatio * maxCameraSpeed;
			midMath::Vector3 cameraMovementDir = { 0,0,0 };
			if (!input.altDown && input.w)
				cameraMovementDir += midMath::Vector3Normalize(gameState->editorState.camera.target - gameState->editorState.camera.position);
			if (!input.altDown && input.s)
				cameraMovementDir += midMath::Vector3Negate(midMath::Vector3Normalize(gameState->editorState.camera.target - gameState->editorState.camera.position));
			if (input.altDown && input.w)
				cameraMovementDir += { 0, 0, 1 };
			if (input.altDown && input.s)
				cameraMovementDir += { 0, 0, -1 };
			if (input.d)
				cameraMovementDir += midMath::Vector3Negate(midMath::Vector3Normalize(midMath::Vector3CrossProduct(gameState->editorState.camera.up, gameState->editorState.camera.target - gameState->editorState.camera.position)));
			if (input.a)
				cameraMovementDir += midMath::Vector3Normalize(midMath::Vector3CrossProduct(gameState->editorState.camera.up, gameState->editorState.camera.target - gameState->editorState.camera.position));

			gameState->editorState.camera.position += cameraMovementDir * cameraSpeed;
			gameState->editorState.camera.target += cameraMovementDir * cameraSpeed;

			gameState->middleState.activeCamera = gameState->editorState.camera;
		}

		if (gameState->middleState.applicationMode == middle::ApplicationMode::GAME_MODE) {
			auto cameraIt = cameraCache->begin<components::CameraComponent>();
			for (int i = 0; i < cameraCache->getSize(); ++i) {
				auto cameraComponent = *cameraIt;
				if (!middle::isValidId(gameState, cameraCache->relevantIdVector[i])) {
					continue;
				}
				auto& shape = middle::getShape(gameState, cameraCache->relevantIdVector[i].index);

				if (cameraComponent->active) {
					auto position = middle::getComponent<components::GlobalTransform>(shape);
					assert(position);
					midPrimitive::Camera camera;
					camera.fovy = cameraComponent->fovy;
					camera.position = position->pos;
					camera.projection = cameraComponent->projection;
					camera.target = { cameraComponent->targetX, cameraComponent->targetY, cameraComponent->targetZ };
					camera.up = { cameraComponent->upX, cameraComponent->upY, cameraComponent->upZ };
					gameState->middleState.activeCamera = camera;
				}
			}
		}
	}
};

static middle::SystemRegistrar<CameraSystem> reg("CameraSystem");
