#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "imgui.h"

class WriterUnBlockingSystem : public middle::MiddleGameplaySystem {
	void init(middle::GameState* gameState) override {

	}
	void update(middle::GameState* gameState) override {
		auto writingUi = [gameState]() {
			ImGui::Begin("TextInput");

			static char title[1024] = ""; // buffer for scene name input
			ImGui::InputText("Title", title, IM_ARRAYSIZE(title));
			if (ImGui::IsItemActive()) {
				gameState->inputBlockers.insert(middle::InputBlockers::KEYBOARD_BLOCK);
				gameState->inputBlockers.insert(middle::InputBlockers::MOUSE_BLOCK);
			}

			// Popup for entering new scene name
			static char text[1024] = ""; // buffer for scene name input
			ImGui::InputTextMultiline("Text", text, IM_ARRAYSIZE(text), ImVec2(0, 200), ImGuiInputTextFlags_WordWrap);
			if (ImGui::IsItemActive()) {
				gameState->inputBlockers.insert(middle::InputBlockers::KEYBOARD_BLOCK);
				gameState->inputBlockers.insert(middle::InputBlockers::MOUSE_BLOCK);
			}
			ImGui::End();

			ImGui::Begin("File");
			if (ImGui::Button("Save Text")) {

			}
			if (ImGui::Button("Save Equation/Term")) {

			}
			if (ImGui::Button("Save Problem reference")) {

			}
			ImGui::End();
			};
		gameState->uiSetups.push_back(writingUi);
	}
};

static middle::SystemRegistrar<WriterUnBlockingSystem> reg("WriterUnBlockingSystem");
