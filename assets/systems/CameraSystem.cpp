#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "CameraComponent.h"
#include "Position.h"
#include "middle_shape_utils.h"

class CameraSystem : public middle::MiddleGameplaySystem {
public:
	CameraSystem() {
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
		systemModeType = middle::SystemModeType::ENGINE;
	}

	middle::Id activeCameraId;

	void init(middle::GameState* gameState) {

	}

	void update(middle::GameState* gameState) override {

		if (gameState->reset || !middle::isIdCurrent(gameState, activeCameraId)) {
			activeCameraId = middle::findFirstShapeWithComp(gameState, middle::getTypeId<components::CameraComponent>());
		}

		if (gameState->applicationMode == middle::ApplicationMode::EDITOR_MODE) {
			// camera controls
			const float maxCameraSpeed = 60;
			float mouseCamRatio = gameState->input.mousePos.y / gameState->screenHeight;
			const float cameraSpeed = mouseCamRatio * mouseCamRatio * mouseCamRatio * maxCameraSpeed;
			Vector3 cameraMovementDir = { 0,0,0 };
			if (!gameState->input.altDown && gameState->input.w)
				cameraMovementDir += Vector3Normalize(gameState->editorState.camera.target - gameState->editorState.camera.position);
			if (!gameState->input.altDown && gameState->input.s)
				cameraMovementDir += Vector3Negate(Vector3Normalize(gameState->editorState.camera.target - gameState->editorState.camera.position));
			if (gameState->input.altDown && gameState->input.w)
				cameraMovementDir += { 0, 0, 1 };
			if (gameState->input.altDown && gameState->input.s)
				cameraMovementDir += { 0, 0, -1 };
			if (gameState->input.d)
				cameraMovementDir += Vector3Negate(Vector3Normalize(Vector3CrossProduct(gameState->editorState.camera.up, gameState->editorState.camera.target - gameState->editorState.camera.position)));
			if (gameState->input.a)
				cameraMovementDir += Vector3Normalize(Vector3CrossProduct(gameState->editorState.camera.up, gameState->editorState.camera.target - gameState->editorState.camera.position));

			gameState->editorState.camera.position += cameraMovementDir * cameraSpeed;
			gameState->editorState.camera.target += cameraMovementDir * cameraSpeed;

			gameState->activeCamera = gameState->editorState.camera;
		}

		if(gameState->applicationMode == middle::ApplicationMode::GAME_MODE){
			auto& shape = middle::getShape(gameState, activeCameraId.index);
			auto cameraComponent = middle::getComponent<components::CameraComponent>(shape);

			if (cameraComponent->active) {
				auto position = middle::getComponent<components::Position>(shape);
				assert(position);
				Camera camera;
				camera.fovy = cameraComponent->fovy;
				camera.position = { position->posX, position->posY, position->posZ };
				camera.projection = cameraComponent->projection;
				camera.target = { cameraComponent->targetX, cameraComponent->targetY, cameraComponent->targetZ };
				camera.up = { cameraComponent->upX, cameraComponent->upY, cameraComponent->upZ };
				gameState->activeCamera = camera;
			}
		}
	}
};

static middle::SystemRegistrar<CameraSystem> reg("CameraSystem");
