#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "BubbleComponent.h"
#include "GlobalTransform.h"
#include "GlobalRect.h"
#include "LocalScale.h"
#include "PauseLayoutTag.h"
#include "BubblePowerComponent.h"
#include "bubble_utils.h"
#include "BubbleSummationComponent.h"
#include "bubble_layout.h"

class BubbleScalingSystem : public middle::MiddleGameplaySystem {
public:
	BubbleScalingSystem() {
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;
	}
	components::CompCache* bubbleCache;
	components::CompCache* powerCache;
	components::CompCache* summationCache;
	const float smoothFactor = 0.3f;

	void init(middle::GameState* gameState) override {
		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::GlobalRect>();
		bubbleCache->addType<components::LocalScale>();
		bubbleCache->addType<components::GlobalTransform>();
		bubbleCache->addType<components::BubblePowerComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleSummationComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::PauseLayoutTag>(components::NOTINTERESTED);

		powerCache = middle::newCompCache(gameState, systemName);
		powerCache->addType<components::BubbleComponent>();
		powerCache->addType<components::GlobalRect>();
		powerCache->addType<components::GlobalTransform>();
		powerCache->addType<components::BubblePowerComponent>();
		powerCache->addType<components::PauseLayoutTag>(components::NOTINTERESTED);

		summationCache = middle::newCompCache(gameState, systemName);
		summationCache->addType<components::BubbleComponent>();
		summationCache->addType<components::GlobalRect>();
		summationCache->addType<components::GlobalTransform>();
		summationCache->addType<components::BubbleSummationComponent>();
		summationCache->addType<components::PauseLayoutTag>(components::NOTINTERESTED);
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
	}
};

static middle::SystemRegistrar<BubbleScalingSystem> reg("BubbleScalingSystem");
