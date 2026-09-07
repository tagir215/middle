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

		const float scalarAcceleration = 5.2f;
		const float scalarDeceleration = 5.2f;
		const float inverseScalarAcceleration = 1.0f / scalarAcceleration;
		const float inverseScalarDeceleration = 1.0f / scalarDeceleration;
		const float maxWorldScaleRate = 4;
		const float inverseMaxWorldScaleRate = 1.0f / maxWorldScaleRate;

		float& worldScalarRate = gameState->bubbleAlgebraState.worldScalarRate;

		// decelerate until stop
		if (!gameState->gameInput.zoomIn && !gameState->gameInput.zoomOut) {
			float scalarScalar;
			const float epsilon = 0.0001f;
			if (worldScalarRate == 1) {
				return;
			}
			else if (worldScalarRate > 1 + epsilon) {
				scalarScalar = std::powf(inverseScalarDeceleration, gameState->frameTime);
				worldScalarRate *= scalarScalar;
			}
			else if (worldScalarRate < 1 - epsilon) {
				scalarScalar = std::powf(scalarDeceleration, gameState->frameTime);
				worldScalarRate *= scalarScalar;
			}
			else {
				worldScalarRate = 1;
			}
		}
		// accelerate zoom in
		else if (gameState->gameInput.zoomIn) {
			float scalarScalar = std::powf(scalarAcceleration, gameState->frameTime);
			worldScalarRate *= scalarScalar;
			if (worldScalarRate > maxWorldScaleRate) {
				worldScalarRate = maxWorldScaleRate;
			}
		}
		// accelerate zoom out
		else {
			float scalarScalar = std::powf(inverseScalarAcceleration, gameState->frameTime);
			gameState->bubbleAlgebraState.worldScalarRate *= scalarScalar;
			if (worldScalarRate < inverseMaxWorldScaleRate) {
				worldScalarRate = inverseMaxWorldScaleRate;
			}
		}

		float scalar = std::powf(gameState->bubbleAlgebraState.worldScalarRate, gameState->frameTime);
		gameState->bubbleAlgebraState.worldScale *= scalar;

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
			middle::assertPos(newPos);

			scale->scale *= scalar;
			middle::setLocalPosition(gameState, id, newPos);
		}
	}
};

static middle::SystemRegistrar<BubbleManualScalingSystem> reg("BubbleManualScalingSystem");
