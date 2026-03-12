#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MouseSelectable.h"
#include "imgui.h"
#include "editor_actions.h"
#include <string>
#include <misc/cpp/imgui_stdlib.cpp>
#include <Position.h>
#include "Text.h"
#include "Rotation.h"

class ComponentUiSystem : public middle::MiddleGameplaySystem {

public:
	ComponentUiSystem() {
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
		systemModeType = middle::SystemModeType::EDITOR;
	}

	components::CompCache* cache;

	void init(middle::GameState* gameState) {
		cache = middle::newCompCache(gameState);
		cache->addType<components::MouseSelectable>();
	}


	const int maxFieldCount = 100;

	void update(middle::GameState* gameState) override {
		if (gameState->fields.size() < maxFieldCount) {
			gameState->fields.resize(maxFieldCount);
		}

		if (gameState->editorState.selectCount != 1) {
			return;
		}

		auto selectableIt = cache->begin<components::MouseSelectable>();
		for (int i = 0; i < cache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, cache->relevantIdVector[i].index);
			auto selectable = *selectableIt;

			if (!selectable->selected) {
				continue;
			}

			auto ui = [&shape, gameState]() {
				ImGui::Begin("ComponentEditor");
				std::string idText = "index_" + std::to_string(shape.id.index) + "_gen_" + std::to_string(shape.id.generation);
				ImGui::Text(idText.c_str());

				if (ImGui::IsWindowHovered()) {
					gameState->inputBlockers.insert(middle::InputBlockers::KEYBOARD_BLOCK);
				}

				for (auto& pair : shape.componentMap) {
					int typeId = pair.first;
					int componentOffset = pair.second.componentOffset;
					middle::Serializable* serializable = middle::componentListMap[typeId]->getSerializable(componentOffset);
					std::string typeData;
					const char* componentName = middle::componentNameMap[typeId].c_str();
					ImGui::Separator();
					ImGui::PushID((char)pair.first);
					if (ImGui::Button("(o)")) {
						middle::queueAction(gameState, std::make_shared<middle::EditorActionOpenComponent>(componentName));
					}
					ImGui::SameLine();
					if (ImGui::Button("(d)")) {
						middle::queueAction(gameState, std::make_shared<middle::EditorActionRemoveComponent>(componentName, middle::getSelectedShapes(gameState)));
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
								ImGui::InputText(field.name, string);
							}
							else if (field.type == middle::FieldType::Vector3) {
								Vector3* vector = static_cast<Vector3*>(field.value);
								ImGui::Text(field.name);
								ImGui::PushID("x");
								ImGui::PushID("y");
								ImGui::PushID("z");
								ImGui::InputFloat("x", &vector->x);
								ImGui::InputFloat("y", &vector->y);
								ImGui::InputFloat("z", &vector->z);
								ImGui::PopID();
								ImGui::PopID();
								ImGui::PopID();
							}
							else if (field.type == middle::FieldType::Vector2) {
								Vector2* vector = static_cast<Vector2*>(field.value);
								ImGui::Text(field.name);
								ImGui::PushID("x");
								ImGui::PushID("y");
								ImGui::InputFloat("x", &vector->x);
								ImGui::InputFloat("y", &vector->y);
								ImGui::PopID();
								ImGui::PopID();
							}
							else if (field.type == middle::FieldType::Quaternion) {
								Quaternion* quat = static_cast<Quaternion*>(field.value);
								ImGui::Text(field.name);
								ImGui::PushID("x");
								ImGui::PushID("y");
								ImGui::PushID("z");
								ImGui::PushID("2");
								ImGui::InputFloat("x", &quat->x);
								ImGui::InputFloat("y", &quat->y);
								ImGui::InputFloat("z", &quat->z);
								ImGui::InputFloat("w", &quat->w);
								ImGui::PopID();
								ImGui::PopID();
								ImGui::PopID();
								ImGui::PopID();
								auto position = middle::getComponent<components::Position>(shape);
								if (position) {
									Vector3 forward = Vector3RotateByQuaternion(middle::ROTATION_FORWARD, *quat);
								}
							}
							else if (field.type == middle::FieldType::Color) {
								Color* color = static_cast<Color*>(field.value);
								ImGui::Text(field.name);
								int r = static_cast<int>(color->r);
								int g = static_cast<int>(color->g);
								int b = static_cast<int>(color->b);
								int a = static_cast<int>(color->a);
								ImGui::SliderInt("r", &r, 0, 255);
								ImGui::SliderInt("g", &g, 0, 255);
								ImGui::SliderInt("b", &b, 0, 255);
								ImGui::SliderInt("a", &a, 0, 255);
								color->r = static_cast<unsigned char>(r);
								color->g = static_cast<unsigned char>(g);
								color->b = static_cast<unsigned char>(b);
								color->a = static_cast<unsigned char>(a);
							}
							else if (field.type == middle::FieldType::IdVector) {
								ImGui::Text(field.name);
								auto vector = static_cast<std::vector<middle::Id>*>(field.value);
								int size = vector->size();
								for (int index = 0; index < size; ++index) {
									ImGui::PushID((char)index);
									if (ImGui::Button("(D)")) {
										vector->erase(vector->begin() + index);
										ImGui::PopID();
										break;
									}
									ImGui::PopID();
									ImGui::SameLine();
									middle::Id* id = &(*vector)[index];
									ImGui::InputInt(std::to_string(index).c_str(), &id->index);
									ImGui::SameLine();
									ImGui::Text(std::to_string(id->generation).c_str());
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

				ImGui::Separator();

				if (ImGui::Button("Save")) {

					// Popup for entering new scene name
					static char inputtedName[128] = ""; // buffer for scene name input

					ImGui::Text("Save");

					auto text = middle::getComponent<components::Text>(shape);
					std::string name;
					if (text) {
						name = text->text;
					}
					else {
						name = "s" + std::to_string(shape.id.index) + "_" + std::to_string(shape.id.generation);
					}


					auto pos = middle::getComponent<components::Position>(shape);
					Vector3 displacement = { 0,0,0 };
					if (pos) {
						displacement = { pos->posX, pos->posY, pos->posZ };
						// move shape to origin
						middle::moveShape(gameState, shape.id.index, Vector3Negate(displacement));
					}

					middle::resetGenerations(gameState);
					middle::saveShape(gameState, shape.id, "../assets/shapes/", name);

					// move shape back after save
					middle::moveShape(gameState, shape.id.index, displacement);

				}


				ImGui::End();
				};

			gameState->uiSetups.push_back(ui);
		}
	}
};

static middle::SystemRegistrar<ComponentUiSystem> reg("ComponentUiSystem");
