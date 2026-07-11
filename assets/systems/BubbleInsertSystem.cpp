#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "InsertableBubble.h"
#include "Button.h"
#include "MouseClickComponent.h"
#include "bubble_utils.h"
#include "bubble_actions.h"
#include "editor_actions.h"


class BubbleInsertSystem : public middle::MiddleGameplaySystem {
	components::CompCache* insertableCache;
	components::CompCache* buttonCache;

	void init(middle::GameState* gameState) override {
		insertableCache = middle::newCompCache(gameState);
		insertableCache->addType<components::InsertableBubble>();

		buttonCache = middle::newCompCache(gameState);
		buttonCache->addType<components::Button>();
		buttonCache->addType<components::MouseClickComponent>();
	}
	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar<BubbleInsertSystem> reg("BubbleInsertSystem");
