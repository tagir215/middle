#pragma once
#include "middle_system_registrar.h"
#include "Position.h"
#include "GridElement.h"
#include "EditorConfigs.h"
#include "middle_math.h"
#include "comp_cache.h"

class GridSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache cachoA;
	components::CompCache cachoB;

	GridSystem() {
		systemModeType = middle::SystemModeType::EDITOR;
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;

		cachoA.addType<components::EditorConfigs>();
		cachoB.addType<components::GridElement>();
		cachoB.addType<components::Position>();
	}


	void update(middle::GameState* gameState) override {

		auto& configIt = cachoA.begin<components::EditorConfigs>();
		auto& gridIt = cachoB.begin<components::GridElement>();
		auto& posIt = cachoB.begin<components::Position>();

		for (int i = 0; i < cachoA.getSize(); ++i) {
			auto config = *configIt;
		}

		for (int i = 0; i < cachoB.getSize(); ++i) {
			auto grid = *gridIt;
			auto pos = *posIt;
		}

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
