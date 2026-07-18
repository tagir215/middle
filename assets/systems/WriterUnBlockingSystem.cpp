#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "imgui.h"
#include "alg_file_utils.h"
#include "bubble_paths.h"
#include "equlab_actions.h"
#include "middle_shape_utils.h"
#include "bubble_utils.h"
#include "PuzzleTextUnit.h"
#include "Text.h"
#include "Position.h"
#include "UiComponent.h"
#include "PuzzleTextPanel.h"
#include "Rectangle.h"
#include "LoopTag.h"
#include "Layer.h"
#include "SceneObjectComponent.h"
#include "TopDogBubbleTag.h"


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
				bubequ::savePuzzleText(title, text);
			}
			ImGui::Separator();

			std::vector<std::string>textFilenames = bubequ::getFilenames(bubblePaths::WORD_PROBLEMS_FOLDER);
			for (const std::string& filename : textFilenames) {
				if (ImGui::Button(filename.c_str())) {
					bubequ::WordProblem problem =  
						bubequ::loadPuzzleText(bubblePaths::WORD_PROBLEMS_FOLDER + "/" + filename);
					// remove .txt 
					std::string titleText = filename.substr(0, filename.size() - 4);
					snprintf(title, sizeof(title), "%s", titleText.c_str());
					snprintf(text, sizeof(text), "%s", problem.rawText.c_str());

					// load bubequs
					Vector3 cameraXZPos = gameState->activeCamera.position;
					cameraXZPos.y = 0;
					const float spacing = 600;
					Vector3 offset = { 0,0,-spacing };

					std::vector < std::shared_ptr<middle::EditorActionContainer>> actions;


					// CREATE TEXT PANEL
					middle::Shape textPanelProto;
					middle::addComponent<components::PuzzleTextPanel>(textPanelProto);
					middle::addComponent<components::Position>(textPanelProto);
					auto rectangle = middle::addComponent<components::Rectangle>(textPanelProto);
					rectangle->width = 150;
					rectangle->height = 300;
					middle::addComponent<components::UiComponent>(textPanelProto);
					middle::addComponent<components::Layer>(textPanelProto);
					middle::addComponent<components::LoopSociety>(textPanelProto);
					middle::addComponent<components::SceneObjectComponent>(textPanelProto);
					middle::Shape& textPanel = middle::registerShape(gameState, textPanelProto);
					auto registerAction0 = std::make_shared<middle::EditorActionRegisterId>(textPanel.id);
					actions.push_back(registerAction0);
					Vector3 targetPos = gameState->activeCamera.position;
					targetPos.y = 0;
					middle::moveShape(gameState, textPanel.id.index, 
						targetPos - middle::getShapePosition(gameState, textPanel.id.index));

					// CREATE TEXT UNITS FOR TEXT PANEL
					for (auto& unit : problem.sentenceUnits) {

						// create text shapes
						middle::Shape textUnitProto;
						middle::addComponent<components::PuzzleTextUnit>(textUnitProto);
						auto textComp = middle::addComponent<components::Text>(textUnitProto);
						textComp->text = unit.text;
						textComp->fontSize = 10;
						middle::addComponent<components::Position>(textUnitProto);
						middle::addComponent<components::Rectangle>(textUnitProto);
						middle::addComponent<components::UiComponent>(textUnitProto);
						middle::addComponent<components::LoopSociety>(textUnitProto);
						middle::Shape& textUnitShape = middle::registerShape(gameState, textUnitProto);
						middle::EditorActionReparent(textPanel.id.index, textUnitShape.id.index).execute(gameState);
						auto registerAction2 = std::make_shared<middle::EditorActionRegisterId>(textUnitShape.id);
						actions.push_back(registerAction2);
					}


					if (actions.size() > 0) {
						auto multiAction = std::make_shared<middle::MultiAction>(actions);
						middle::queueAction(gameState, multiAction);
						gameState->bubbleAlgebraState.bubbleActions.push_back(multiAction);
					}
				}
			}

			ImGui::End();
			};
		gameState->uiSetups.push_back(writingUi);
	}
};

static middle::SystemRegistrar<WriterUnBlockingSystem> reg("WriterUnBlockingSystem");
