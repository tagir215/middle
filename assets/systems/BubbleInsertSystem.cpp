#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "InsertableBubble.h"

class BubbleInsertSystem : public middle::MiddleGameplaySystem {
	components::CompCache* insertableCache;

	void init(middle::GameState* gameState) override {
		insertableCache = middle::newCompCache(gameState);
		insertableCache->addType<components::InsertableBubble>();
	}
	void update(middle::GameState* gameState) override {
		auto insertableIt = insertableCache->begin<components::InsertableBubble>();
		for (int i = 0; i < insertableCache->getSize(); ++i) {

		}
	}
};

static middle::SystemRegistrar<BubbleInsertSystem> reg("BubbleInsertSystem");
