#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "UiNode.h"

class FollowCameraSystem : public middle::MiddleGameplaySystem {
public:
	FollowCameraSystem(){
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;
	}
	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto uinode = middle::getComponent<components::UiNode>(shape);
			if (!uinode) {
				return true;
			}

			Vector3 cameraPos = gameState->activeCamera.position;

			Vector3 pos = middle::getShapePosition(gameState, i);
			middle::moveShape(gameState, i, cameraPos - pos);

			return true;
			});
	}
};

static middle::SystemRegistrar<FollowCameraSystem> reg("FollowCameraSystem");
