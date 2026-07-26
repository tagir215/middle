#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MouseClickComponent.h"
#include "Button.h"
#include "component_utils.h"
#include "bubble_constants.h"
#include "IntersectingTag.h"

class MouseClickSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* clickCache;
	components::CompCache* buttonCache;

	void init(middle::GameState* gameState) {
		clickCache = middle::newCompCache(gameState, systemName);
		clickCache->addType<components::MouseClickComponent>();
		buttonCache = middle::newCompCache(gameState, systemName);
		buttonCache->addType<components::Button>();
		buttonCache->addType<components::IntersectingTag>();
	}
	void update(middle::GameState* gameState) override {

		auto clickIt = clickCache->begin<components::MouseClickComponent>();
		for (int i = 0; i < clickCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, clickCache->relevantIdVector[i].index);
			auto click = *clickIt;
			middle::queueComponentDeletion<components::MouseClickComponent>(gameState, shape.id);
		}

		if (gameState->input.mouseClicked) {
			auto buttonIt = buttonCache->begin<components::Button>();
			auto intersectableIt = buttonCache->begin<components::IntersectingTag>();

			for (int i = 0; i < buttonCache->getSize(); ++i) {
				auto intersectable = *intersectableIt;
				auto& shape = middle::getShape(gameState, buttonCache->relevantIdVector[i].index);
				middle::attachComponent<components::MouseClickComponent>(gameState, shape.id);
				middle::queueSound(gameState, bubbleSounds::CLICK_SOUND);
			}
		}


	}
};

static middle::SystemRegistrar<MouseClickSystem> reg("MouseClickSystem");
