#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "LevelReference.h"
#include "MouseClickComponent.h"

class LevelNavigationSystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {
		// early exit
		if (!gameState->input.mouseClicked) {
			return;
		}

		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto levelRef = middle::getComponent<components::LevelReference>(shape);
			if (!levelRef) {
				return true;
			}
			auto click = middle::getComponent<components::MouseClickComponent>(shape);
			if (click) {
				middle::resetScene(gameState);
				middle::loadScene(gameState, "../assets/scenes/", levelRef->levelName, false);
			}

			return true;
			});
	}
};

static middle::SystemRegistrar<LevelNavigationSystem> reg("LevelNavigationSystem");
