#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MouseSelectable.h"
#include "Constraint.h"
#include "ConstraintEntity.h"
#include "IntersectingTag.h"

namespace MouseSelectionSystem {

	class MouseSelectionSystem : public middle::MiddleGameplaySystem {
	public:
		MouseSelectionSystem() {
			systemUpdateType = middle::SystemUpdateType::PREFRAME;
			systemModeType = middle::SystemModeType::EDITOR;
		}

		components::CompCache* intersectableCache;

		void init(middle::GameState* gameState) {
			intersectableCache = middle::newCompCache(gameState, systemName);
			intersectableCache->addType<components::MouseSelectable>();
		}
		void update(middle::GameState* gameState) override {

			if (gameState->input.mouseClicked) {
				gameState->editorState.selectChangeCountAfterClick = 0;
			}

			if (gameState->editorState.creationMode == middle::CreationMode::LOOP_MODE) {
				if (gameState->input.mouseClicked || gameState->editorState.selectCount > 1) {
					middle::unselect(gameState);
				}
			}


			auto selectableIt = intersectableCache->begin<components::MouseSelectable>();
			for (int i = 0; i < intersectableCache->getSize(); ++i) {
				auto selectable = *selectableIt;
				auto& shape = middle::getShape(gameState, intersectableCache->relevantIdVector[i].index);
				auto intersecting = middle::getComponent<components::IntersectingTag>(shape);

				// in constraint mode unselect constraints 
				if (gameState->editorState.creationMode == middle::CreationMode::CONSTRAINT_MODE) {
					auto constraint = middle::getComponent<components::Constraint>(shape);
					if (constraint) {
						selectable->selected = false;
						continue;
					}
				}

				// when holding down, don't immediatedly toggle once when starting intersect
				if (intersecting && intersecting->framesIntersected < 2 && intersecting->intersectingTop && gameState->input.mouseHeld) {
					selectable->selected = !selectable->selected;
					++gameState->editorState.selectChangeCountAfterClick;
				}

				// toggle selection when clicking
				if (intersecting && intersecting->intersectingTop && gameState->input.mouseClicked) {
					selectable->selected = !selectable->selected;
					++gameState->editorState.selectChangeCountAfterClick;
				}

				// grabbing activates selected if there's no selections yet, except can't grab constraints
				if (intersecting && intersecting->intersectingTop && gameState->input.grabDown && gameState->editorState.selectCount == 0) {
					auto constraint = middle::getComponent<components::Constraint>(shape);
					if (!constraint) {
						selectable->selected = true;
						++gameState->editorState.selectCount;
						++gameState->editorState.selectChangeCountAfterClick;
					}
				}
			}

			// count update
			gameState->editorState.intersectCount = 0;
			gameState->editorState.selectCount = 0;

			selectableIt = intersectableCache->begin<components::MouseSelectable>();
			for (int i = 0; i < intersectableCache->getSize(); ++i) {
				auto selectable = *selectableIt;
				++gameState->editorState.intersectCount;
				if (selectable->selected) {
					++gameState->editorState.selectCount;
				}
			}

			// unselect
			if (gameState->input.mouseReleased && gameState->editorState.selectChangeCountAfterClick == 0) {
				unselect(gameState);
			}

		}
	};

	static middle::SystemRegistrar<MouseSelectionSystem> reg("MouseSelectionSystem");

}
