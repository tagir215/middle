#pragma once
#include "game_state.h"
#include "registrars.h"
#include "middle_shape_utils.h"
#include "MouseIntersectable.h"
#include "SystemReference.h"

class EnviromentalFileNavigationSystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto system = middle::getComponent<components::SystemReference>(shape);
			if (!system)
				return;

			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			if (intersectable->intersecting) {
				if (gameState->input.navigateToFileClick) {
					gameState->editorState.nextEditorAction = middle::EditorAction::OPEN_SYSTEM;
					gameState->editorState.nextEditorActionParams.stringValue = system->systemName;
				}
			}
			});
	}
};

static middle::SystemRegistrar<EnviromentalFileNavigationSystem> reg("EnviromentalFileNavigationSystem");
