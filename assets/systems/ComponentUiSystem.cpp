#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MouseSelectable.h"
#include "imgui.h"
#include "editor_actions.h"

class ComponentUiSystem : public middle::MiddleGameplaySystem {

	const int maxFieldCount = 100;

	void update(middle::GameState* gameState) override {
		if (gameState->fields.size() < maxFieldCount) {
			gameState->fields.resize(maxFieldCount);
		}

		if (gameState->editorState.selectCount != 1) {
			return;
		}


		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto selected = middle::getComponent<components::MouseSelectable>(shape);
			if (!selected || !selected->selected)
				return;


			auto ui = [&shape, gameState]() {
				ImGui::Begin("ComponentEditor");

				for (auto& pair : shape.componentMap) {
					int typeId = pair.first;
					int componentOffset = pair.second.componentOffset;
					middle::Serializable* serializable = middle::componentListMap[typeId]->getSerializable(componentOffset);
					std::string typeData;
					const char* componentName = middle::componentNameMap[typeId].c_str();
					ImGui::Separator();
					ImGui::PushID((char)pair.first);
					if (ImGui::Button(".")) {
						gameState->editorState.editorActions.push_back(
							std::make_unique<middle::EditorActionOpenComponent>(componentName)
						);
					}
					ImGui::PopID();
					ImGui::SameLine();
					if (ImGui::CollapsingHeader(componentName)) {
						int size = 0;
						serializable->getFields(gameState->fields, &size);
						for (int fieldIndex = 0; fieldIndex < size; ++fieldIndex) {
							middle::FieldInfo field = gameState->fields[fieldIndex];
							if(field.type == middle::FieldType::Bool){
								ImGui::Checkbox(field.name, static_cast<bool*>(field.value));
							}
							else if(field.type == middle::FieldType::Float){
								ImGui::InputFloat(field.name, static_cast<float*>(field.value));
							}
							else if (field.type == middle::FieldType::Id) {
								middle::Id* id = static_cast<middle::Id*>(field.value);
								ImGui::InputInt(field.name, &id->index);
							}
							else if (field.type == middle::FieldType::String) {
								std::string* string = static_cast<std::string*>(field.value);
								//ImGui::InputText(field.name, , string);
							}
							else{
								ImGui::Text(field.name);
							}
						}
					}

				};

				ImGui::End();
				};

			gameState->uiSetups.push_back(ui);
			});
	}
};

static middle::SystemRegistrar<ComponentUiSystem> reg("ComponentUiSystem");
