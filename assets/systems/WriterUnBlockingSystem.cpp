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
#include "GlobalTransform.h"
#include "UiComponent.h"
#include "PuzzleTextPanel.h"
#include "Rectangle.h"
#include "LoopTag.h"
#include "Layer.h"
#include "SceneObjectComponent.h"
#include "TopDogBubbleTag.h"
#include "bubble_paths.h"
#include "LocalPosition.h"
#include "LocalScale.h"


class WriterUnBlockingSystem : public middle::MiddleGameplaySystem {
	void init(middle::GameState* gameState) override {

	}

	middle::Id createTextPanel(middle::GameState* gameState, bubequ::WordProblem* problem, const Vector3& pos) {

		// load bubequs
		const float spacing = 600;
		Vector3 offset = { 0,0,-spacing };

		// CREATE TEXT PANEL
		middle::Shape textPanelProto;
		middle::addComponent<components::PuzzleTextPanel>(textPanelProto);
		middle::addComponent<components::GlobalTransform>(textPanelProto);
		middle::addComponent<components::LocalPosition>(textPanelProto);
		middle::addComponent<components::LocalScale>(textPanelProto);
		auto rectangle = middle::addComponent<components::Rectangle>(textPanelProto);
		rectangle->width = 150;
		rectangle->height = 300;
		middle::addComponent<components::UiComponent>(textPanelProto);
		middle::addComponent<components::Layer>(textPanelProto);
		middle::addComponent<components::LoopSociety>(textPanelProto);
		middle::addComponent<components::SceneObjectComponent>(textPanelProto);
		middle::Shape& textPanel = middle::registerShape(gameState, textPanelProto);
		Vector3 targetPos = gameState->activeCamera.position;
		targetPos.y = 0;
		middle::moveShape(gameState, textPanel.id.index,
			targetPos - middle::getGlobalPosition(gameState, textPanel.id.index));

		// CREATE TEXT UNITS FOR TEXT PANEL
		for (auto& unit : problem->sentenceUnits) {

			// create text shapes
			middle::Shape textUnitProto;
			middle::addComponent<components::PuzzleTextUnit>(textUnitProto);
			auto textComp = middle::addComponent<components::Text>(textUnitProto);
			textComp->text = unit.text;
			textComp->fontSize = 10;
			middle::addComponent<components::GlobalTransform>(textUnitProto);
			middle::addComponent<components::LocalPosition>(textUnitProto);
			middle::addComponent<components::LocalScale>(textUnitProto);
			middle::addComponent<components::Rectangle>(textUnitProto);
			middle::addComponent<components::UiComponent>(textUnitProto);
			middle::addComponent<components::LoopSociety>(textUnitProto);
			middle::Shape& textUnitShape = middle::registerShape(gameState, textUnitProto);
			middle::EditorActionReparent(textPanel.id.index, textUnitShape.id.index).execute(gameState);
			auto registerAction2 = std::make_shared<middle::EditorActionRegisterId>(textUnitShape.id);
		}
		return textPanel.id;
	}


	void update(middle::GameState* gameState) override {
		auto writingUi = [gameState, this]() {

			ImGui::Begin("Word Problem");
			static char title[128] = ""; // buffer for scene name input
			ImGui::InputText("Title", title, IM_ARRAYSIZE(title));
			if (ImGui::IsItemActive()) {
				gameState->inputBlockers.insert(middle::InputBlockers::KEYBOARD_BLOCK);
				gameState->inputBlockers.insert(middle::InputBlockers::MOUSE_BLOCK);
			}
			static char textProblem[1024] = ""; // buffer for scene name input
			ImGui::InputTextMultiline("Text", textProblem, IM_ARRAYSIZE(textProblem), ImVec2(0, 200), ImGuiInputTextFlags_WordWrap);
			if (ImGui::IsItemActive()) {
				gameState->inputBlockers.insert(middle::InputBlockers::KEYBOARD_BLOCK);
				gameState->inputBlockers.insert(middle::InputBlockers::MOUSE_BLOCK);
			}
			if (ImGui::Button("Save Text")) {
				std::string path = bubblePaths::WORD_PROBLEMS_FOLDER + "/" + title + ".txt";
				bubequ::saveTextFile(path, textProblem);
			}
			ImGui::End();


			ImGui::Begin("Word Problem Mobjs");
			static char titleProblemMobj[128] = ""; // buffer for scene name input
			ImGui::InputText("Title", titleProblemMobj, IM_ARRAYSIZE(title));
			if (ImGui::IsItemActive()) {
				gameState->inputBlockers.insert(middle::InputBlockers::KEYBOARD_BLOCK);
				gameState->inputBlockers.insert(middle::InputBlockers::MOUSE_BLOCK);
			}
			static char textProblemMobj[612] = ""; // buffer for scene name input
			ImGui::InputTextMultiline("Text", textProblemMobj, IM_ARRAYSIZE(textProblemMobj), ImVec2(0, 100), ImGuiInputTextFlags_WordWrap);
			if (ImGui::IsItemActive()) {
				gameState->inputBlockers.insert(middle::InputBlockers::KEYBOARD_BLOCK);
				gameState->inputBlockers.insert(middle::InputBlockers::MOUSE_BLOCK);
			}
			if (ImGui::Button("Save Text")) {
				std::string path = bubblePaths::WORD_PROBLEM_MOBJS_FOLDER + "/" + titleProblemMobj + ".txt";
				bubequ::saveTextFile(path, textProblemMobj);
			}
			ImGui::End();


			ImGui::Begin("WordProblemFile");
			std::vector<std::string>textFilenames = bubequ::getFilenames(bubblePaths::WORD_PROBLEMS_FOLDER);
			for (const std::string& filename : textFilenames) {
				if (ImGui::Button(filename.c_str())) {
					bubequ::WordProblem problem =
						bubequ::loadWordProblem(bubblePaths::WORD_PROBLEMS_FOLDER + "/" + filename);
					// remove .txt 
					std::string titleText = filename.substr(0, filename.size() - 4);
					snprintf(title, sizeof(title), "%s", titleText.c_str());
					snprintf(textProblem, sizeof(textProblem), "%s", problem.rawText.c_str());
					Vector3 cameraXZPos = gameState->activeCamera.position;
					cameraXZPos.y = 0;
					middle::Id textPanelId = createTextPanel(gameState, &problem, cameraXZPos);
					auto registerAction = std::make_shared<middle::EditorActionRegisterId>(textPanelId);
					middle::queueAction(gameState, registerAction);
					gameState->bubbleAlgebraState.bubbleActions.push_back(registerAction);
				}
			}
			ImGui::End();


			ImGui::Begin("WordProblemMobjFile");
			std::vector<std::string>mobjFilenames = bubequ::getFilenames(bubblePaths::WORD_PROBLEM_MOBJS_FOLDER);
			for (const std::string& filename : mobjFilenames) {
				if (ImGui::Button(filename.c_str())) {
					// fill input text
					std::string text = bubequ::loadText(bubblePaths::WORD_PROBLEM_MOBJS_FOLDER + "/" + filename);
					// remove .txt 
					std::string titleText = filename.substr(0, filename.size() - 4);
					snprintf(titleProblemMobj, sizeof(titleProblemMobj), "%s", titleText.c_str());
					snprintf(textProblemMobj, sizeof(textProblemMobj), "%s", text.c_str());

					std::vector<std::shared_ptr<middle::EditorActionContainer>> actions;

					// create text panel
					bubequ::WordProblemMobjs mobjs = bubequ::loadWordProblemMobjs(bubblePaths::WORD_PROBLEM_MOBJS_FOLDER + "/" + filename);
					Vector3 cameraXZPos = gameState->activeCamera.position;
					cameraXZPos.y = 0;
					middle::Id textPanelId = createTextPanel(gameState, mobjs.problem.get(), cameraXZPos);
					auto registerPanel = std::make_shared<middle::EditorActionRegisterId>(textPanelId);
					actions.push_back(registerPanel);

					const float spacing = 400;
					// load solution mobj
					if (mobjs.solutionMobj != nullptr) {
						Vector3 targetPos = cameraXZPos;
						targetPos.z -= spacing;
						middle::Id bubbleId = equlab::bubequToBubble(gameState, targetPos, mobjs.solutionMobj);
						auto registerAction = std::make_shared<middle::EditorActionRegisterId>(bubbleId);
						actions.push_back(registerAction);
					}

					auto multiAction = std::make_shared<middle::MultiAction>(actions);
					middle::queueAction(gameState, multiAction);
					gameState->bubbleAlgebraState.bubbleActions.push_back(multiAction);
				}
			}

			ImGui::End();
			};
		gameState->uiSetups.push_back(writingUi);
	}
};

static middle::SystemRegistrar<WriterUnBlockingSystem> reg("WriterUnBlockingSystem");
