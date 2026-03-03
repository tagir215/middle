#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MouseIntersectable.h"
#include "MouseClickComponent.h"

class MouseClickSystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {

		if (!gameState->input.mouseClicked) {
			return;
		}

		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			auto click = middle::getComponent<components::MouseClickComponent>(shape);
			if (click) {
				click->clicked = false;
			}
			if (!intersectable || !click) {
				return true;
			}

			if (intersectable->intersectingTop) {
				click->clicked = true;
			}

			return true;
			});
	}
};

static middle::SystemRegistrar<MouseClickSystem> reg("MouseClickSystem");
