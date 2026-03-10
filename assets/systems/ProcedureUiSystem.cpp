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

class ProcedureUiSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* buttonCache;
	components::CompCache* procedureCache;

	void init(middle::GameState* gameState) {
		buttonCache = middle::newCompCache(gameState);
		buttonCache->addType<components::Button>();
		buttonCache->addType<components::MouseClickComponent>();
		procedureCache = middle::newCompCache(gameState);
		procedureCache->addType<components::ProcedureContainer>();
	}

	void update(middle::GameState* gameState) override {

		auto procedureIt = procedureCache->begin<components::ProcedureContainer>();
		middle::Id procId;
		components::ProcedureContainer* procComp = nullptr;
		if (procedureCache->getSize() == 1) {
			procId = procedureCache->relevantIdVector[0];
			procComp = *procedureIt;
		}
		if (!procComp) {
			return;
		}

		auto buttonIt = buttonCache->begin<components::Button>();
		for (int i = 0; i < buttonCache->getSize(); ++i) {
			auto button = *buttonIt;
			if (button->function == bubbleButton::SAVE_BUTTON) {
				middle::saveShape(gameState, procId, "../bubbleData/procedures/", "procedure1");
			}
			if (button->function == bubbleButton::LOAD_BUTTON) {
				std::string folder = "../bubbleData/procedures/";
				std::string shapeName = "procedure1";
				Vector3 mouseXZ = gameState->input.mouseXZ_PlanePos;
				middle::Id loadedProcId = middle::loadShape(gameState, folder, shapeName, true, mouseXZ);
				auto& procedureShape = middle::getShape(gameState, loadedProcId.index);
				//middle::addComponent<components::PlacementComponent>(procedureShape);
			}

		}

		if (procComp->activeBlock.index != middle::UNASSIGNED) {
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
	}
};

static middle::SystemRegistrar<ProcedureUiSystem> reg("ProcedureUiSystem");
