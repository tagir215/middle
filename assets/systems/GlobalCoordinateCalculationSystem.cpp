#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "GlobalTransform.h"
#include "LocalPosition.h"
#include "LocalScale.h"
#include "component_utils.h"
#include "Position.h"

class GlobalCoordinateCalculationSystem : public middle::MiddleGameplaySystem {
	components::CompCache* posScaleCache;

	void init(middle::GameState* gameState) override {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;

		posScaleCache = middle::newCompCache(gameState, systemName);
		posScaleCache->addType<components::LocalPosition>();
		posScaleCache->addType<components::LocalScale>();
		posScaleCache->addType<components::GlobalTransform>();
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
		Matrix transform = MatrixMultiply(scaleM, translateM);
		m = MatrixMultiply(transform, m);


		auto globalT = middle::getComponent<components::GlobalTransform>(shape);
		const Quaternion assumedRotation = { 1,1,1,1 };
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
	}
};

static middle::SystemRegistrar<GlobalCoordinateCalculationSystem> reg("GlobalCoordinateCalculationSystem");
