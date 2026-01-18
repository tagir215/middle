#pragma once
#include "game_state.h"
#include "registrars.h"
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
							ImGui::Text(gameState->fields[fieldIndex].name);
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
