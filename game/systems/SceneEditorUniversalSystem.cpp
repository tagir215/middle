#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "ActiveSceneEditableTag.h"
#include "component_utils.h"
#include "imgui.h"

class SceneEditorUniversalSystem : public middle::MiddleGameplaySystem {
	components::CompCache* activeObjectCache;

	void init(middle::GameState* gameState) override {
		activeObjectCache = middle::newCompCache(gameState, systemName);
		activeObjectCache->addType<components::ActiveSceneSelectableTag>();
	}
	void update(middle::GameState* gameState) override {
		auto ui = [gameState, this]() {
			ImGui::Begin("Universal editor");
			if (ImGui::Button("Delete active")) {
				for (middle::Id activeId : activeObjectCache->relevantIdVector) {
					auto deleteAction = std::make_shared<middle::EditorActionDeleteSingle>(activeId);
					middle::queueAction(gameState, deleteAction);
					gameState->bubbleAlgebraState.bubbleActions.push_back(deleteAction);
				}
			}
			ImGui::End();
			};
		gameState->uiSetups.push_back(ui);

	}
};

static middle::SystemRegistrar<SceneEditorUniversalSystem> reg("SceneEditorUniversalSystem");
