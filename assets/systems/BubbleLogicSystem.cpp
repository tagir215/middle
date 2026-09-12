#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "BubbleLogicComponent.h"
#include "ModifiedBubbleTag.h"
#include "bubble_utils.h"

class BubbleLogicSystem : public middle::MiddleGameplaySystem {
	components::CompCache* modifiedCache;

	void init(middle::GameState* gameState) override {
		modifiedCache = middle::newCompCache(gameState, systemName);
		modifiedCache->addType<components::ModifiedBubbleTag>();
	}
	void update(middle::GameState* gameState) override {

		middle::Id logicBubbleToUpdate;
		for (middle::Id id : modifiedCache->relevantIdVector) {
			middle::queueComponentDeletion<components::ModifiedBubbleTag>(gameState, id);
			middle::Id logicId = bubble::findIdWithCompFromShapeOrItsParents<components::BubbleLogicComponent>(gameState, id);
			logicBubbleToUpdate = logicId;
			break;
		}

		// check similarity 
		if (middle::isValidId(gameState, logicBubbleToUpdate)) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, logicBubbleToUpdate, children);
			if (children.size() < 2) {
				return;
			}
			middle::Id left, right;
			bubble::getLogicLeftAndRight(gameState, logicBubbleToUpdate, left, right);
			if (bubble::matchingBubbles(gameState, left, right)) {
				int a = 0;
			}
		}
	}
};

static middle::SystemRegistrar<BubbleLogicSystem> reg("BubbleLogicSystem");
