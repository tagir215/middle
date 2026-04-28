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
#include "ProcedureComponent.h"
#include "component_utils.h"
#include "UiNode.h"
#include "SnapRef.h"


class ProcedureUiSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* buttonCache;
	components::CompCache* procedureContainerCache;
	components::CompCache* procedureUseCache;
	components::CompCache* procContainerCache;
	components::CompCache* inputCache;
	components::CompCache* procedureCodeCache;
	components::CompCache* procedureImportCache;

	Color highlightColor = bubbleColors::HOVERED_ITEM;
	Color highlightColor2 = bubbleColors::PROCEDURE_SELECTED;

	void init(middle::GameState* gameState) {
		buttonCache = middle::newCompCache(gameState);
		buttonCache->addType<components::Button>();
		buttonCache->addType<components::MouseClickComponent>();
		buttonCache->addType<components::Text>();
		procedureContainerCache = middle::newCompCache(gameState);
		procedureContainerCache->addType<components::ProcedureContainer>();
		procedureCodeCache = middle::newCompCache(gameState);
		procedureCodeCache->addType<components::ProcedureComponent>();
		procedureUseCache = middle::newCompCache(gameState);
		procedureUseCache->addType<components::ProcedureUseUiTag>();
		procedureUseCache->addType<components::LoopSociety>();
		inputCache = middle::newCompCache(gameState);
		inputCache->addType<components::InputVariable>();
		inputCache->addType<components::Highlight>();
		inputCache->addType<components::Position>();
		procedureImportCache = middle::newCompCache(gameState);
		procedureImportCache->addType<components::ProcedureImportContainer>();
		procedureImportCache->addType<components::Position>();
	}

	void update(middle::GameState* gameState) override {

		auto procedureIt = procedureContainerCache->begin<components::ProcedureContainer>();
		middle::Id procContainerId;
		components::ProcedureContainer* procContainer = nullptr;
		if (procedureContainerCache->getSize() == 1) {
			procContainerId = procedureContainerCache->relevantIdVector[0];
			procContainer = *procedureIt;
		}
		middle::Id procId;
		if (procedureCodeCache->getSize() == 1) {
			procId = procedureCodeCache->relevantIdVector[0];
		}

		// buttons
		// TODO MOVE THIS OR MOVE RENDERING
		auto buttonIt = buttonCache->begin<components::Button>();
		auto buttonTextIt = buttonCache->begin<components::Text>();
		for (int i = 0; i < buttonCache->getSize(); ++i) {
			auto button = *buttonIt;
			auto text = *buttonTextIt;

			const float scrollSpeed = 50;
			if (button->function == bubbleButton::SCROLL_UP) {
				middle::moveShape(gameState, procId.index, { 0,0, -scrollSpeed });
			}
			if (button->function == bubbleButton::SCROLL_DOWN) {
				middle::moveShape(gameState, procId.index, { 0,0, scrollSpeed });
			}


			if (button->function == bubbleButton::IMPORT_PROCEDURE) {
				std::string folder = "../bubbleData/procedures/";
				std::string shapeName = text->text;

				// shape the procedure is contained into
				if (procContainerCache->getSize() > 0) {
					middle::Id& containerId = procContainerCache->relevantIdVector[0];
					auto delAction = std::make_shared<middle::CustomAction>([containerId](middle::GameState* gameState) {
						middle::deleteShapeRecursive(gameState, containerId.index);
						});
					middle::queueAction(gameState, delAction);
				}
				else {
					middle::Id loadedProcReferenceId = middle::loadShape(gameState, folder, shapeName, true, {0,0,0});

					auto& loadedProcReferenceShape = middle::getShape(gameState, loadedProcReferenceId.index);
					auto loadedPosition = middle::getComponent<components::Position>(loadedProcReferenceShape);
					auto loadedReferenceLoop = middle::getComponent<components::LoopSociety>(loadedProcReferenceShape);
					Vector3 loadedPos = { loadedPosition->posX, loadedPosition->posY, loadedPosition->posZ };

					// child of reference is the actual object
					middle::Id loadedProcId = loadedReferenceLoop->loopMemberIds[0];

					// set bubble ref to the ref import container is pointing to
					auto& procContainerShape = middle::getShape(gameState, loadedProcId.index);
					auto procContainer = middle::getComponent<components::ProcedureContainer>(procContainerShape);
					assert(procContainer);
					// Unassign bubble ref, it might have been serialized
					procContainer->bubbleRef = middle::Id();
					procContainer->editMode = false;
					middle::queueComponentDeletion<components::UiNode>(gameState, procContainerShape.id);

					// move procedure to center
					middle::Id procCompId = middle::getFirstChildWithComponent(gameState, procContainerShape.id, middle::getTypeId<components::ProcedureComponent>());
					Vector3 currentPos = middle::getShapePosition(gameState, procCompId.index);
					Vector3 targetPos = middle::getShapePosition(gameState, procedureImportCache->relevantIdVector[0].index);
					middle::moveShape(gameState, procCompId.index, targetPos - currentPos);

					// delete ui components to not have it appear on top of ui
					std::vector<middle::Id>procChildren;
					middle::getAllChildren(gameState, procCompId, procChildren);
					for (middle::Id& id : procChildren) {
						auto& childShape = middle::getShape(gameState, id.index);
						if (middle::getComponent<components::UiComponent>(childShape)) {
							middle::queueComponentDeletion<components::UiComponent>(gameState, id);
						}
					}
					break;
				}


			}

		}


		for (int i = 0; i < procedureUseCache->getSize(); ++i) {
			middle::Id procUiId = procedureUseCache->relevantIdVector[i];
			std::vector<middle::Id>procUiBlocks;
			middle::getChildren(gameState, procUiId, procUiBlocks);

			auto& procedureNames = gameState->bubbleAlgebraState.procedureNames;
			if (procedureNames.size() == 0) {
				procedureNames = middle::loadFileNamesInFolder("../bubbleData/procedures");
				for (int j = 0; j < procedureNames.size(); ++j) {
					middle::Shape shape;
					middle::addComponent<components::UiComponent>(shape);
					middle::addComponent<components::MouseIntersectable>(shape);
					middle::addComponent<components::Position>(shape);
					middle::addComponent<components::LoopTag>(shape);
					middle::addComponent<components::LoopSociety>(shape);
					auto sphere = middle::addComponent<components::Sphere>(shape);
					auto inventoryItem = middle::addComponent<components::InventoryItem>(shape);
					sphere->radius = middle::DEF_RADIUS;
					auto text = middle::addComponent<components::Text>(shape);
					auto button = middle::addComponent<components::Button>(shape);
					auto rectangle = middle::addComponent<components::Rectangle>(shape);
					text->fontColorA = 255;
					text->fontColorB = 255;
					text->text = procedureNames[j];
					text->fontSize = 15;
					rectangle->width = 100;
					rectangle->height = 50;
					// set to reference proc block in ui
					if(procUiBlocks.size() > j) {
						auto snapRef = middle::addComponent<components::SnapRef>(shape);
						snapRef->snapTargetId = procUiBlocks[j];
						button->function = bubbleButton::IMPORT_PROCEDURE;
						middle::Shape& registeredShape = middle::registerAsGhostShape(gameState, shape);
					}
				}
			}
		}

		if (procedureContainerCache->getSize() > 0) {
			// execution iterator rendering
			bool procedureInAction = procContainer && procContainer->procedureTransitionStack.size() > 0;
			if (procContainer && procContainer->activeBlock.index != middle::UNASSIGNED && procedureInAction) {
				auto& activeBlockShape = middle::getShape(gameState, procContainer->activeBlock.index);
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

	}
};

static middle::SystemRegistrar<ProcedureUiSystem> reg("ProcedureUiSystem");
