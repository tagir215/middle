#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "imgui.h"
#include "middle_shape_utils.h"
#include "bubble_utils.h"
#include "bubble_actions.h"
#include "MouseSelectable.h"
#include "UiComponent.h"


class BubbleLevelEditorSystem : public middle::MiddleGameplaySystem {
public:
	BubbleLevelEditorSystem() {
		systemModeType = middle::SystemModeType::EDITOR;
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
	}

	components::CompCache* selectableCache = nullptr;

	void init(middle::GameState* gameState) override {
		selectableCache = middle::newCompCache(gameState);
		selectableCache->addType<components::MouseSelectable>();
		selectableCache->addType<components::UiComponent>(components::NOTINTERESTED);
	}

	middle::Id getSelected() {
		auto selectableIt = selectableCache->begin<components::MouseSelectable>();
		for (int i = 0; i < selectableCache->getSize(); ++i) {
			auto selectable = *selectableIt;
			if (selectable->selected) {
				return selectableCache->relevantIdVector[i];
			}
		}
		return middle::Id();
	}

	void update(middle::GameState* gameState) override {
		auto ui = [gameState, this]() 
		{
			ImGui::Begin("BubbleEditor");

			middle::Id selectedId = getSelected();

			bool somethingSelected = selectedId.index != middle::UNASSIGNED;

			if (!somethingSelected) {
				if (ImGui::Button("NewBubble")) {
					middle::unselect(gameState);
					auto bubbleProto = bubble::newBubble(gameState, { 0,0,0 });
					auto& bubbleShape = middle::registerShape(gameState, bubbleProto);
					auto selectable = middle::getComponent<components::MouseSelectable>(bubbleShape);
					selectable->selected = true;
				}
			}


			if (somethingSelected) {

				if (ImGui::Button("New Bubble")) {
					auto bubbleProto = bubble::newBubble(gameState, { 0,0,0 });
					auto& bubbleShape = middle::registerShape(gameState, bubbleProto);
					middle::Id newId = bubbleShape.id;
					middle::EditorActionReparent(selectedId.index, newId.index).execute(gameState);
				}
				if (ImGui::Button("New Unit")) {
					Vector3 containerPos = middle::getShapePosition(gameState, selectedId.index);
					auto unitProto = bubble::newUnit(gameState, containerPos);
					auto& unit = middle::registerShape(gameState, unitProto);
					middle::Id newId = unit.id;
					middle::EditorActionReparent(selectedId.index, newId.index).execute(gameState);
				}
				if (ImGui::Button("New Variable")) {
					static char* label;
					ImGui::InputText("variable lable", label, 20);
					Vector3 containerPos = middle::getShapePosition(gameState, selectedId.index);
					auto unitProto = bubble::newVariable(gameState, label, containerPos);
					auto& unit = middle::registerShape(gameState, unitProto);
					middle::Id newId = unit.id;
					middle::EditorActionReparent(selectedId.index, newId.index).execute(gameState);
				}
				if (ImGui::Button("New Multiplication")) {
					Vector3 containerPos = middle::getShapePosition(gameState, selectedId.index);
					float xOffset = 20;
					Vector3 offsetVec = { xOffset, 0,0 };
					auto bubbleAProto = bubble::newBubble(gameState, containerPos + offsetVec);
					auto bubbleBProto = bubble::newBubble(gameState, containerPos + Vector3Negate(offsetVec));
					auto& bubbleA = middle::registerShape(gameState, bubbleAProto);
					auto& bubbleB = middle::registerShape(gameState, bubbleBProto);
					auto mulAction = bubbleActions::NewMultiplication(bubbleA.id, bubbleB.id);
					mulAction.execute(gameState);
					middle::Id newId = mulAction.resultShapeId;
					middle::EditorActionReparent(selectedId.index, newId.index).execute(gameState);
				}
				if (ImGui::Button("To Fraciton")) {
					static int dividend = 2;
					ImGui::SliderInt("dividend", &dividend, 2, 10);
					Vector3 containerPos = middle::getShapePosition(gameState, selectedId.index);
					middle::Id fraction = bubble::shapeToFraction(gameState, selectedId, containerPos, dividend);
					bubbleActions::Replace(selectedId, fraction).execute(gameState);
				}

			}

			ImGui::End();
			};

		gameState->uiSetups.push_back(ui);
	}
};

static middle::SystemRegistrar<BubbleLevelEditorSystem> reg("BubbleLevelEditorSystem");
