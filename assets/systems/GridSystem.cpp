#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "Position.h"
#include "GridElement.h"
#include "EditorConfigs.h"
#include "middle_math.h"

class GridSystem : public middle::MiddleGameplaySystem {
public:
	GridSystem() {
		systemModeType = middle::SystemModeType::EDITOR;
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;
	}

	void update(middle::GameState* gameState) override {

		middle::Id& editorConfigs = 
			middle::findFirstShapeWithComp(gameState, middle::getTypeId<components::EditorConfigs>());

		if (editorConfigs.index == middle::UNASSIGNED) {
			return;
		}

		auto& configShape = middle::getShape(gameState, editorConfigs.index);
		auto configs = middle::getComponent<components::EditorConfigs>(configShape);


		middle::loopInstances(gameState, [gameState, configs](int i, middle::Shape& shape) {
			auto gridElement = middle::getComponent<components::GridElement>(shape);
			if (!gridElement)
				return true;
			auto position = middle::getComponent<components::Position>(shape);

			Vector3 pos = middle::getShapePosition(gameState, i);

			Vector3 targetPos = middle::gridPosition(pos, configs->gridSize);

			middle::moveShape(gameState, i, targetPos - pos);
			return true;
			});
	}
};

static middle::SystemRegistrar<GridSystem> reg("GridSystem");
