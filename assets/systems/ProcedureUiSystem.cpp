#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "Button.h"
#include "middle_shape_utils.h"
#include "bubble_algebra_buttons.h"
#include "MouseIntersectable.h"
#include "editor_file_utils.h"
#include "ProcedureContainer.h"
#include "PlacementComponent.h"
#include "MouseClickComponent.h"
#include "Rectangle.h"
#include "ProcedureUseUiTag.h"
#include "LoopSociety.h"
#include "UiComponent.h"
#include "Position.h"
#include "Text.h"
#include "InventoryItem.h"
#include "editor_actions.h"
#include "Sphere.h"
#include "ProcedureImportContainer.h"
#include "bubble_utils.h"
#include "InputVariable.h"
#include "Highlight.h"
#include "Circle.h"
#include "ProcedureInputVariable.h"
#include "bubble_colors.h"



class ProcedureUiSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* buttonCache;
	components::CompCache* procedureCache;
	components::CompCache* procedureUseCache;
	components::CompCache* procedureImportContainerCache;
	components::CompCache* inputCache;
	std::vector<std::string>procedureNames;

	Color highlightColor = bubbleColors::HOVERED_ITEM;
	Color highlightColor2 = bubbleColors::PROCEDURE_SELECTED;

	void init(middle::GameState* gameState) {
		buttonCache = middle::newCompCache(gameState);
		buttonCache->addType<components::Button>();
		buttonCache->addType<components::MouseClickComponent>();
		buttonCache->addType<components::Text>();
		procedureCache = middle::newCompCache(gameState);
		procedureCache->addType<components::ProcedureContainer>();
		procedureUseCache = middle::newCompCache(gameState);
		procedureUseCache->addType<components::ProcedureUseUiTag>();
		procedureUseCache->addType<components::LoopSociety>();
		procedureImportContainerCache = middle::newCompCache(gameState);
		procedureImportContainerCache->addType<components::ProcedureImportContainer>();
		inputCache = middle::newCompCache(gameState);
		inputCache->addType<components::InputVariable>();
		inputCache->addType<components::Highlight>();
		inputCache->addType<components::Position>();
	}

	void update(middle::GameState* gameState) override {

		auto procedureIt = procedureCache->begin<components::ProcedureContainer>();
		middle::Id procId;
		components::ProcedureContainer* procComp = nullptr;
		if (procedureCache->getSize() == 1) {
			procId = procedureCache->relevantIdVector[0];
			procComp = *procedureIt;
		}

		// buttons
		// TODO MOVE THIS OR MOVE RENDERING
		auto buttonIt = buttonCache->begin<components::Button>();
		auto buttonTextIt = buttonCache->begin<components::Text>();
		for (int i = 0; i < buttonCache->getSize(); ++i) {
			auto button = *buttonIt;
			auto text = *buttonTextIt;
			if (button->function == bubbleButton::SAVE_BUTTON) {
				middle::saveShape(gameState, procId, "../bubbleData/procedures/", text->text);
			}
			if (button->function == bubbleButton::IMPORT_PROCEDURE) {
				std::string folder = "../bubbleData/procedures/";
				std::string shapeName = text->text;

				// shape the procedure is contained into
				middle::Id& containerId = procedureImportContainerCache->relevantIdVector[0];
				auto& containerShape = middle::getShape(gameState, containerId.index);
				auto procImportContainer = middle::getComponent<components::ProcedureImportContainer>(containerShape);

				if (procImportContainer->loadedProcedureName == shapeName) {
					auto containerLoop = middle::getComponent<components::LoopSociety>(containerShape);
					middle::Id currentLoadedProcedureId = containerLoop->loopMemberIds[0];
					middle::queueAction(gameState, std::make_shared<middle::EditorActionDeleteSingle>(currentLoadedProcedureId));
					procImportContainer->loadedProcedureName = "";
				}
				else {
					middle::Id loadedProcReferenceId = middle::loadShape(gameState, folder, shapeName, true, {0,0,0});
					// update pointer because loading shape messes component arrays
					procImportContainer = middle::getComponent<components::ProcedureImportContainer>(containerShape);

					auto containerLoop = middle::getComponent<components::LoopSociety>(containerShape);
					auto containerPos = middle::getComponent<components::Position>(containerShape);

					Vector3 targetPos = { containerPos->posX, containerPos->posY, containerPos->posZ };
					auto& loadedProcReferenceShape = middle::getShape(gameState, loadedProcReferenceId.index);
					auto loadedPosition = middle::getComponent<components::Position>(loadedProcReferenceShape);
					auto loadedReferenceLoop = middle::getComponent<components::LoopSociety>(loadedProcReferenceShape);
					Vector3 loadedPos = { loadedPosition->posX, loadedPosition->posY, loadedPosition->posZ };
					procImportContainer->loadedProcedureName = shapeName;

					// remove current procedure if it exists
					if (containerLoop->loopMemberIds.size() > 0) {
						middle::Id currentLoadedProcedureId = containerLoop->loopMemberIds[0];
						middle::queueAction(gameState, std::make_shared<middle::EditorActionDeleteSingle>(currentLoadedProcedureId));
					}

					middle::queueAction(gameState, std::make_shared<middle::EditorActionReparent>(containerId.index, loadedProcReferenceId.index));
					middle::moveShape(gameState, loadedProcReferenceId.index, Vector3Subtract(targetPos, loadedPos));

					// child of reference is the actual object
					middle::Id loadedProcId = loadedReferenceLoop->loopMemberIds[0];

					// set bubble ref to the ref import container is pointing to
					auto& procContainerShape = middle::getShape(gameState, loadedProcId.index);
					auto procContainer = middle::getComponent<components::ProcedureContainer>(procContainerShape);
					assert(procContainer);
					procContainer->bubbleRef = procImportContainer->bubbleRef;
					// Unassign bubble ref, it might have been serialized
					procContainer->bubbleRef = middle::Id();

					// resize the container to fit the procedure
					middle::queueAction(gameState, std::make_shared<middle::CustomAction>([loadedProcId, containerId](middle::GameState* gameState) {
						float left, right, bottom, top;
						bubble::loopChildrenOnlyRectBoundingBox(gameState, loadedProcId, &left, &right, &bottom, &top);
						auto& procContainer = middle::getShape(gameState, containerId.index);
						auto containerRect = middle::getComponent<components::Rectangle>(procContainer);
						containerRect->width = right - left;
						containerRect->height = top - bottom;
						}));
				}


				procComp = nullptr;

			}

		}

		if (procedureCache->getSize() > 0) {
			// execution iterator rendering
			bool procedureInAction = procComp->procedureTransitionStack.size() > 0;
			if (procComp && procComp->activeBlock.index != middle::UNASSIGNED && procedureInAction) {
				auto& activeBlockShape = middle::getShape(gameState, procComp->activeBlock.index);
				Vector3 position = middle::getShapePosition(gameState, activeBlockShape.id.index);
				auto rect = middle::getComponent<components::Rectangle>(activeBlockShape);
				middle::RenderItem activeBlockItem;
				activeBlockItem.type = middle::RenderItemType::RECTANGLE;
				activeBlockItem.backgroundColor = highlightColor;
				activeBlockItem.width = rect->width;
				activeBlockItem.height = rect->height;
				activeBlockItem.length = 0.2f;
				activeBlockItem.center = position;
				gameState->renderData.push_back(activeBlockItem);
			}
		}

		for (int i = 0; i < procedureUseCache->getSize(); ++i) {
			middle::Id procUiId = procedureUseCache->relevantIdVector[i];
			if (procedureNames.size() == 0) {
				procedureNames = middle::loadFileNamesInFolder("../bubbleData/procedures");
				for (int j = 0; j < procedureNames.size(); ++j) {
					middle::Shape shape;
					middle::addComponent<components::UiComponent>(shape);
					middle::addComponent<components::MouseIntersectable>(shape);
					middle::addComponent<components::Position>(shape);
					middle::addComponent<components::InventoryItem>(shape);
					middle::addComponent<components::LoopSociety>(shape);
					auto sphere = middle::addComponent<components::Sphere>(shape);
					sphere->radius = middle::DEF_RADIUS;
					auto text = middle::addComponent<components::Text>(shape);
					auto button = middle::addComponent<components::Button>(shape);
					auto rectangle = middle::addComponent<components::Rectangle>(shape);
					text->text = procedureNames[j];
					rectangle->width = 100;
					rectangle->height = 50;
					button->function = bubbleButton::IMPORT_PROCEDURE;
					middle::Shape& registeredShape = middle::registerAsGhostShape(gameState, shape);
					middle::queueAction(gameState, std::make_shared<middle::EditorActionReparent>(procUiId.index, registeredShape.id.index));
				}
			}
		}


		// render highlighted inputs
		auto inputPosIt = inputCache->begin<components::Position>();
		auto inputIt = inputCache->begin<components::InputVariable>();
		for (int i = 0; i < inputCache->getSize(); ++i) {
			auto position = *inputPosIt;
			auto input = *inputIt;
			auto& shape = middle::getShape(gameState, inputCache->relevantIdVector[i].index);
			middle::RenderItem item;
			item.type = middle::RenderItemType::CYLINDER;
			item.center = { position->posX, position->posY, position->posZ };
			item.radius = 4;
			item.ringRadius = 4;
			item.length = 0.1f;
			item.color = highlightColor;
			gameState->renderData.push_back(item);

			// highlight reference bubble if it exists at the moment
			if (input->unitRef.index != middle::UNASSIGNED) {
				if (!middle::isShapeAlive(gameState, input->unitRef.index)) {
					continue;
				}
				// todo refactor to always check generation
				middle::Id id = gameState->ids[input->unitRef.index];
				if (id != input->unitRef) {
					continue;
				}
				middle::Shape& refShape = middle::getShape(gameState, input->unitRef.index);
				if (refShape.componentMap.size() == 0) {
					continue;
				}
				Vector3 pos = middle::getShapePosition(gameState, input->unitRef.index);
				auto circle = middle::getComponent<components::Circle>(refShape);


				Color color = highlightColor;

				// highlight procedure refs with different color
				auto procInput = middle::getComponent<components::ProcedureInputVariable>(shape);
				if (procInput) {
					color = highlightColor2;
				}

				middle::RenderItem inputHighlight;
				inputHighlight.type = middle::RenderItemType::CYLINDER;
				inputHighlight.center = pos;
				inputHighlight.color = color;
				inputHighlight.radius = circle->radius;
				inputHighlight.ringRadius = circle->radius;
				inputHighlight.length = 0.1f;
				gameState->renderData.push_back(inputHighlight);
			}

		}
	}
};

static middle::SystemRegistrar<ProcedureUiSystem> reg("ProcedureUiSystem");
