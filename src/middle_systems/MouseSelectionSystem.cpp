#pragma once
#include "game_state.h"
#include "registrars.h"
#include "MouseIntersectable.h"
#include "middle_shape_utils.h"
#include "MouseSelectable.h"
#include "Constraint.h"

namespace MouseSelectionSystem {

	class MouseSelectionSystem : public middle::MiddleGameplaySystem {
		void update(middle::GameState* gameState) override {
			for (int i = 0; i < gameState->shapes.size(); ++i) {
				// ghost shapes can't be selected or edited
				if (middle::isGhostShape(i)) {
					return;
				}
				if (!middle::isShapeAlive(gameState, i)) {
					continue;
				}

				auto& shape = middle::getShape(gameState, i);
				auto selectable = middle::getComponent<components::MouseSelectable>(shape);
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				auto constraint = middle::getComponent<components::Constraint>(shape);
				if (selectable == nullptr || intersectable == nullptr)
					continue;

				// when holding down, don't immediatedly toggle once when starting intersect
				if (!intersectable->wasIntersecting && intersectable->intersecting && gameState->input.mouseHeld) {
					selectable->selected = !selectable->selected;
				}

				// toggle selection when clicking
				if (intersectable->intersecting && gameState->input.mouseClicked) {
					selectable->selected = !selectable->selected;
				}

				// grabbing activates selected if there's no selections yet, except can't grab constraints
				if (intersectable->intersecting && gameState->input.grabDown && gameState->selectCount == 0 && constraint == nullptr) {
					selectable->selected = true;
					++gameState->selectCount;
				}

			}
		}
	};

	static middle::SystemRegistrar<MouseSelectionSystem> reg("MouseSelectionSystem");

}
