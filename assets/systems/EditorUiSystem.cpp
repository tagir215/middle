#pragma once
#include "game_state.h"
#include "editor_actions.h"
#include "middle_system_registrar.h"
#include "imgui.h"
#include "middle_shape_utils.h"
#include "LoopSociety.h"
#include "EditorConfigs.h"

class EditorUiSystem : public middle::MiddleGameplaySystem {
public:
	EditorUiSystem() {
		systemUpdateType = middle::SystemUpdateType::RENDERING;
		systemModeType = middle::SystemModeType::ENGINE;
	}

	void init(middle::GameState* gameState) {

	}

	void editorUi(middle::GameState* gameState) {

		middle::Id& editorConfigs = 
			middle::findFirstShapeWithComp(gameState, middle::getTypeId<components::EditorConfigs>());

		components::EditorConfigs* configs = nullptr;
		if (editorConfigs.index != middle::UNASSIGNED) {
			auto& configShape = middle::getShape(gameState, editorConfigs.index);
			configs = middle::getComponent<components::EditorConfigs>(configShape);
		}


		auto ui = [gameState, configs]() {
			Vector3 referencePos = { 0,0,0 };

			auto ImGuiDisplayText = [](const char* label, const char* text) {
				if (ImGui::CollapsingHeader(label))
					ImGui::Text("%s", text);
				};

			auto ImGuiDisplayBool = [](const char* label, bool b) {
				if (ImGui::CollapsingHeader(label))
					ImGui::Text("%s", b ? "true" : "false");
				};

			auto ImGuiDisplayInt = [](const char* label, int i) {
				if (ImGui::CollapsingHeader(label))
					ImGui::Text("%d", i);
				};

			auto ImGuiDisplayFloat = [](const char* label, float f) {
				if (ImGui::CollapsingHeader(label))
					ImGui::Text("%.25f", f);
				};

			auto ImGuiDisplayVector2 = [](const char* label, Vector2 v) {
				if (ImGui::CollapsingHeader(label))
					ImGui::Text("(%.2f, %.2f)", v.x, v.y);
				};

			auto ImGuiDisplayVector3 = [&referencePos](const char* label, Vector3 v) {
				if (ImGui::CollapsingHeader(label)) {
					ImGui::Text("x: (%.25f)", v.x);
					ImGui::Text("y: (%.25f)", v.y);
					ImGui::Text("z: (%.25f)", v.z);
				}
				};

			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
				gameState->inputBlockers.insert(middle::InputBlockers::MOUSE_BLOCK);
			}

			ImGui::Begin("Control");
			if (!gameState->paused) {
				if (ImGui::Button("pause")) {
					gameState->paused = true;
				}
				if (ImGui::Button("undo")) {
					int& actionPointer = gameState->editorState.actionPointer;
					if (actionPointer >= 1) {
						gameState->editorState.editorActions[actionPointer - 1]->undo(gameState);
						--actionPointer;
					}
				}
				if (ImGui::Button("redo")) {
					int& actionPointer = gameState->editorState.actionPointer;
					if (actionPointer < gameState->editorState.editorActions.size()) {
						gameState->editorState.editorActions[actionPointer]->execute(gameState);
						++actionPointer;
					}
				}
			}
			else {
				if (ImGui::Button("continue")) {
					gameState->paused = false;
					gameState->editorState.stepDir = 1;
				}
				if (ImGui::Button("next step")) {
					gameState->editorState.doOneStep = true;
					gameState->editorState.stepDir = 1;
				}
				if (ImGui::Button("previous step")) {
					gameState->editorState.doOneStep = true;
					gameState->editorState.stepDir = -1;
				}
			}
			if (ImGui::Button("reset")) {
				gameState->reset = true;
			}
			if (configs) {
				if (ImGui::Button("increase grid")) {
					++configs->gridSize;
				}
				if (ImGui::Button("decrease grid")) {
					--configs->gridSize;
					if (configs->gridSize < 1)
						configs->gridSize = 1;
				}
			}
			if (ImGui::Button("PLAY")) {
				gameState->applicationMode = middle::ApplicationMode::GAME_MODE;
			}
			ImGui::End();
			ImGui::Begin("GameState");
			ImGuiDisplayVector3("camera position", gameState->editorState.camera.position);
			ImGuiDisplayVector3("camera up", gameState->editorState.camera.up);
			ImGuiDisplayVector3("camera target", gameState->editorState.camera.target);
			ImGuiDisplayVector2("mousePos", gameState->input.mousePos);
			ImGuiDisplayVector3("mouse near plane pos", gameState->input.mouseNearPlanePos);
			ImGuiDisplayVector2("mouse normalized pos", gameState->input.mouseNormalizedPos);
			ImGuiDisplayVector3("mouse dir", gameState->input.mouseDir);
			ImGuiDisplayVector3("mouse xz pos", gameState->input.mouseXZ_PlanePos);
			ImGuiDisplayVector3("mouse xz vel", gameState->input.mouseXZ_PlaneVelocity);
			ImGuiDisplayInt("screen width", gameState->screenWidth);
			ImGuiDisplayInt("screen height", gameState->screenHeight);
			ImGui::End();

			ImGui::Begin("Editor");

			const char* items[] = { "SELECT MODE", "SPHERE MODE", "CONSTRAINT MODE", "CAMERA MODE", "LOOP_MODE" };
			int currentItem = static_cast<int>(gameState->editorState.creationMode);
			ImGui::Combo("Select things to add", &currentItem, items, IM_ARRAYSIZE(items));
			gameState->editorState.creationMode = static_cast<middle::CreationMode>(currentItem);

			auto end = gameState->inputBlockers.end();
			auto& blockers = gameState->inputBlockers;


			if (ImGui::Button("DELETE OBJECT")) {
				gameState->editorState.editorActions.push_back(
					std::make_unique<middle::EditorActionDelete>(middle::getSelectedShapes(gameState))
				);
			}

			if (ImGui::Button("SAVE SCENE")) {
				gameState->editorState.editorActions.push_back(
					std::make_unique<middle::EditorActionSaveScene>(gameState->sceneNames[gameState->activeScene])
				);
			}

			if (ImGui::Button("CREATE LOOP")) {
				gameState->editorState.editorActions.push_back(
					std::make_unique<middle::EditorActionCreateLoop>(middle::getSelectedShapes(gameState))
				);
			}

			ImGui::Separator();

			if (ImGui::Button("BUILD")) {
				gameState->editorState.editorActions.push_back(
					std::make_unique<middle::EditorActionBuild>()
				);
			}



			// SCENE MANAGER

			ImGui::Begin("Scene");

			if (gameState->sceneNames.size() > 0) {
				ImGui::Text(("ActiveScene: " + gameState->sceneNames[gameState->activeScene]).c_str());
			}

			static int action = 0;
			int load = 1;
			int import = 2;
			// Add new scene button
			if (ImGui::Button("OPEN SCENE")) {
				ImGui::OpenPopup("Scene Selector");
				action = load;
			}


			if (ImGui::Button("IMPORT SCENE")) {
				ImGui::OpenPopup("Scene Selector");
				action = import;
			}

			if (ImGui::BeginPopup("Scene Selector")) {

				for (int i = 0; i < gameState->sceneNames.size(); ++i) {
					auto name = gameState->sceneNames[i];
					if (ImGui::Button(name.c_str())) {
						if (action == load) {
							gameState->editorState.editorActions.push_back(
								std::make_unique<middle::EditorActionLoadScene>(name)
							);
						}
						if (action == import) {
							gameState->editorState.editorActions.push_back(
								std::make_unique<middle::EditorActionImportScene>("../assets/scenes/", name)
							);
						}

						ImGui::CloseCurrentPopup();
					}
				}

				ImGui::EndPopup();
			}

			if (ImGui::Button("IMPORT SHAPE")) {
				ImGui::OpenPopup("Shape Selector");
			}

			if (ImGui::BeginPopup("Shape Selector")) {
				for (int i = 0; i < gameState->shapeNames.size(); ++i) {
					auto name = gameState->shapeNames[i];
					if (ImGui::Button(name.c_str())) {
						gameState->editorState.editorActions.push_back(
							std::make_unique<middle::EditorActionImportScene>("../assets/shapes/", name)
						);
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndPopup();
			}

			// Add new scene button
			if (ImGui::Button("ADD NEW SCENE")) {
				ImGui::OpenPopup("New Scene Popup");
			}

			// Popup for entering new scene name
			static char newSceneName[128] = ""; // buffer for scene name input
			if (ImGui::BeginPopup("New Scene Popup")) {
				gameState->inputBlockers.insert(middle::InputBlockers::KEYBOARD_BLOCK);

				ImGui::Text("Enter new scene name:");
				ImGui::InputText("##newSceneName", newSceneName, IM_ARRAYSIZE(newSceneName));

				if (ImGui::Button("Add")) {
					if (strlen(newSceneName) > 0) {
						// Add the new scene to the editor
						gameState->editorState.editorActions.push_back(
							std::make_unique<middle::EditorActionNewScene>(newSceneName)
						);

						// Clear buffer and close popup
						newSceneName[0] = '\0';
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					newSceneName[0] = '\0';
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			if (ImGui::Button("Sync Generations")) {
				middle::loopInstances(gameState, [gameState](int j, middle::Shape& shape) {
					auto loop = middle::getComponent<components::LoopSociety>(shape);
					if (loop) {
						for (int index = 0; index < loop->loopMemberIds.size(); ++index) {
							loop->loopMemberIds[index] = gameState->ids[loop->loopMemberIds[index].index];
						}
					}
					return true;
					});
			}


			ImGui::Separator();

			ImGui::End();





			// SCRIPT MANAGER

			ImGui::Begin("System Manager");

			// import SCRIPT
			if (ImGui::Button("IMPORT SYSTEM")) {
				ImGui::OpenPopup("System Selector");
			}

			// import COMPONENT
			ImGui::Button("IMPORT COMPONENT");
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
				ImGui::OpenPopup("Component Selector");
			}


			if (ImGui::BeginPopup("System Selector")) {

				for (auto& name : gameState->systemNames) {
					if (ImGui::Button(name.c_str())) {

						gameState->editorState.editorActions.push_back(
							std::make_unique<middle::EditorActionImportSystem>(name)
						);

						ImGui::CloseCurrentPopup();
					}
				}

				ImGui::EndPopup();
			}


			if (ImGui::BeginPopup("Component Selector")) {
				gameState->inputBlockers.insert(middle::InputBlockers::MOUSE_BLOCK);
				for (auto& name : gameState->componentNames) {
					if (ImGui::Button(name.c_str())) {
						gameState->editorState.editorActions.push_back(
							std::make_unique<middle::EditorActionImportComponent>(name, middle::getSelectedShapes(gameState))
						);
						ImGui::CloseCurrentPopup();
					}
				}

				ImGui::EndPopup();
			}

			// Add new system button
			if (ImGui::Button("ADD NEW SCRIPT")) {
				ImGui::OpenPopup("New Script Popup");
			}

			// Popup for entering new scene name
			static char newScriptName[128] = ""; // buffer for scene name input
			if (ImGui::BeginPopup("New Script Popup")) {
				gameState->inputBlockers.insert(middle::InputBlockers::KEYBOARD_BLOCK);

				ImGui::Text("Enter new initSystem name:");
				ImGui::InputText("##newScriptName", newScriptName, IM_ARRAYSIZE(newScriptName));

				const char* systemItems[] = { "SYSTEM", "COMPONENT" };
				static int selectedSystemItem = 0;
				ImGui::Combo("Component or System?", &selectedSystemItem, systemItems, IM_ARRAYSIZE(systemItems));

				if (ImGui::Button("Add")) {
					if (strlen(newScriptName) > 0) {
						// Add the new scene to the editor
						if (selectedSystemItem == 0) {
							gameState->editorState.editorActions.push_back(
								std::make_unique<middle::EditorActionNewSystem>(newScriptName)
							);
						}
						else {

							gameState->editorState.editorActions.push_back(
								std::make_unique<middle::EditorActionNewComponent>(newScriptName)
							);
						}

						// Clear buffer and close popup
						newScriptName[0] = '\0';
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					newScriptName[0] = '\0';
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			ImGui::End();


			ImGui::End();

			};


		gameState->uiSetups.push_back(ui);
	}

	void gameEditorUi(middle::GameState* gameState) {

		auto ui = [gameState]() {
			ImGui::Begin("Control");
			if (ImGui::Button("Edit")) {
				gameState->applicationMode = middle::ApplicationMode::EDITOR_MODE;
			}
			ImGui::End();
			};

		gameState->uiSetups.push_back(ui);
	}

	void update(middle::GameState* gameState) override {


		if (gameState->applicationMode == middle::ApplicationMode::EDITOR_MODE) {
			editorUi(gameState);
		}
		if (gameState->applicationMode == middle::ApplicationMode::GAME_MODE) {
			gameEditorUi(gameState);
		}
	}

};

static middle::SystemRegistrar<EditorUiSystem> reg("EditorUiSystem");
