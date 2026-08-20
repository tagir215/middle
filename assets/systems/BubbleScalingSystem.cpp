#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "BubbleComponent.h"
#include "GlobalRadius.h"
#include "GlobalTransform.h"
#include "LocalScale.h"

class BubbleScalingSystem : public middle::MiddleGameplaySystem {
	components::CompCache* bubbleCache;

	const float targetRatio = 0.21803398875;

	void init(middle::GameState* gameState) override {
		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::GlobalRadius>();
		bubbleCache->addType<components::LocalScale>();
		bubbleCache->addType<components::GlobalTransform>();
	}
	void update(middle::GameState* gameState) override {
		auto globalRIt = bubbleCache->begin<components::GlobalRadius>();
		auto localScaleIt = bubbleCache->begin<components::LocalScale>();
		for (middle::Id id : bubbleCache->relevantIdVector) {
			auto globalR = *globalRIt;
			auto localScale = *localScaleIt;
			middle::Id parentId = middle::getParent(gameState, id);
			if (parentId.index == middle::UNASSIGNED) {
				continue;
			}
			auto parentGlobalR = middle::getComp<components::GlobalRadius>(gameState, parentId);
			if (parentGlobalR == 0) {
				continue;
			}
			float currentRatio = globalR->radius / parentGlobalR->radius;
			if (currentRatio == 0) {
				continue;
			}
			float scalar = targetRatio / currentRatio;
			localScale->scale *= scalar;
		}
	}
};

static middle::SystemRegistrar<BubbleScalingSystem> reg("BubbleScalingSystem");
