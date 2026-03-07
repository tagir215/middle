#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MouseClickComponent.h"
#include "Button.h"
#include "BubbleAlgebraProblem.h"
#include "bubble_actions.h"

class AlgebraProblemSystem : public middle::MiddleGameplaySystem {
public:
	void init(middle::GameState* gameState) {

	}

	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto click = middle::getComponent<components::MouseClickComponent>(shape);
			if (!click) {
				return true;
			}
			auto button = middle::getComponent<components::Button>(shape);
			if (button->function == bubbleButton::DONE) {
				std::vector<middle::Id> formulas;
				middle::findShapesWithComp(gameState, formulas, middle::getTypeId<components::BubbleAlgebraProblem>());
				assert(formulas.size() == 2);
				middle::Id& formulaA = formulas[0];
				middle::Id& formulaB = formulas[1];
				bool matching = bubbleActions::matchingBubbles(gameState, formulaA, formulaB);

				if (matching) {
					middle::loadShape(gameState, "../assets/shapes/", "ScoreScreen", true);
				}
			}
			if (button->function == bubbleButton::UNDO) {
				if (gameState->bubbleAlgebraState.bubbleActions.size() > 0) {
					gameState->bubbleAlgebraState.bubbleActions.back()->undo(gameState);
					gameState->bubbleAlgebraState.bubbleActions.pop_back();
				}
			}
			return true;
			});
	}
};

static middle::SystemRegistrar<AlgebraProblemSystem> reg("AlgebraProblemSystem");
