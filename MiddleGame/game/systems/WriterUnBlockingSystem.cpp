#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "imgui.h"
#include "alg_file_utils.h"
#include "bubble_paths.h"
#include "equlab_actions.h"
#include "middle_shape_utils.h"
#include "bubble_utils.h"
#include "MidComp/PuzzleTextUnit.h"
#include "MidComp/Text.h"
#include "MidComp/GlobalTransform.h"
#include "MidComp/UiComponent.h"
#include "MidComp/PuzzleTextPanel.h"
#include "MidComp/Rectangle.h"
#include "MidComp/LoopTag.h"
#include "MidComp/Layer.h"
#include "MidComp/SceneObjectComponent.h"
#include "MidComp/TopDogBubbleTag.h"
#include "MidComp/LocalPosition.h"
#include "MidComp/LocalScale.h"
#include "bubequ_mapping.h"
#include "MidComp/IntersectingTag.h"
#include "bubble_actions.h"
#include "midconfig.h"


class WriterUnBlockingSystem : public middle::MiddleGameplaySystem {
	components::CompCache* textCache;

	void init(middle::GameState* gameState) override {
		textCache = middle::newCompCache(gameState, systemName);
		textCache->addType<components::BubbleTextComponent>();
		textCache->addType<components::IntersectingTag>();
	}

	void update(middle::GameState* gameState) override {

		static char title[128] = ""; // buffer for scene name input
		static char textProblem[1024] = ""; // buffer for scene name input

		auto writingUi = [gameState, this]() {
			ImGui::Begin("Word Problem");
			ImGui::InputText("Title", title, IM_ARRAYSIZE(title));
			if (ImGui::IsItemActive()) {
				middle::insertInputBlock(gameState, middle::InputBlockers::KEYBOARD_BLOCK);
				middle::insertInputBlock(gameState, middle::InputBlockers::MOUSE_BLOCK);
			}
			ImGui::InputTextMultiline("Text", textProblem, IM_ARRAYSIZE(textProblem), ImVec2(0, 200), ImGuiInputTextFlags_WordWrap);
			if (ImGui::IsItemActive()) {
				middle::insertInputBlock(gameState, middle::InputBlockers::KEYBOARD_BLOCK);
				middle::insertInputBlock(gameState, middle::InputBlockers::MOUSE_BLOCK);
			}
			if (ImGui::Button("Save Text")) {
				std::string path = std::string(bubblePaths::WORD_PROBLEMS_FOLDER) + "/" + title + ".txt";
				bubequ::saveTextFile(path, textProblem);
			}
			ImGui::End();

			};
		middle::queueUi(gameState, writingUi);

		if (gameState->middleState.equlabInput.f9Clicked) {
			for (middle::Id id : textCache->relevantIdVector) {
				auto action = std::make_shared<equlab::LinkTextToTextBubble>(id, title, textProblem);
				bubble::queueEqulabAction(gameState, id, action);
			}
		}
	}
};

static middle::SystemRegistrar<WriterUnBlockingSystem> reg("WriterUnBlockingSystem");
