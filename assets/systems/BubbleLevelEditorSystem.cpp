#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "imgui.h"
#include "middle_shape_utils.h"
#include "bubble_utils.h"
#include "bubble_actions.h"
#include "MouseSelectable.h"
#include "UiComponent.h"
#include "component_utils.h"
#include "BubbleAlgebraProblem.h"
#include "BubbleRootComponent.h"


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

	Vector3 randomOffset() {
		float randomX = (std::rand() % 50 + 1);
		float randomZ = (std::rand() % 50 + 1);
		return { 1.0f / randomX, 0, 1.0f / randomZ };
	}

	void update(middle::GameState* gameState) override {
		auto ui = [gameState, this]()
			{

				ImGui::Begin("BubbleEditor");

				if (ImGui::IsWindowHovered()) {
					gameState->inputBlockers.insert(middle::InputBlockers::KEYBOARD_BLOCK);
				}


				middle::Id selectedId = getSelected();

				bool somethingSelected = selectedId.index != middle::UNASSIGNED;

				if (!somethingSelected) {
					if (ImGui::Button("New Bubble")) {
						middle::unselect(gameState);
						auto bubbleProto = bubble::newBubble(gameState, { 0,0,0 });
						auto& bubbleShape = middle::registerShape(gameState, bubbleProto);
						middle::attachComponent<components::BubbleAlgebraProblem>(gameState, bubbleShape.id);
						auto selectable = middle::getComponent<components::MouseSelectable>(bubbleShape);
						selectable->selected = true;
					}

					if (ImGui::Button("New Equals")) {
						middle::unselect(gameState);
						auto bubbleProtoA = bubble::newBubble(gameState, randomOffset());
						auto& bubbleShapeA = middle::registerShape(gameState, bubbleProtoA);
						auto bubbleProtoB = bubble::newBubble(gameState, randomOffset());
						auto& bubbleShapeB = middle::registerShape(gameState, bubbleProtoB);
						middle::Id equals = bubble::newEquals(gameState, bubbleShapeA.id, bubbleShapeB.id, { 0,0,0 });
						middle::attachComponent<components::BubbleAlgebraProblem>(gameState, equals);
					}

					if (ImGui::Button("Import Bubble Level Content")) {
						const std::string folder = "../assets/shapes/";
						middle::loadShape(gameState, folder, "BubbleGameplaySystems", true, { 400,0,-500 });
						middle::loadShape(gameState, folder, "ManipulationSystems", true, { 800,0,-500 });
						middle::loadShape(gameState, folder, "GreatWallAndProcedureAndProblemContainers", true, { 400,0,0 });
						middle::loadShape(gameState, folder, "BubbleAlgebraUi", true, { -800,0,-500 });
						Vector3 cameraPos = { 0,-1000,0 };
						middle::loadShape(gameState, folder, "BubbleCamera", true, cameraPos);
					}

					if (ImGui::Button("Import Procedure Level Content")) {
						const std::string folder = "../assets/shapes/";
						middle::loadShape(gameState, folder, "BubbleGameplaySystems", true, { 400,0,-500 });
						middle::loadShape(gameState, folder, "ProcedureContainer", true, { 300,0,-0200 });
						middle::loadShape(gameState, folder, "ProcedureVisualizationSystems", true, { 1000,0,0 });
						middle::loadShape(gameState, folder, "GreatWallAndProcedureAndProblemContainers", true, { 400,0,0 });
						middle::loadShape(gameState, folder, "ProcedureUI", true, { 0,800,0 });
						Vector3 cameraPos = { 0,-1000,0 };
						middle::loadShape(gameState, folder, "BubbleCamera", true, cameraPos);
					}
				}


				if (somethingSelected) {

					if (ImGui::Button("New Bubble")) {
						Vector3 containerPos = middle::getShapePosition(gameState, selectedId.index);
						auto bubbleProto = bubble::newBubble(gameState, containerPos + randomOffset());
						auto& bubbleShape = middle::registerShape(gameState, bubbleProto);
						middle::Id newId = bubbleShape.id;
						middle::EditorActionReparent(selectedId.index, newId.index).execute(gameState);
					}
					ImGui::Separator();
					if (ImGui::Button("New Unit")) {
						Vector3 containerPos = middle::getShapePosition(gameState, selectedId.index);
						auto unitProto = bubble::newUnit(gameState, containerPos + randomOffset());
						auto& unit = middle::registerShape(gameState, unitProto);
						middle::Id newId = unit.id;
						middle::EditorActionReparent(selectedId.index, newId.index).execute(gameState);
					}
					ImGui::Separator();
					static char label[20] = "x";
					ImGui::InputText("variable lable", label, IM_ARRAYSIZE(label));
					if (ImGui::Button("New Variable")) {
						Vector3 containerPos = middle::getShapePosition(gameState, selectedId.index);
						std::string string(label);
						auto unitProto = bubble::newVariable(gameState, string, containerPos + randomOffset());
						auto& unit = middle::registerShape(gameState, unitProto);
						middle::Id newId = unit.id;
						middle::EditorActionReparent(selectedId.index, newId.index).execute(gameState);
					}
					ImGui::Separator();
					if (ImGui::Button("New Multiplication")) {
						Vector3 containerPos = middle::getShapePosition(gameState, selectedId.index);
						float xOffset = 20;
						auto bubbleAProto = bubble::newBubble(gameState, containerPos + randomOffset());
						auto bubbleBProto = bubble::newBubble(gameState, containerPos + randomOffset());
						auto& bubbleA = middle::registerShape(gameState, bubbleAProto);
						auto& bubbleB = middle::registerShape(gameState, bubbleBProto);
						auto mulAction = bubbleActions::NewMultiplication(bubbleA.id, bubbleB.id);
						mulAction.execute(gameState);
						middle::Id newId = mulAction.resultShapeId;
						middle::EditorActionReparent(selectedId.index, newId.index).execute(gameState);
					}
					ImGui::Separator();
					if (ImGui::Button("Link Multiplication")) {
						Vector3 containerPos = middle::getShapePosition(gameState, selectedId.index);
						auto bubbleProto = bubble::newBubble(gameState, containerPos + randomOffset());
						auto& bubble = middle::registerShape(gameState, bubbleProto);
						auto linkAction = bubbleActions::LinkMultiplicationTerm(selectedId, bubble.id);
						linkAction.execute(gameState);
					}
					ImGui::Separator();
					static int dividend = 2;
					ImGui::SliderInt("dividend", &dividend, 2, 10);
					if (ImGui::Button("To Fraciton")) {
						Vector3 containerPos = middle::getShapePosition(gameState, selectedId.index);
						middle::Id fraction = bubble::shapeToFraction(gameState, selectedId, containerPos, dividend);
						bubbleActions::Replace(selectedId, fraction).execute(gameState);
					}
					ImGui::Separator();
					static bool isInverse = false;
					static int power = 1;
					ImGui::SliderInt("power", &power, -4, 4);
					ImGui::Checkbox("isInverse", &isInverse);
					if (ImGui::Button("New Power")) {
						Vector3 containerPos = middle::getShapePosition(gameState, selectedId.index);
						middle::Shape& powerShape = middle::registerShape(gameState, bubble::newPower(gameState, containerPos + randomOffset()));
						auto powerComp = middle::getComponent<components::BubbleRootComponent>(powerShape);
						powerComp->power = power;
						powerComp->isInverse = isInverse;
						middle::EditorActionReparent(selectedId.index, powerShape.id.index).execute(gameState);
					}
				}

				ImGui::End();
			};

		gameState->uiSetups.push_back(ui);
	}
};

static middle::SystemRegistrar<BubbleLevelEditorSystem> reg("BubbleLevelEditorSystem");
