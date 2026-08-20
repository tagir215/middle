#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "TopDogBubbleTag.h"
#include "LocalScale.h"
#include "IntersectingTag.h"
#include "GlobalTransform.h"

class BubbleManualScalingSystem : public middle::MiddleGameplaySystem {
	components::CompCache* topDogCache;

	void init(middle::GameState* gameState) override {
		topDogCache = middle::newCompCache(gameState, systemName);
		topDogCache->addType<components::TopDogBubbleTag>();
		topDogCache->addType<components::LocalScale>();
		topDogCache->addType<components::GlobalTransform>();
	}
	void update(middle::GameState* gameState) override {
		if (gameState->gameInput.mouseWheelMove == 0) {
			return;
		}

		const float scaleSpeed = 0.3f * -gameState->gameInput.mouseWheelMove;

		float scalar = 1;
		if (gameState->gameInput.zoomIn) {
			scalar = scalar + scaleSpeed;
		}
		else {
			scalar = scalar - scaleSpeed;
		}

		Vector3 mousePos = gameState->input.mouseXZ_PlanePos;
		Matrix transM = MatrixTranslate(-mousePos.x, -mousePos.y, -mousePos.z);
		Matrix scaleM = MatrixScale(scalar, 0, scalar);
		Matrix trans2M = MatrixTranslate(mousePos.x, mousePos.y, mousePos.z);
		Matrix m = MatrixMultiply(transM, scaleM);
		m = MatrixMultiply(m, trans2M);

		auto scaleIt = topDogCache->begin<components::LocalScale>();
		auto transformIt = topDogCache->begin<components::GlobalTransform>();
		for (middle::Id id : topDogCache->relevantIdVector) {
			auto scale = *scaleIt;
			auto transform = *transformIt;

			Vector3 newPos = Vector3Transform(transform->pos, m);

			scale->scale *= scalar;
			middle::moveShape(gameState, id.index, newPos - transform->pos);
		}
	}
};

static middle::SystemRegistrar<BubbleManualScalingSystem> reg("BubbleManualScalingSystem");
