#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MouseClickComponent.h"
#include "Button.h"
#include "BubbleAlgebraProblem.h"
#include "bubble_actions.h"
#include "bubble_utils.h"
#include "BubbleAlgebraProblemContainer.h"
#include "Rectangle.h"

class AlgebraProblemSystem : public middle::MiddleGameplaySystem {
public:


	components::CompCache* cache = nullptr;
	components::CompCache* problemCache = nullptr;
	components::CompCache* containerCache = nullptr;

	void init(middle::GameState* gameState) {
		cache = middle::newCompCache(gameState);
		cache->addType<components::MouseClickComponent>();
		cache->addType<components::Button>();
		containerCache = middle::newCompCache(gameState);
		containerCache->addType<components::BubbleAlgebraProblemContainer>();
		containerCache->addType<components::Position>();
		containerCache->addType<components::Rectangle>();
		problemCache = middle::newCompCache(gameState);
		problemCache->addType<components::BubbleAlgebraProblem>();
		problemCache->addType<components::Position>();
	}

	void update(middle::GameState* gameState) override {

		auto buttonIt = cache->begin<components::Button>();
		int size = cache->getSize();

		for (int i = 0; i < size; ++i) {
			auto button = *buttonIt;
			if (button->function == bubbleButton::DONE) {
				std::vector<middle::Id> formulas;
				assert(problemCache->getSize() == 2);
				middle::Id& formulaA = problemCache->relevantIdVector[0];
				middle::Id& formulaB = problemCache->relevantIdVector[1];
				bool matching = bubble::matchingBubbles(gameState, formulaA, formulaB);

				if (matching) {
					middle::loadShape(gameState, "../assets/shapes/", "ScoreScreen", true);
					gameState->bubbleAlgebraState.justCompletedLevel = true;
					gameState->bubbleAlgebraState.completedLevelName = gameState->activeSceneName;
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

		if(containerCache->getSize() == 1){
			auto containerPosIt = containerCache->begin<components::Position>();
			auto containerRectIt = containerCache->begin<components::Rectangle>();
			auto containerPos = *containerPosIt;
			auto containerRect = *containerRectIt;

			const float minMargin = 40;

			components::Position* problemPos;
			int problemIndex = -1;
			auto posIt = problemCache->begin<components::Position>();
			for (int i = 0; i < problemCache->getSize(); ++i) {
				auto pos = *posIt;
				if (pos->posX > containerPos->posX - minMargin) {
					problemPos = pos;
					problemIndex = i;
				}
			}

			auto& problemShape = middle::getShape(gameState, problemCache->relevantIdVector[problemIndex].index);

			const float margin = 10;

			// resize to match child
			float left, right, bottom, top;
			bubble::bubbleRectBoundingBox(gameState, problemShape.id, &left, &right, &bottom, &top);
			containerRect->width = right - left + margin;
			containerRect->height = top - bottom + margin;

			// move to center of problem
			containerPos->posX = left + containerRect->width * 0.5f - margin * 0.5f;
			containerPos->posZ = bottom + containerRect->height * 0.5f - margin * 0.5f;
		}
	}
};

static middle::SystemRegistrar<AlgebraProblemSystem> reg("AlgebraProblemSystem");
