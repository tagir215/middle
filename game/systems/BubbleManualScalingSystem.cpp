#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "TopDogInViewTag.h"
#include "LocalScale.h"
#include "IntersectingTag.h"
#include "LocalPosition.h"
#include "TopDogBubbleTag.h"
#include "NonPhysicalBubbleTag.h"
#include "middle_debug_utils.h"

class BubbleManualScalingSystem : public middle::MiddleGameplaySystem {
	components::CompCache* topDogCache;

	void init(middle::GameState* gameState) override {
		topDogCache = middle::newCompCache(gameState, systemName);
		topDogCache->addType<components::TopDogInViewTag>();
		topDogCache->addType<components::LocalScale>();
		topDogCache->addType<components::LocalPosition>();
		topDogCache->addType<components::NonPhysicalBubbleTag>(components::NOTINTERESTED);
	}

	float prevMouseWheelMove = 0;
	const float mouseWheelMoveMinTimeSeconds = 0.05f;
	std::stack<float>wheelMoveTimeStack;

	void update(middle::GameState* gameState) override {

		const float scalarAcceleration = 10000000000;
		const float scalarDeceleration = 1000000000000;
		const float inverseScalarAcceleration = 1.0f / scalarAcceleration;
		const float inverseScalarDeceleration = 1.0f / scalarDeceleration;
		const float maxWorldScaleRate = 6000;
		const float inverseMaxWorldScaleRate = 1.0f / maxWorldScaleRate;

		float& worldScalarRate = gameState->bubbleAlgebraState.worldScalarRate;
		float mouseWheelMove = gameState->gameInput.mouseWheelMove;


		middle::drawImGuiFloat(gameState, "mousewheelmove", mouseWheelMove);


		float acceleration = scalarAcceleration;
		float inverseAcceleration = inverseScalarAcceleration;

		if (mouseWheelMove == 0) {

			if (wheelMoveTimeStack.size() > 0) {
				if (wheelMoveTimeStack.top() < mouseWheelMoveMinTimeSeconds) {
					mouseWheelMove = prevMouseWheelMove;
					wheelMoveTimeStack.top() += gameState->frameTime;
				}
				else {
					wheelMoveTimeStack.pop();
				}
			}
			else {
				prevMouseWheelMove = 0;
			}
		}
		else {
			prevMouseWheelMove = mouseWheelMove;
			wheelMoveTimeStack.push(0);
		}

		// accelerate zoom in
		if (gameState->gameInput.zoomIn || mouseWheelMove > 0) {
			float scalarScalar = std::powf(acceleration, gameState->frameTime);
			worldScalarRate *= scalarScalar;
			if (worldScalarRate > maxWorldScaleRate) {
				worldScalarRate = maxWorldScaleRate;
			}
		}
		// accelerate zoom out
		else if (gameState->gameInput.zoomOut || mouseWheelMove < 0) {
			float scalarScalar = std::powf(inverseAcceleration, gameState->frameTime);
			worldScalarRate *= scalarScalar;
			if (worldScalarRate < inverseMaxWorldScaleRate) {
				worldScalarRate = inverseMaxWorldScaleRate;
			}
		}
		// decelerate until stop
		else {
			float scalarScalar;
			const float epsilon = 0.0001f;
			if (worldScalarRate == 1) {
				return;
			}
			else if (worldScalarRate > 1 + epsilon) {
				scalarScalar = std::powf(inverseScalarDeceleration, gameState->frameTime);
				float newRate = worldScalarRate * scalarScalar;
				if (worldScalarRate < 1 && newRate > 1 || worldScalarRate > 1 && newRate < 1) {
					worldScalarRate = 1;
				}
				else {
					worldScalarRate = newRate;
				}
			}
			else if (worldScalarRate < 1 - epsilon) {
				scalarScalar = std::powf(scalarDeceleration, gameState->frameTime);
				float newRate = worldScalarRate * scalarScalar;
				if (worldScalarRate < 1 && newRate > 1 || worldScalarRate > 1 && newRate < 1) {
					worldScalarRate = 1;
				}
				else {
					worldScalarRate = newRate;
				}
			}
			else {
				worldScalarRate = 1;
			}
		}

		float scalar = std::powf(gameState->bubbleAlgebraState.worldScalarRate, gameState->frameTime);
		gameState->bubbleAlgebraState.worldScale *= scalar;

		midMath::Vector3 mousePos = gameState->input.mouseXZ_PlanePos;
		midMath::Matrix transM = midMath::MatrixTranslate(-mousePos.x, -mousePos.y, -mousePos.z);
		midMath::Matrix scaleM = midMath::MatrixScale(scalar, 0, scalar);
		midMath::Matrix trans2M = midMath::MatrixTranslate(mousePos.x, mousePos.y, mousePos.z);
		midMath::Matrix m = MatrixMultiply(transM, scaleM);
		m = midMath::MatrixMultiply(m, trans2M);

		auto scaleIt = topDogCache->begin<components::LocalScale>();
		auto localPosIt = topDogCache->begin<components::LocalPosition>();
		for (middle::Id id : topDogCache->relevantIdVector) {
			auto scale = *scaleIt;
			auto localPos = *localPosIt;

			midMath::Vector3 newPos = Vector3Transform(localPos->pos, m);
			middle::assertPos(newPos);

			scale->scale *= scalar;
			middle::setLocalPosition(gameState, id, newPos);
		}
	}
};

static middle::SystemRegistrar<BubbleManualScalingSystem> reg("BubbleManualScalingSystem");
