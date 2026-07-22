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

class GlobalCoordinateCalculationSystem : public middle::MiddleGameplaySystem {
	components::CompCache* posScaleCache;
	components::CompCache* circleCache;

	void init(middle::GameState* gameState) override {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::PREFRAME;

		posScaleCache = middle::newCompCache(gameState, systemName);
		posScaleCache->addType<components::LocalPosition>();
		posScaleCache->addType<components::LocalScale>();
		posScaleCache->addType<components::GlobalTransform>();

		circleCache = middle::newCompCache(gameState, systemName);
		circleCache->addType<components::Circle>();
		circleCache->addType<components::GlobalRadius>();
		circleCache->addType<components::GlobalTransform>();
	}


	void calculateTransforms(middle::GameState* gameState, middle::Id id, const Matrix& parentM, const Vector3& parentScale) {
		auto& shape = middle::getShape(gameState, id.index);
		auto scaleComp = middle::getComponent<components::LocalScale>(shape);
		auto posComp = middle::getComponent<components::LocalPosition>(shape);

		if (!scaleComp) {
			middle::attachComponent<components::LocalScale>(gameState, shape.id);
			return;
		}
		if (!posComp) {
			auto oldPos = middle::getComponent<components::Position>(shape);
			auto pos = middle::attachComponent<components::LocalPosition>(gameState, shape.id);
			if (oldPos) {
				pos->pos = { oldPos->posX, oldPos->posY, oldPos->posZ };
			}
			return;
		}
		if (!middle::getComponent<components::GlobalTransform>(shape)) {
			middle::attachComponent<components::GlobalTransform>(gameState, id);
			return;
		}

		const Vector3& scale = scaleComp->scale;
		const Vector3& pos = posComp->pos;
		Matrix scaleM = MatrixScale(scale.x, scale.y, scale.z);
		Matrix translateM = MatrixTranslate(pos.x, pos.y, pos.z);


		Matrix m = parentM;
		Matrix localM = MatrixMultiply(scaleM, translateM);
		m = MatrixMultiply(localM, m);


		auto globalT = middle::getComponent<components::GlobalTransform>(shape);
		const Quaternion assumedRotation = { 0,0,0,0 };
		globalT->pos = Vector3Transform(Vector3{ 0,0,0 }, m);
		globalT->scale = scaleComp->scale * parentScale;
		globalT->rotation = assumedRotation;

		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);
		for (middle::Id childId : children) {
			calculateTransforms(gameState, childId, m, globalT->scale);
		}

	}

	void update(middle::GameState* gameState) override {


		std::vector<middle::Id>topLevelIds;
		for (middle::Id id : posScaleCache->relevantIdVector) {
			if (middle::getParent(gameState, id).index == middle::UNASSIGNED) {
				topLevelIds.push_back(id);
			}
		}

		for (middle::Id id : topLevelIds) {
			calculateTransforms(gameState, id, MatrixIdentity(), Vector3{1,1,1});
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
	}
};

static middle::SystemRegistrar<GlobalCoordinateCalculationSystem> reg("GlobalCoordinateCalculationSystem");
