#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "GlobalTransform.h"
#include "LocalPosition.h"
#include "LocalScale.h"
#include "component_utils.h"

class GlobalCoordinateCalculationSystem : public middle::MiddleGameplaySystem {
	components::CompCache* posScaleCache;

	void init(middle::GameState* gameState) override {
		posScaleCache = middle::newCompCache(gameState, systemName);
		posScaleCache->addType<components::LocalPosition>();
		posScaleCache->addType<components::LocalScale>();
		posScaleCache->addType<components::GlobalTransform>();

	}

	std::vector<middle::Id>getParents(middle::GameState* gameState, middle::Id id) {
		std::vector<middle::Id>ids;
		while (true) {
			middle::Id parentId = middle::getParent(gameState, id);
			if (parentId.index == middle::UNASSIGNED) {
				break;
			}
			ids.push_back(parentId);
		}
		return ids;
	}

	void update(middle::GameState* gameState) override {

		Quaternion assumedRotation = { 1,1,1,1 };

		auto scaleIt = posScaleCache->begin<components::LocalScale>();
		auto posIt = posScaleCache->begin<components::LocalPosition>();
		auto globalIt = posScaleCache->begin<components::GlobalTransform>();
		int i = 0;
		for (middle::Id id : posScaleCache->relevantIdVector) {
			auto scale = *scaleIt;
			auto pos = *posIt;
			auto global = *globalIt;
			std::vector<middle::Id>parents = getParents(gameState, id);
			Vector3 totalScale = scale->scale;
			Vector3 totalPos = pos->pos;
			for (middle::Id& parentId : parents) {
				auto parentShape = middle::getShape(gameState, parentId.index);
				auto parentScale = middle::getComponent<components::LocalScale>(parentShape);
				auto parentPos = middle::getComponent<components::LocalPosition>(parentShape);
				totalScale *= parentScale->scale;
				totalPos += parentPos->pos;
			}
			global->pos = totalPos;
			global->rotation = assumedRotation;
			global->scale = totalScale;
		}

	}
};

static middle::SystemRegistrar<GlobalCoordinateCalculationSystem> reg("GlobalCoordinateCalculationSystem");
