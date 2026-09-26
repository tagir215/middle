#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "MidComp/BubbleComponent.h"
#include "MidComp/GlobalTransform.h"
#include "MidComp/GlobalRect.h"
#include "MidComp/LocalScale.h"
#include "MidComp/PauseLayoutTag.h"
#include "MidComp/BubblePowerComponent.h"
#include "bubble_utils.h"
#include "MidComp/BubbleSummationComponent.h"
#include "bubble_layout.h"
#include "MidComp/BubbleSwapComponent.h"
#include "MidComp/InViewTag.h"

class BubbleScalingSystem : public middle::MiddleGameplaySystem {
public:
	BubbleScalingSystem() {
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;
	}
	components::CompCache* bubbleCache;
	components::CompCache* powerCache;
	components::CompCache* summationCache;
	components::CompCache* swapCache;
	const float smoothFactor = 0.3f;

	void init(middle::GameState* gameState) override {
		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::InViewTag>();
		bubbleCache->addType<components::BubblePowerComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleSummationComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleSwapComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::PauseLayoutTag>(components::NOTINTERESTED);

		powerCache = middle::newCompCache(gameState, systemName);
		powerCache->addType<components::InViewTag>();
		powerCache->addType<components::BubbleComponent>();
		powerCache->addType<components::BubblePowerComponent>();
		powerCache->addType<components::PauseLayoutTag>(components::NOTINTERESTED);

		summationCache = middle::newCompCache(gameState, systemName);
		summationCache->addType<components::BubbleComponent>();
		summationCache->addType<components::InViewTag>();
		summationCache->addType<components::BubbleSummationComponent>();
		summationCache->addType<components::PauseLayoutTag>(components::NOTINTERESTED);

		swapCache = middle::newCompCache(gameState, systemName);
		swapCache->addType<components::BubbleSwapComponent>();
		swapCache->addType<components::InViewTag>();
		swapCache->addType<components::PauseLayoutTag>(components::NOTINTERESTED);
	}


	void update(middle::GameState* gameState) override {

		for (middle::Id id : bubbleCache->relevantIdVector) {
			bubble::updateBubbleLayoutScale(gameState, id, smoothFactor);
		}

		for (middle::Id id : powerCache->relevantIdVector) {
			bubble::updatePowerLayoutScale(gameState, id, smoothFactor);
		}

		for (middle::Id id : summationCache->relevantIdVector) {
			bubble::updateSummationLayoutScale(gameState, id, smoothFactor);
		}

		for (middle::Id id : swapCache->relevantIdVector) {
			bubble::updateSwapButtonLayoutScale(gameState, id, smoothFactor);
		}
	}
};

static middle::SystemRegistrar<BubbleScalingSystem> reg("BubbleScalingSystem");
