#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "MidComp/NewBubbleTag.h"
#include "MidComp/BubbleSwapComponent.h"
#include "MidComp/BubbleManipulatable.h"
#include "MidComp/Button.h"
#include "bubble_utils.h"
#include "MidComp/RuntimeHiddenTag.h"
#include "MidComp/NonPhysicalBubbleTag.h"
#include "MidComp/BubbleGateComponent.h"
#include "MidComp/BubbleLockedComponent.h"
#include "MidComp/BubbleTextComponent.h"
#include "bubble_utils.h"


class NewBubbleManagerSystem : public middle::MiddleGameplaySystem {
	components::CompCache* newBubbleCache;
	components::CompCache* newSwapBubbleCache;

	void init(middle::GameState* gameState) override {
		systemUpdateType = middle::SystemUpdateType::POSTFRAME;
		systemModeType = middle::SystemModeType::ENGINE;
		// update last
		updatePriority = 20;
		newBubbleCache = middle::newCompCache(gameState, systemName);
		newBubbleCache->addType<components::NewBubbleTag>();

		newSwapBubbleCache = middle::newCompCache(gameState, systemName);
		newSwapBubbleCache->addType<components::NewBubbleTag>();
		newSwapBubbleCache->addType<components::BubbleSwapComponent>();


	}
	void update(middle::GameState* gameState) override {

		for (middle::Id id : newBubbleCache->relevantIdVector) {
			middle::queueComponentDeletion<components::NewBubbleTag>(gameState, id);
		}

		// initialize swap bubbles
		auto swapCompIt = newSwapBubbleCache->begin<components::BubbleSwapComponent>();
		for (middle::Id id : newSwapBubbleCache->relevantIdVector) {
			auto swapComp = *swapCompIt;
			middle::Id activeId, inActiveId;
			bubble::getSwapBubbleActiveInActive(gameState, id, activeId, inActiveId);

			bubble::recursiveDeleteComponent<components::BubbleManipulatable>(gameState, activeId);
			if (swapComp->status == components::SwapComponentStatus::SWAP_ENABLED) {
				middle::attachComponent<components::Button>(gameState, activeId);
			}

			bubble::recursiveAttachComponent<components::RuntimeHiddenTag>(gameState, inActiveId);
			bubble::recursiveAttachComponent<components::NonPhysicalBubbleTag>(gameState, inActiveId);
			bubble::recursiveDeleteComponent<components::BubbleManipulatable>(gameState, inActiveId);
		}

	}
};

static middle::SystemRegistrar<NewBubbleManagerSystem> reg("NewBubbleManagerSystem");
