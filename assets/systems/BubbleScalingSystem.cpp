#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "BubbleComponent.h"
#include "GlobalRadius.h"
#include "GlobalTransform.h"
#include "LocalScale.h"
#include "PauseLayoutTag.h"
#include "BubblePowerComponent.h"
#include "bubble_utils.h"

class BubbleScalingSystem : public middle::MiddleGameplaySystem {
	components::CompCache* bubbleCache;
	components::CompCache* powerCache;
	const float scaleRatio = 0.958;
	const float oneChildScaleRatio = 0.758;
	const float smoothFactor = 10;
	const float powerBaseRatio = 1.28f;
	const float powerExponentRatio = 0.25f;

	void init(middle::GameState* gameState) override {
		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::GlobalRadius>();
		bubbleCache->addType<components::LocalScale>();
		bubbleCache->addType<components::GlobalTransform>();
		bubbleCache->addType<components::BubblePowerComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::PauseLayoutTag>(components::NOTINTERESTED);

		powerCache = middle::newCompCache(gameState, systemName);
		powerCache->addType<components::BubbleComponent>();
		powerCache->addType<components::GlobalRadius>();
		powerCache->addType<components::GlobalTransform>();
		powerCache->addType<components::BubblePowerComponent>();
		powerCache->addType<components::PauseLayoutTag>(components::NOTINTERESTED);
	}

	void updateScale(middle::GameState* gameState, middle::Id id, float targetRadius) {
		auto childGlobalR = middle::getComp<components::GlobalRadius>(gameState, id);
		auto childScale = middle::getComp<components::LocalScale>(gameState, id);
		float scalar = targetRadius / childGlobalR->radius;
		Vector3 targetScale = childScale->scale * scalar;
		childScale->scale += (targetScale - childScale->scale) * smoothFactor * gameState->frameTime;
	}

	void update(middle::GameState* gameState) override {

		auto globalRIt = bubbleCache->begin<components::GlobalRadius>();
		for (middle::Id id : bubbleCache->relevantIdVector) {
			auto globalR = *globalRIt;

			std::vector<middle::Id>children;
			middle::getChildren(gameState, id, children);
			int childCount = children.size();

			if (childCount == 0) {
				continue;
			}

			float ratio = scaleRatio;
			if (childCount == 1) {
				ratio = oneChildScaleRatio;
			}

			const float targetRadius = (globalR->radius / childCount) * ratio;

			for (middle::Id childId : children) {
				updateScale(gameState, childId, targetRadius);
			}
		}

		auto powerGlobalRIt = powerCache->begin<components::GlobalRadius>();
		for (middle::Id id : powerCache->relevantIdVector) {
			auto globalR = *powerGlobalRIt;

			middle::Id baseId, exponentId;
			bubble::getPowerBaseAndExponent(gameState, id, baseId, exponentId);

			float halfRadius = globalR->radius * 0.5f;

			const float targetRadiusBase = halfRadius * powerBaseRatio;
			const float targetRadiusExponent = halfRadius * powerExponentRatio;

			updateScale(gameState, baseId, targetRadiusBase);
			updateScale(gameState, exponentId, targetRadiusExponent);
		}

	}
};

static middle::SystemRegistrar<BubbleScalingSystem> reg("BubbleScalingSystem");
