#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MouseClickComponent.h"
#include "Button.h"
#include "bubble_actions.h"
#include "bubble_utils.h"
#include "Rectangle.h"
#include "CameraComponent.h"
#include "ProcedureContainer.h"
#include "bubble_constants.h"
#include "editor_file_utils.h"
#include "imgui.h"
#include "GlobalTransform.h"

class AlgebraProblemSystem : public middle::MiddleGameplaySystem {
public:


	void init(middle::GameState* gameState) {
	}

	void undo(middle::GameState* gameState) {


		if (gameState->bubbleAlgebraState.bubbleActions.size() > 0) {
			middle::queueAction(gameState, std::make_shared<middle::CustomAction>([](middle::GameState* gameState) {
				gameState->bubbleAlgebraState.bubbleActions.back()->undo(gameState);
				gameState->bubbleAlgebraState.bubbleActions.pop_back();
				}));
		}
		gameState->bubbleAlgebraState.postUndoFrames = 2;
		queueSound(gameState, bubbleSounds::UNDO_SOUND);
	}

	void update(middle::GameState* gameState) override {

		if (gameState->bubbleAlgebraState.postUndoFrames > 0) {
			--gameState->bubbleAlgebraState.postUndoFrames;
		}

		if (gameState->gameInput.undo) {
			undo(gameState);
		}

		auto undoUi = [this, gameState] {
			ImGui::Begin("--");

			if (ImGui::Button("UNDO")) {
				undo(gameState);
			}
			ImGui::End();
			};

		gameState->uiSetups.push_back(undoUi);


	}

};

static middle::SystemRegistrar<AlgebraProblemSystem> reg("AlgebraProblemSystem");
