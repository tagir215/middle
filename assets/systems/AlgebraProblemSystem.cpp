#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MouseClickComponent.h"
#include "Button.h"
#include "BubbleAlgebraProblem.h"
#include "bubble_actions.h"
#include "bubble_utils.h"

class AlgebraProblemSystem : public middle::MiddleGameplaySystem {
public:


	components::CompCache* cache = nullptr;

	void init(middle::GameState* gameState) {
		cache = middle::newCompCache(gameState);
		cache->addType<components::MouseClickComponent>();
		cache->addType<components::Button>();
	}

	void update(middle::GameState* gameState) override {

		auto buttonIt = cache->begin<components::Button>();
		int size = cache->getSize();

		for (int i = 0; i < size; ++i) {
			auto button = *buttonIt;
			if (button->function == bubbleButton::DONE) {
				std::vector<middle::Id> formulas;
				middle::findShapesWithComp(gameState, formulas, middle::getTypeId<components::BubbleAlgebraProblem>());
				assert(formulas.size() == 2);
				middle::Id& formulaA = formulas[0];
				middle::Id& formulaB = formulas[1];
				bool matching = bubble::matchingBubbles(gameState, formulaA, formulaB);

				if (matching) {
					middle::loadShape(gameState, "../assets/shapes/", "ScoreScreen", true);
				}
			}
			if (button->function == bubbleButton::UNDO) {
				if (gameState->bubbleAlgebraState.bubbleActions.size() > 0) {
					middle::queueAction(gameState, std::make_shared<middle::CustomAction>([](middle::GameState* gameState) {
						gameState->bubbleAlgebraState.bubbleActions.back()->undo(gameState);
						gameState->bubbleAlgebraState.bubbleActions.pop_back();
						}));
				}
			}
		}
	}
};

static middle::SystemRegistrar<AlgebraProblemSystem> reg("AlgebraProblemSystem");
