#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MouseSelectable.h"
#include "imgui.h"
#include "editor_actions.h"
#include <string>

class ComponentUiSystem : public middle::MiddleGameplaySystem {

public:
	ComponentUiSystem() {
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
		systemModeType = middle::SystemModeType::EDITOR;
	}

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
				std::string idText = "index: " + std::to_string(shape.id.index);
				ImGui::Text(idText.c_str());

				for (auto& pair : shape.componentMap) {
					int typeId = pair.first;
					int componentOffset = pair.second.componentOffset;
					middle::Serializable* serializable = middle::componentListMap[typeId]->getSerializable(componentOffset);
					std::string typeData;
					const char* componentName = middle::componentNameMap[typeId].c_str();
					ImGui::Separator();
					ImGui::PushID((char)pair.first);
					if (ImGui::Button("(o)")) {
						gameState->editorState.editorActions.push_back(
							std::make_unique<middle::EditorActionOpenComponent>(componentName)
						);
					}
					ImGui::SameLine();
					if (ImGui::Button("(d)")) {
						gameState->editorState.editorActions.push_back(
							std::make_unique<middle::EditorActionRemoveComponent>(componentName, middle::getSelectedShapes(gameState))
						);
					}


					ImGui::PopID();
					ImGui::SameLine();
					int size = 0;
					serializable->getFields(gameState->fields, &size);

					if (size > 0 && ImGui::CollapsingHeader(componentName)) {
						for (int fieldIndex = 0; fieldIndex < size; ++fieldIndex) {
							middle::FieldInfo field = gameState->fields[fieldIndex];
							if (field.type == middle::FieldType::Bool) {
								ImGui::Checkbox(field.name, static_cast<bool*>(field.value));
							}
							if (field.type == middle::FieldType::Int) {
								ImGui::InputInt(field.name, static_cast<int*>(field.value));
							}
							else if (field.type == middle::FieldType::Float) {
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
							else if (field.type == middle::FieldType::IdVector) {
								ImGui::Text(field.name);
								auto vector = static_cast<std::vector<middle::Id>*>(field.value);
								int size = vector->size();
								for (int index = 0; index < size; ++index) {
									ImGui::PushID((char)index);
									if(ImGui::Button("(D)")) {
										vector->erase(vector->begin() + index);
										ImGui::PopID();
										break;
									}
									ImGui::PopID();
									ImGui::SameLine();
									middle::Id* id = &(*vector)[index];
									ImGui::InputInt(std::to_string(index).c_str(), &id->index);
								}
								if (ImGui::Button("Add")) {
									vector->resize(size + 1);
								}
							}
							else {
								ImGui::Text(field.name);
							}
						}
					}
					else if (size == 0) {
						ImGui::Text(componentName);
					}


				};

				ImGui::End();
				};

			gameState->uiSetups.push_back(ui);
			});
	}
};

static middle::SystemRegistrar<ComponentUiSystem> reg("ComponentUiSystem");
