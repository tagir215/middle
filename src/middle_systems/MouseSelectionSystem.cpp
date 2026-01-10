#pragma once
#include "game_state.h"
#include "registrars.h"
#include "MouseIntersectable.h"
#include "middle_shape_utils.h"

namespace MouseSelectionSystem {

	class MouseSelectionSystem : public middle::MiddleGameplaySystem {
		void update(middle::GameState* gameState) override {
			for (int i = 0; i < gameState->shapes.size(); ++i) {
				// ghost shapes can't be selected or edited
				if (middle::isGhostShape(i)) {
					return;
				}

				// when holding down, don't immediatedly toggle once when starting intersect
				//if (!wasIntersecting && instance.mouseIntersects && gameState->input.mouseHeld) {
				//	instance.selected = !instance.selected;
				//}

				//// toggle selection when clicking
				//if (instance.mouseIntersects && gameState->input.mouseClicked) {
				//	instance.selected = !instance.selected;
				//}

				//// grabbing activates selected if there's no selections yet, except can't grab constraints
				//if (instance.mouseIntersects && gameState->input.grabDown && gameState->selectCount == 0 && constraint == nullptr) {
				//	instance.selected = true;
				//	++gameState->selectCount;
				//}

				//auto& vec = middle::getComponentArray<components::MouseIntersectable>();
				//for (auto& component : vec) {

				//}

			}
		}
	};

	static middle::SystemRegistrar<MouseSelectionSystem> reg("MouseSelectionSystem");

}
