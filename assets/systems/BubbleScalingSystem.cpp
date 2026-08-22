#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "BubbleComponent.h"
#include "GlobalRadius.h"
#include "GlobalTransform.h"
#include "LocalScale.h"
#include "PauseLayoutTag.h"

class BubbleScalingSystem : public middle::MiddleGameplaySystem {
	components::CompCache* bubbleCache;
	const float scaleRatio = 0.958;
	const float oneChildScaleRatio = 0.758;
	const float smoothFactor = 10;

	void init(middle::GameState* gameState) override {
		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::GlobalRadius>();
		bubbleCache->addType<components::LocalScale>();
		bubbleCache->addType<components::GlobalTransform>();
		bubbleCache->addType<components::PauseLayoutTag>(components::NOTINTERESTED);
	}
	void update(middle::GameState* gameState) override {

		auto globalRIt = bubbleCache->begin<components::GlobalRadius>();
		auto localScaleIt = bubbleCache->begin<components::LocalScale>();
		for (middle::Id id : bubbleCache->relevantIdVector) {
			auto globalR = *globalRIt;
			auto localScale = *localScaleIt;
			middle::Id parentId = middle::getParent(gameState, id);
			if (parentId.index == middle::UNASSIGNED 
				|| middle::getComp<components::PauseLayoutTag>(gameState, parentId)) {
				continue;
			}
			std::vector<middle::Id>children;
			middle::getChildren(gameState, parentId, children);
			int childCount = children.size();
			auto parentGlobalR = middle::getComp<components::GlobalRadius>(gameState, parentId);

			float ratio = scaleRatio;
			if (childCount == 1) {
				ratio = oneChildScaleRatio;
			}

			float targetRadius = (parentGlobalR->radius / childCount) * ratio;
		
			float scalar = targetRadius / globalR->radius;
			Vector3 targetScale = localScale->scale * scalar;
			localScale->scale += (targetScale - localScale->scale) * smoothFactor * gameState->frameTime;
		}
	}
};

static middle::SystemRegistrar<BubbleScalingSystem> reg("BubbleScalingSystem");
