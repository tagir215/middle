#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "InsertableBubble.h"
#include "Button.h"
#include "MouseClickComponent.h"
#include "bubble_utils.h"
#include "bubble_actions.h"
#include "editor_actions.h"


class BubbleInsertSystem : public middle::MiddleGameplaySystem {
	components::CompCache* insertableCache;
	components::CompCache* buttonCache;

	void init(middle::GameState* gameState) override {
		insertableCache = middle::newCompCache(gameState);
		insertableCache->addType<components::InsertableBubble>();

		buttonCache = middle::newCompCache(gameState);
		buttonCache->addType<components::Button>();
		buttonCache->addType<components::MouseClickComponent>();
	}
	void update(middle::GameState* gameState) override {
		auto clickedButtonIt = buttonCache->begin<components::Button>();
		for (int i = 0; i < buttonCache->getSize(); ++i) {
			auto button = *clickedButtonIt;
			if (button->function == bubbleButton::NEGATE_HELPERS) {
				std::vector<std::shared_ptr<middle::EditorActionContainer>>actions;
				for (int j = 0; j < insertableCache->getSize(); ++j) {
					middle::Id insertableId = insertableCache->relevantIdVector[j];
					middle::Id replacementId = bubbleActions::createNegatedReplacementShape(gameState, insertableId);
					auto registerAction = std::make_shared<middle::EditorActionRegisterId>(replacementId);
					auto replaceAction = std::make_shared<bubbleActions::Replace>(insertableId, replacementId);
					actions.push_back(registerAction);
					actions.push_back(replaceAction);
				}
				auto multiAction = std::make_shared<middle::MultiAction>(actions);
				middle::queueAction(gameState, multiAction);
				gameState->bubbleAlgebraState.bubbleActions.push_back(multiAction);
			}

			if (button->function == bubbleButton::INVERT_HELPERS) {
				std::vector<std::shared_ptr<middle::EditorActionContainer>>actions;
				for (int j = 0; j < insertableCache->getSize(); ++j) {
					middle::Id insertableId = insertableCache->relevantIdVector[j];
					auto customAction = std::make_shared<CustomActionWithUndo>(
						[insertableId](middle::GameState* gameState) {
							bubble::invert(gameState, insertableId);
						},
						[insertableId](middle::GameState* gameState) {
							bubble::invert(gameState, insertableId);
						});
					actions.push_back(customAction);
				}
				auto multiAction = std::make_shared<middle::MultiAction>(actions);
				middle::queueAction(gameState, multiAction);
				gameState->bubbleAlgebraState.bubbleActions.push_back(multiAction);
			}
		}
	}
};

static middle::SystemRegistrar<BubbleInsertSystem> reg("BubbleInsertSystem");
