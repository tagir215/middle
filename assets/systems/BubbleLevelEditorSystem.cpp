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
#include "ExponentComponent.h"
#include "BubbleAlgebraLevelConfigs.h"
#include "JointEntity.h"
#include "Inventory.h"
#include "InventorySlot.h"
#include "InventoryItem.h"
#include "BubbleVariable.h"
#include "Reference.h"
#include "CameraComponent.h"


class BubbleLevelEditorSystem : public middle::MiddleGameplaySystem {
public:
	BubbleLevelEditorSystem() {
		systemModeType = middle::SystemModeType::EDITOR;
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
	}

	components::CompCache* selectableCache = nullptr;
	components::CompCache* inventorySlotCache = nullptr;
	components::CompCache* refCache = nullptr;

	void init(middle::GameState* gameState) override {
		selectableCache = middle::newCompCache(gameState);
		selectableCache->addType<components::MouseSelectable>();
		selectableCache->addType<components::UiComponent>(components::NOTINTERESTED);
		inventorySlotCache = middle::newCompCache(gameState);
		inventorySlotCache->addType <components::InventorySlot>();
		refCache = middle::newCompCache(gameState);
		refCache->addType<components::Reference>();
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

					if (ImGui::Button("Save Camera Pos")) {
						for (middle::Id& id : refCache->relevantIdVector) {
							middle::Id cameraId = middle::getFirstChildWithComponent(gameState, id, middle::getTypeId<components::CameraComponent>());
							if (cameraId.index != middle::UNASSIGNED) {
								auto& camShape = middle::getShape(gameState, cameraId.index);
								Vector3 targetPos = middle::getShapePosition(gameState, cameraId.index);
								Vector3 currPos = middle::getShapePosition(gameState, id.index);
								middle::moveShape(gameState, id.index, targetPos - currPos);
							}
						}
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

					if (ImGui::Button("Create Configs")) {
						int freeIndex = middle::findFreeIndex(gameState);
						entities::initJoint(gameState, freeIndex, { 200,0,0 });
						auto& shape = middle::getShape(gameState, freeIndex);
						middle::attachComponent<components::BubbleAlgebraLevelConfigs>(gameState, shape.id);
						auto sphere = middle::getComponent<components::Sphere>(shape);
						sphere->radius = 10;
					}

					if (ImGui::Button("Create Inventory")) {
						middle::Shape inventoryProto;
						middle::addComponent<components::LoopTag>(inventoryProto);
						middle::addComponent<components::LoopSociety>(inventoryProto);
						auto position = middle::addComponent<components::Position>(inventoryProto);
						middle::addComponent<components::Inventory>(inventoryProto);
						auto selectable = middle::addComponent<components::MouseSelectable>(inventoryProto);
						middle::addComponent<components::MouseGrabbable>(inventoryProto);
						middle::addComponent<components::MouseIntersectable>(inventoryProto);
						Vector3 initPos = { 0,0,-50 };
						position->posX = initPos.x;
						position->posY = initPos.y;
						position->posZ = initPos.z;
						selectable->selected = true;
						middle::Shape& inventoryShape = middle::registerShape(gameState, inventoryProto);

						// setup placeholder inventory items
						int inventorySize = inventorySlotCache->getSize();
						const float zOffset = -20;
						const float xDist = 100;
						for (int i = 0; i < inventorySize; ++i) {
							auto bubble = bubble::newBubble(gameState, initPos + Vector3{xDist* i + 1, 0, 0});
							auto& bubbleShape = middle::registerShape(gameState, bubble);
							auto inventoryItem = middle::attachComponent<components::InventoryItem>(gameState, bubbleShape.id);
							inventoryItem->itemType = bubbleInventoryItemType::NEW_ADDITION_TERM;
							//inventoryItem->idRef = inventorySlotCache->relevantIdVector[i];
							middle::EditorActionReparent(inventoryShape.id.index, bubbleShape.id.index).execute(gameState);
						}
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
					if (ImGui::Button("New Multiplication")) {
						Vector3 containerPos = middle::getShapePosition(gameState, selectedId.index);
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
					if (ImGui::Button("New Power")) {
						Vector3 containerPos = middle::getShapePosition(gameState, selectedId.index);
						auto containerProto = bubble::newBubble(gameState, containerPos);
						auto bubbleAProto = bubble::newBubble(gameState, containerPos + randomOffset());
						auto bubbleBProto = bubble::newBubble(gameState, containerPos + randomOffset());
						auto& bubbleA = middle::registerShape(gameState, bubbleAProto);
						auto& bubbleB = middle::registerShape(gameState, bubbleBProto);
						auto mulAction = bubbleActions::NewMultiplication(bubbleA.id, bubbleB.id);
						mulAction.execute(gameState);
						middle::Id newId = mulAction.resultShapeId;
						auto& newShape = middle::getShape(gameState, newId.index);
						auto mul = middle::getComponent<components::BubbleMultiplyComponent>(newShape);
						mul->operationType = static_cast<int>(components::OperationType::POWER);
						middle::EditorActionReparent(selectedId.index, newId.index).execute(gameState);
					}
					ImGui::Separator();
					static char label[20] = "x";
					ImGui::InputText("variable lable", label, IM_ARRAYSIZE(label));
					if (ImGui::Button("Bubble To Variable")) {
						std::string string(label);
						auto var = middle::attachComponent<components::BubbleVariable>(gameState, selectedId);
						var->label = string;
					}
					ImGui::Separator();
					static int dividend = 2;
					if (ImGui::Button("Bubble To Inverse")) {
						middle::Shape& selectedShape = middle::getShape(gameState, selectedId.index);
						auto bubble = middle::getComponent<components::BubbleComponent>(selectedShape);
						bubble->inverse = true;
					}
					ImGui::Separator();
				}

				ImGui::End();
			};

		gameState->uiSetups.push_back(ui);
	}
};

static middle::SystemRegistrar<BubbleLevelEditorSystem> reg("BubbleLevelEditorSystem");
