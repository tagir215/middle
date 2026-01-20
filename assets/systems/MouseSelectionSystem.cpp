#pragma once
#include "game_state.h"
#include "registrars.h"
#include "MouseIntersectable.h"
#include "middle_shape_utils.h"
#include "MouseSelectable.h"
#include "Constraint.h"
#include "ConstraintEntity.h"

namespace MouseSelectionSystem {

	class MouseSelectionSystem : public middle::MiddleGameplaySystem {
		void update(middle::GameState* gameState) override {

			if (gameState->input.mouseClicked) {
				gameState->editorState.selectChangeCountAfterClick = 0;
			}

			if (gameState->editorState.creationMode == middle::CreationMode::LOOP_MODE) {
				if (gameState->input.mouseClicked || gameState->editorState.selectCount > 1) {
					middle::unselect(gameState);
				}
			}

			for (int i = 0; i < gameState->shapes.size(); ++i) {
				// ghost shapes can't be selected or edited
				if (middle::isGhostShape(i)) {
					break;
				}
				if (!middle::isShapeAlive(gameState, i)) {
					continue;
				}

				auto& shape = middle::getShape(gameState, i);
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				auto selectable = middle::getComponent<components::MouseSelectable>(shape);
				if (selectable == nullptr || intersectable == nullptr)
					continue;

				auto constraint = middle::getComponent<components::Constraint>(shape);

				// in constraint mode unselect constraints 
				if (gameState->editorState.creationMode == middle::CreationMode::CONSTRAINT_MODE && constraint) {
					selectable->selected = false;
					continue;
				}

				// when holding down, don't immediatedly toggle once when starting intersect
				if (!intersectable->wasIntersecting && intersectable->intersecting && gameState->input.mouseHeld) {
					selectable->selected = !selectable->selected;
					++gameState->editorState.selectChangeCountAfterClick;
				}

				// toggle selection when clicking
				if (intersectable->intersecting && gameState->input.mouseClicked) {
					selectable->selected = !selectable->selected;
					++gameState->editorState.selectChangeCountAfterClick;
				}

				// grabbing activates selected if there's no selections yet, except can't grab constraints
				if (intersectable->intersecting && gameState->input.grabDown && gameState->editorState.selectCount == 0 && constraint == nullptr) {
					selectable->selected = true;
					++gameState->editorState.selectCount;
					++gameState->editorState.selectChangeCountAfterClick;
				}

			}

			// count update
			gameState->editorState.intersectCount = 0;
			gameState->editorState.selectCount = 0;
			for (int i = 0; i < gameState->shapes.size(); ++i) {
				if (isShapeSelected(gameState, i))
					++gameState->editorState.selectCount;
				if (isMouseIntersectingShape(gameState, i))
					++gameState->editorState.intersectCount;
			}

			// unselect
			if (gameState->input.mouseReleased && gameState->editorState.selectChangeCountAfterClick == 0) {
				unselect(gameState);
			}

		}
	};

	static middle::SystemRegistrar<MouseSelectionSystem> reg("MouseSelectionSystem");

}
