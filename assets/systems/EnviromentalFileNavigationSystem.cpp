#pragma once
#include "game_state.h"
#include "registrars.h"
#include "middle_shape_utils.h"
#include "MouseIntersectable.h"
#include "SystemReference.h"
#include "ComponentReference.h"

class EnviromentalFileNavigationSystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto system = middle::getComponent<components::SystemReference>(shape);
			auto componentRef = middle::getComponent<components::ComponentReference>(shape);
			if (!system && !componentRef)
				return;

			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			if (intersectable->intersecting) {
				if (gameState->input.navigateToFileClick) {
					if (system) {
						gameState->editorState.nextEditorAction = middle::EditorAction::OPEN_SYSTEM;
						gameState->editorState.nextEditorActionParams.stringValue = system->systemName;
					}
					if (componentRef) {
						gameState->editorState.nextEditorAction = middle::EditorAction::OPEN_COMPONENT;
						gameState->editorState.nextEditorActionParams.stringValue = componentRef->componentName;
					}
				}
			}
			});
	}
};

static middle::SystemRegistrar<EnviromentalFileNavigationSystem> reg("EnviromentalFileNavigationSystem");
