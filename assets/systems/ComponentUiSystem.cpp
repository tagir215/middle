#pragma once
#include "game_state.h"
#include "registrars.h"
#include "middle_shape_utils.h"
#include "MouseSelectable.h"
#include "imgui.h"
#include "editor_actions.h"

class ComponentUiSystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto selected = middle::getComponent<components::MouseSelectable>(shape);
			if (!selected || !selected->selected)
				return;


			auto ui = [&shape, gameState]() {
				ImGui::Begin("ComponentEditor");

				for (auto& pair : shape.componentMap) {
					int typeId = pair.first;
					middle::Serializable* serializable = middle::getSerializableComponent(shape, typeId);
					std::string typeData;
					//serializable->serialize(typeData);
					//std::vector<std::string>lines = middle::splitString(typeData, '\n');
					const char* componentName = middle::componentNameMap[typeId].c_str();
					ImGui::Separator();
					ImGui::Text(componentName); 
					ImGui::SameLine();
					ImGui::PushID((char)pair.first);
					if (ImGui::Button(".")) {
						gameState->editorState.editorActions.push_back(
							std::make_unique<middle::EditorActionOpenComponent>(componentName)
						);
					}
					ImGui::PopID();

					//for (auto& line : lines) {
					//	char t = line[0];
					//	if (t == 'f') {
					//		static float value;
					//		ImGui::InputFloat(line.c_str(), &value);
					//	}
					//}
				};

				ImGui::End();
				};

			gameState->uiSetups.push_back(ui);
			});
	}
};

static middle::SystemRegistrar<ComponentUiSystem> reg("ComponentUiSystem");
