#include "middle_imgui.h"
#include "middle_math.h"
#include "editor_actions.h"
#include <memory>


namespace middle {

	Vector3 referencePos = { 0,0,0 };

	void DrawVector(Vector3 pos, Vector3 v) {
		Vector3 end = pos + v;
		rlDisableDepthTest();
		rlDisableDepthMask();
		DrawLine3D(pos, end, RED);
		rlEnableDepthMask();
		rlEnableDepthTest();
	}

	void ImGuiDisplay(const char* label, const char* text) {
		if (ImGui::CollapsingHeader(label))
			ImGui::Text("%s", text);
	}

	void ImGuiDisplay(const char* label, bool b) {
		if (ImGui::CollapsingHeader(label))
			ImGui::Text("%s", b ? "true" : "false");
	}

	void ImGuiDisplay(const char* label, int i) {
		if (ImGui::CollapsingHeader(label))
			ImGui::Text("%d", i);
	}

	void ImGuiDisplay(const char* label, float f) {
		if (ImGui::CollapsingHeader(label))
			ImGui::Text("%.25f", f);
	}

	void ImGuiDisplay(const char* label, Vector2 v) {
		if (ImGui::CollapsingHeader(label))
			ImGui::Text("(%.2f, %.2f)", v.x, v.y);
	}

	void ImGuiDisplay(const char* label, Vector3 v) {
		if (ImGui::CollapsingHeader(label)) {
			ImGui::Text("x: (%.25f)", v.x);
			ImGui::Text("y: (%.25f)", v.y);
			ImGui::Text("z: (%.25f)", v.z);
			DrawVector(referencePos, v);
		}

	}

	void setupUI(GameState* gameState)
	{
		rlImGuiBegin();

		ImGui::Begin("Control");
		if (!gameState->paused) {
			if (ImGui::Button("pause")) {
				gameState->paused = true;
			}
		}
		else {
			if (ImGui::Button("continue")) {
				gameState->paused = false;
				gameState->stepDir = 1;
			}
			if (ImGui::Button("next step")) {
				gameState->doOneStep = true;
				gameState->stepDir = 1;
			}
			if (ImGui::Button("previous step")) {
				gameState->doOneStep = true;
				gameState->stepDir = -1;
			}
		}
		if (ImGui::Button("reset")) {
			gameState->reset = true;
		}
		if (ImGui::Button("increase grid")) {
			++gameState->gridSize;
		}
		if (ImGui::Button("decrease grid")) {
			--gameState->gridSize;
			if (gameState->gridSize < 1)
				gameState->gridSize = 1;
		}
		ImGui::End();

		ImGui::Begin("GameState");
		ImGuiDisplay("camera position", gameState->camera.position);
		ImGuiDisplay("camera up", gameState->camera.up);
		ImGuiDisplay("camera target", gameState->camera.target);
		ImGuiDisplay("mousePos", gameState->input.mousePos);
		ImGuiDisplay("mouse near plane pos", gameState->input.mouseNearPlanePos);
		ImGuiDisplay("mouse normalized pos", gameState->input.mouseNormalizedPos);
		ImGuiDisplay("mouse dir", gameState->input.mouseDir);
		ImGuiDisplay("mouse xz pos", gameState->input.mouseXZ_PlanePos);
		ImGuiDisplay("mouse xz vel", gameState->input.mouseXZ_PlaneVelocity);
		ImGuiDisplay("screen width", gameState->screenWidth);
		ImGuiDisplay("screen height", gameState->screenHeight);
		ImGui::End();

		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!gameState->isShapeAlive(i))
				continue;
			Shape& shape = gameState->shapes[i];
			ShapeInstance& shapeInstance = gameState->getShapeInstance(i);

			Vector3 pos = FromDescVec(shapeInstance.pData.position);

			if ((shape.physicalShape && shapeInstance.selected && shapeInstance.infoVisible) || gameState->showAllInfo) {
				referencePos = pos;

				ImGui::Begin(std::to_string(i).c_str());
				descart::PhysicsBody& pData = shapeInstance.pData;
				descart::SweepResult& sweepData = shapeInstance.sweepData;
				ImGuiDisplay("radius", shape.radius);
				ImGuiDisplay("linear acceleration", FromDescVec(pData.linearAcc));
				ImGuiDisplay("linear vel", FromDescVec(pData.linearVel));
				ImGuiDisplay("linear vel before", FromDescVec(pData.linearVelBefore));
				ImGuiDisplay("linear vel after forces", FromDescVec(pData.linearVelAfterForces));
				ImGuiDisplay("linear vel after impulses", FromDescVec(pData.linearVelAfterImpulses));
				ImGuiDisplay("linear disp", FromDescVec(pData.linearDisp));
				ImGuiDisplay("linear disp before", FromDescVec(pData.linearDispBefore));
				ImGuiDisplay("linear disp after forces", FromDescVec(pData.linearDispAfterForces));
				ImGuiDisplay("linear disp after impulses", FromDescVec(pData.linearDispAfterImpulses));
				ImGuiDisplay("mouse intersects", shapeInstance.mouseIntersects);
				ImGuiDisplay("position", pos);
				ImGuiDisplay("collided", sweepData.collided);
				ImGuiDisplay("collision point", FromDescVec(sweepData.collisionPoint));
				ImGuiDisplay("normal", FromDescVec(sweepData.normal));
				ImGuiDisplay("toi", sweepData.toi);
				ImGui::Separator();
				ImGuiDisplay("parent loop", shape.parentLoopIndex);
				ImGuiDisplay("loop array offset", shape.loopArrayOffset);
				ImGui::End();
			}

		}

		ImGui::Begin("Editor");

		const char* items[] = { "SELECT MODE", "SPHERE MODE", "CONSTRAINT MODE" };
		static int currentItem = 0;
		if (gameState->input.selectModeClick) {
			currentItem = 0;
		}
		if (gameState->input.sphereModeClick) {
			currentItem = 1;
		}
		if (gameState->input.constraintModeClick) {
			currentItem = 2;
		}
		ImGui::Combo("Select things to add", &currentItem, items, IM_ARRAYSIZE(items));
		gameState->creationMode = static_cast<CreationMode>(currentItem);

		auto end = gameState->inputBlockers.end();
		auto& blockers = gameState->inputBlockers;

		if (gameState->creationMode != CreationMode::SELECT_MODE) {

			if (gameState->creationMode == CreationMode::SPHERE_MODE && gameState->input.mouseClicked) {
				gameState->nextEditorAction = EditorAction::NEW_SPHERE;
			}
			if (gameState->creationMode == CreationMode::CONSTRAINT_MODE && gameState->selectCount == 2) {
				gameState->nextEditorAction = EditorAction::NEW_CONSTRAINT;
			}

		}

		if (ImGui::Button("DELETE OBJECT") || gameState->input.deleteClick) {
			gameState->nextEditorAction = EditorAction::DELETE_SHAPES;
		}

		if (ImGui::Button("SAVE SCENE") || gameState->input.saveClick) {
			gameState->nextEditorAction = EditorAction::SAVE_SCENE;
		}

		if (ImGui::Button("CREATE LOOP") || gameState->input.loopClick) {
			gameState->nextEditorAction = EditorAction::CREATE_LOOPS;
		}

		ImGui::Separator();

		if (ImGui::Button("BUILD")) {
			gameState->nextEditorAction = EditorAction::BUILD;
		}



		// SCENE MANAGER
		ImGui::Begin("Scene");

		if (gameState->sceneNames.size() > 0) {
			ImGui::Text(("ActiveScene: " + gameState->sceneNames[gameState->activeScene]).c_str());
		}

		static EditorAction popupAction = EditorAction::LOAD_SCENE;
		// Add new scene button
		if (ImGui::Button("OPEN SCENE")) {
			ImGui::OpenPopup("Scene Selector");
			popupAction = EditorAction::LOAD_SCENE;
		}
		if (ImGui::Button("IMPORT SCENE")) {
			ImGui::OpenPopup("Scene Selector");
			popupAction = EditorAction::IMPORT_SCENE;
		}

		if (ImGui::BeginPopup("Scene Selector")) {

			for (int i = 0; i < gameState->sceneNames.size(); ++i) {
				auto name = gameState->sceneNames[i];
				if (ImGui::Button(name.c_str())) {
					EditorActionContainer::Params params;
					params.intValue = i;
					gameState->nextEditorAction = popupAction;
					gameState->nextEditorActionParams = params;
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		ImGui::Separator();

		// Add new scene button
		if (ImGui::Button("ADD NEW SCENE")) {
			ImGui::OpenPopup("New Scene Popup");
		}

		// Popup for entering new scene name
		static char newSceneName[128] = ""; // buffer for scene name input
		if (ImGui::BeginPopup("New Scene Popup")) {
			gameState->inputBlockers.insert(InputBlockers::KEYBOARD_BLOCK);

			ImGui::Text("Enter new scene name:");
			ImGui::InputText("##newSceneName", newSceneName, IM_ARRAYSIZE(newSceneName));

			if (ImGui::Button("Add")) {
				if (strlen(newSceneName) > 0) {
					// Add the new scene to the editor
					EditorActionContainer::Params params;
					params.stringValue = std::string(newSceneName);
					params.intValue = gameState->sceneNames.size();
					gameState->nextEditorAction = EditorAction::NEW_SCENE;
					gameState->nextEditorActionParams = params;

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
		ImGui::End();


		ImGui::End();

		rlImGuiEnd();
	}


}
