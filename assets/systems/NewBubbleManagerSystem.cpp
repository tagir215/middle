#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "NewBubbleTag.h"
#include "BubbleSwapComponent.h"
#include "BubbleManipulatable.h"
#include "Button.h"
#include "bubble_utils.h"
#include "RuntimeHiddenTag.h"
#include "NonPhysicalBubbleTag.h"
#include "BubbleGateComponent.h"
#include "BubbleLockedComponent.h"
#include "BubbleTextComponent.h"
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
		for (middle::Id id : newSwapBubbleCache->relevantIdVector) {
			middle::Id activeId, inActiveId;
			bubble::getSwapBubbleActiveInActive(gameState, id, activeId, inActiveId);

			bubble::recursiveDeleteComponent<components::BubbleManipulatable>(gameState, activeId);
			middle::attachComponent<components::Button>(gameState, activeId);

			bubble::recursiveAttachComponent<components::RuntimeHiddenTag>(gameState, inActiveId);
			bubble::recursiveAttachComponent<components::NonPhysicalBubbleTag>(gameState, inActiveId);
			bubble::recursiveDeleteComponent<components::BubbleManipulatable>(gameState, inActiveId);
		}

	}
};

static middle::SystemRegistrar<NewBubbleManagerSystem> reg("NewBubbleManagerSystem");
