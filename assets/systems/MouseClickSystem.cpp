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
	void init(middle::GameState* gameState) {

	}
	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto click = middle::getComponent<components::MouseClickComponent>(shape);
			if (click) {
				middle::queueComponentDeletion<components::MouseClickComponent>(gameState, shape.id);
			}
			return true;
			});

		if (gameState->input.mouseClicked) {
			middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
				auto button = middle::getComponent<components::Button>(shape);
				if (!button) {
					return true;
				}
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				if (intersectable->intersecting) {
					middle::addComponent<components::MouseClickComponent>(shape);
				}

				return true;
				});
		}


	}
};

static middle::SystemRegistrar<MouseClickSystem> reg("MouseClickSystem");
