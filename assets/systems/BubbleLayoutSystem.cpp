#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "BubbleComponent.h"
#include "Rectangle.h"
#include "imgui.h"
#include "PauseLayoutTag.h"
#include "BubblePowerComponent.h"
#include "bubble_utils.h"
#include "BubbleSummationComponent.h"
#include "bubble_layout.h"
#include "BubbleswapComponent.h"

class BubbleLayoutSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* bubbleCache;
	components::CompCache* powerCache;
	components::CompCache* summationCache;
	components::CompCache* pausedBubblesCache;
	components::CompCache* swapCache;

	void init(middle::GameState* gameState) override {
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;

		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::Rectangle>();
		bubbleCache->addType<components::PauseLayoutTag>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubblePowerComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleSummationComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleSwapComponent>(components::NOTINTERESTED);

		powerCache = middle::newCompCache(gameState, systemName);
		powerCache->addType<components::BubbleComponent>();
		powerCache->addType<components::Rectangle>();
		powerCache->addType<components::BubblePowerComponent>();
		powerCache->addType<components::PauseLayoutTag>(components::NOTINTERESTED);

		summationCache = middle::newCompCache(gameState, systemName);
		summationCache->addType<components::BubbleComponent>();
		summationCache->addType<components::Rectangle>();
		summationCache->addType<components::BubbleSummationComponent>();
		summationCache->addType<components::PauseLayoutTag>(components::NOTINTERESTED);

		swapCache = middle::newCompCache(gameState, systemName);
		swapCache->addType<components::BubbleSwapComponent>();
		swapCache->addType<components::Rectangle>();
		swapCache->addType<components::PauseLayoutTag>(components::NOTINTERESTED);

		pausedBubblesCache = middle::newCompCache(gameState, systemName);
		pausedBubblesCache->addType<components::PauseLayoutTag>();
	}



	void update(middle::GameState* gameState) override {

		const float ratioPerSecond = 0.8f;
		float moveRatio = std::powf(ratioPerSecond, gameState->frameTime);

		auto pauseTagIt = pausedBubblesCache->begin<components::PauseLayoutTag>();
		for (middle::Id id : pausedBubblesCache->relevantIdVector) {
			auto pause = *pauseTagIt;
			if (pause->timeLeft <= 0) {
				middle::queueComponentDeletion<components::PauseLayoutTag>(gameState, id);
			}
			pause->timeLeft -= gameState->frameTime;
		}

		// bubbles
		auto rectIt = bubbleCache->begin<components::Rectangle>();
		for (middle::Id id : bubbleCache->relevantIdVector) {
			if (id.index == 131) {
				int a = 0;
			}

			auto rect = *rectIt;
			bubble::updateBubbleLayout(gameState, id, rect->width, moveRatio);
		}


		// powers
		auto powerRectIt = powerCache->begin<components::Rectangle>();
		for (middle::Id id : powerCache->relevantIdVector) {
			auto rect = *powerRectIt;
			bubble::updatePowerLayout(gameState, id, rect->width, moveRatio);
		}

		// summations
		auto summationRectIt = summationCache->begin<components::Rectangle>();
		for (middle::Id id : summationCache->relevantIdVector) {
			auto rect = *summationRectIt;
			bubble::updateSummationLayout(gameState, id, rect->width, moveRatio);
		}

		// swaps
		auto swapRectIt = swapCache->begin<components::Rectangle>();
		for (middle::Id id : swapCache->relevantIdVector) {
			auto rect = *swapRectIt;
			bubble::updateSwapButtonLayout(gameState, id, rect->width);
		}
	}
};

static middle::SystemRegistrar<BubbleLayoutSystem> reg("BubbleLayoutSystem");
