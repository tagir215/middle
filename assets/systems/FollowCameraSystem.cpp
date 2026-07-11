#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "UiNode.h"

class FollowCameraSystem : public middle::MiddleGameplaySystem {
public:
	FollowCameraSystem(){
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_MIDFRAME;
	}

	components::CompCache* uiCache;

	void init(middle::GameState* gameState) {
		uiCache = middle::newCompCache(gameState, systemName);
		uiCache->addType<components::UiNode>();
	}
	void update(middle::GameState* gameState) override {

		auto uiIt = uiCache->begin<components::UiNode>();
		for (int i = 0; i < uiCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, uiCache->relevantIdVector[i].index);
			Vector3 cameraPos = gameState->activeCamera.position;
			Vector3 pos = middle::getShapePosition(gameState, shape.id.index);
			middle::moveShape(gameState, shape.id.index, cameraPos - pos);
		}
	}
};

static middle::SystemRegistrar<FollowCameraSystem> reg("FollowCameraSystem");
