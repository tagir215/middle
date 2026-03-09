#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MouseIntersectable.h"
#include "MouseClickComponent.h"
#include "Button.h"
#include "component_utils.h"

class MouseClickSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* clickCache;
	components::CompCache* buttonCache;

	void init(middle::GameState* gameState) {
		clickCache = middle::newCompCache(gameState);
		clickCache->addType<components::MouseClickComponent>();
		buttonCache = middle::newCompCache(gameState);
		buttonCache->addType<components::Button>();
		buttonCache->addType<components::MouseIntersectable>();
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
			auto intersectableIt = buttonCache->begin<components::MouseIntersectable>();

			for (int i = 0; i < buttonCache->getSize(); ++i) {
				auto intersectable = *intersectableIt;
				auto& shape = middle::getShape(gameState, buttonCache->relevantIdVector[i].index);
				if (intersectable->intersecting) {
					middle::attachComponent<components::MouseClickComponent>(gameState, shape.id);
				}
			}
		}


	}
};

static middle::SystemRegistrar<MouseClickSystem> reg("MouseClickSystem");
