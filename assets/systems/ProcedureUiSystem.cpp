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

class ProcedureUiSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* buttonCache;

	void init(middle::GameState* gameState) {
		buttonCache = middle::newCompCache(gameState);
		buttonCache->addType<components::Button>();
		buttonCache->addType<components::MouseClickComponent>();
	}

	// there should be only one procedure... so it's the first one 
	middle::Id& findProcedure(middle::GameState* gameState) {

		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!middle::isShapeAlive(gameState, i))
				continue;

			middle::Shape& shape = middle::getShape(gameState, i);
			auto procedure = middle::getComponent<components::ProcedureContainer>(shape);
			if (procedure) {
				return shape.id;
			}
		}
		return middle::Id();
	}

	void update(middle::GameState* gameState) override {

		auto buttonIt = buttonCache->begin<components::Button>();
		for (int i = 0; i < buttonCache->getSize(); ++i) {
			auto button = *buttonIt;
			if (button->function == bubbleButton::SAVE_BUTTON) {
				middle::Id procedure = findProcedure(gameState);
				middle::saveShape(gameState, procedure, "../bubbleData/procedures/", "procedure1");
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
	}
};

static middle::SystemRegistrar<ProcedureUiSystem> reg("ProcedureUiSystem");
