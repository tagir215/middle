#pragma once
#include "middle_system_registrar.h"
#include "GridElement.h"
#include "EditorConfigs.h"
#include "middle_math.h"
#include "comp_cache.h"
#include "middle_shape_utils.h"
#include "LocalPosition.h"

class GridSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* cachoA = nullptr;
	components::CompCache* cachoB = nullptr;

	GridSystem() {
		systemModeType = middle::SystemModeType::EDITOR;
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;
	}
	void init(middle::GameState* gameState) override {
		cachoA = middle::newCompCache(gameState, systemName);
		cachoA->addType<components::EditorConfigs>();
		cachoB = middle::newCompCache(gameState, systemName);
		cachoB->addType<components::GridElement>();
		cachoB->addType<components::LocalPosition>();

	}

	bool initialized = false;

	void update(middle::GameState* gameState) override {

		auto& configIt = cachoA->begin<components::EditorConfigs>();
		auto& gridIt = cachoB->begin<components::GridElement>();
		auto& posIt = cachoB->begin<components::LocalPosition>();

		components::EditorConfigs* editorConfigs = nullptr;
		for (int i = 0; i < cachoA->getSize(); ++i) {
			editorConfigs = *configIt;
		}

		if (!editorConfigs)
			return;

		for (int i = 0; i < cachoB->getSize(); ++i) {
			auto gridElement = *gridIt;
			auto position = *posIt;
			middle::Id id = cachoB->relevantIdVector[i];

			Vector3 pos = middle::getGlobalPosition(gameState, id.index);
			Vector3 targetPos = middle::gridPosition(pos, editorConfigs->gridSize);
			middle::moveShape(gameState, id.index, targetPos - pos);
		}

	}
};

static middle::SystemRegistrar<GridSystem> reg("GridSystem");
