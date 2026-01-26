#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MouseIntersectable.h"
#include "SystemReference.h"
#include "ComponentReference.h"
#include "MouseSelectable.h"
#include "ComponentRefParent.h"
#include "LoopTag.h"
#include "Position.h"
#include "Text.h"
#include "editor_actions.h"

class EnviromentalFileNavigationSystem : public middle::MiddleGameplaySystem {
public:
	EnviromentalFileNavigationSystem() {
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
		systemModeType = middle::SystemModeType::EDITOR;
	}

	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto system = middle::getComponent<components::SystemReference>(shape);
			auto componentRef = middle::getComponent<components::ComponentReference>(shape);

			if (system || componentRef) {
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				if (intersectable->intersecting) {
					if (gameState->input.navigateToFileClick) {
						if (system) {
							gameState->editorState.editorActions.push_back(
								std::make_unique<middle::EditorActionOpenSystem>(system->systemName)
							);
						}
						if (componentRef) {
							gameState->editorState.editorActions.push_back(
								std::make_unique<middle::EditorActionOpenComponent>(componentRef->componentName)
							);
						}
					}
				}
			}

			
			});
	}
};

static middle::SystemRegistrar<EnviromentalFileNavigationSystem> reg("EnviromentalFileNavigationSystem");
