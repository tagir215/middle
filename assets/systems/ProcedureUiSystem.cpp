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


class ProcedureUiSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* buttonCache;
	components::CompCache* procedureCache;
	components::CompCache* procedureUseCache;
	components::CompCache* procedureImportContainerCache;
	std::vector<std::string>procedureNames;

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
				Vector3 mouseXZ = gameState->input.mouseXZ_PlanePos;
				middle::Id loadedProcId = middle::loadShape(gameState, folder, shapeName, true, mouseXZ);
				auto& procedureShape = middle::getShape(gameState, loadedProcId.index);
				auto loadedPosition = middle::getComponent<components::Position>(procedureShape);
				Vector3 loadedPos = { loadedPosition->posX, loadedPosition->posY, loadedPosition->posZ };

				// shape the procedure is contained into
				middle::Id& containerId = procedureImportContainerCache->relevantIdVector[0];
				auto& containerShape = middle::getShape(gameState, containerId.index);
				auto containerLoop = middle::getComponent<components::LoopSociety>(containerShape);
				auto containerPos = middle::getComponent<components::Position>(containerShape);
				Vector3 targetPos = { containerPos->posX, containerPos->posY, containerPos->posZ };
				if (containerLoop->loopMemberIds.size() > 0) {
					middle::queueAction(gameState, std::make_shared<middle::EditorActionDeleteSingle>(containerId));
				}
				middle::queueAction(gameState, std::make_shared<middle::EditorActionReparent>(containerId.index, loadedProcId.index));
				middle::moveShape(gameState, loadedProcId.index, Vector3Subtract(targetPos, loadedPos));
			}

		}

		// execution iterator rendering
		if (procComp && procComp->activeBlock.index != middle::UNASSIGNED) {
			auto& activeBlockShape = middle::getShape(gameState, procComp->activeBlock.index);
			Vector3 position = middle::getShapePosition(gameState, activeBlockShape.id.index);
			auto rect = middle::getComponent<components::Rectangle>(activeBlockShape);
			middle::RenderItem activeBlockItem;
			activeBlockItem.type = middle::RenderItemType::RECTANGLE;
			Color color = { GREEN.r, GREEN.g, GREEN.b, 0.2f };
			activeBlockItem.color = GREEN;
			activeBlockItem.width = rect->width;
			activeBlockItem.height = rect->height;
			activeBlockItem.length = 0.2f;
			activeBlockItem.center = { 0,0,0 };
			Transform transform = {
				position, {0,0,0,0}, {1,1,1}
			};
			activeBlockItem.transform = transform;
			gameState->renderData.push_back(activeBlockItem);
		}

		auto procedureUseAiIt = procedureUseCache->begin<components::LoopSociety>();
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

	}
};

static middle::SystemRegistrar<ProcedureUiSystem> reg("ProcedureUiSystem");
