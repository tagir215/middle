#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "GlobalTransform.h"
#include "LocalPosition.h"
#include "LocalScale.h"
#include "component_utils.h"
#include "Position.h"
#include "GlobalRadius.h"
#include "Circle.h"
#include "Rectangle.h"
#include "GlobalRect.h"

class GlobalCoordinateCalculationSystem : public middle::MiddleGameplaySystem {
	components::CompCache* posScaleCache;
	components::CompCache* circleCache;
	components::CompCache* rectCache;


	void init(middle::GameState* gameState) override {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::POSTFRAME;
		updatePriority = 10;

		posScaleCache = middle::newCompCache(gameState, systemName);
		posScaleCache->addType<components::LocalPosition>();
		posScaleCache->addType<components::LocalScale>();
		posScaleCache->addType<components::GlobalTransform>();

		circleCache = middle::newCompCache(gameState, systemName);
		circleCache->addType<components::Circle>();
		circleCache->addType<components::GlobalRadius>();
		circleCache->addType<components::GlobalTransform>();

		rectCache = middle::newCompCache(gameState, systemName);
		rectCache->addType<components::Rectangle>();
		rectCache->addType<components::GlobalRect>();
		rectCache->addType<components::GlobalTransform>();
	}

	void update(middle::GameState* gameState) override {
		std::vector<middle::Id>topLevelIds;
		for (middle::Id id : posScaleCache->relevantIdVector) {
			if (middle::getParent(gameState, id).index == middle::UNASSIGNED) {
				topLevelIds.push_back(id);
			}
		}

		for (middle::Id id : topLevelIds) {
			middle::updateGlobalTransforms(gameState, id, MatrixIdentity(), Vector3{1,1,1});
		}

		// updated global radiuses
		auto circleIt = circleCache->begin<components::Circle>();
		auto globalRadiusIt = circleCache->begin<components::GlobalRadius>();
		auto transformIt = circleCache->begin<components::GlobalTransform>();
		for (middle::Id id : circleCache->relevantIdVector) {
			auto circle = *circleIt;
			auto globalR = *globalRadiusIt;
			auto transform = *transformIt;
			globalR->radius = circle->radius * transform->scale.x;
		}

		// updated global rects
		auto rectIt = rectCache->begin<components::Rectangle>();
		auto globalRectIt = rectCache->begin<components::GlobalRect>();
		auto rectTransformIt = rectCache->begin<components::GlobalTransform>();
		for (middle::Id id : rectCache->relevantIdVector) {
			auto rect = *rectIt;
			auto globalRect = *globalRectIt;
			auto transform = *rectTransformIt;
			globalRect->width = rect->width * transform->scale.x;
			globalRect->height = rect->height * transform->scale.z;
		}
	}
};

static middle::SystemRegistrar<GlobalCoordinateCalculationSystem> reg("GlobalCoordinateCalculationSystem");
