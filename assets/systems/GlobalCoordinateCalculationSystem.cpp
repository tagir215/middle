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

	void calculateTransforms(middle::GameState* gameState, middle::Id id, std::unordered_map<int, Matrix>& matrixMap) {
		auto& shape = middle::getShape(gameState, id.index);
		auto scaleComp = middle::getComponent<components::LocalScale>(shape);
		auto posComp = middle::getComponent<components::LocalPosition>(shape);
		const Vector3& scale = scaleComp->scale;
		const Vector3& pos = posComp->pos;
		Matrix scaleM = MatrixScale(scale.x, scale.y, scale.z);
		Matrix translateM = MatrixTranslate(pos.x, pos.y, pos.z);

		Matrix& m = matrixMap[id.index];

		Matrix transform = MatrixMultiply(translateM, scaleM);
		m = MatrixMultiply(transform, m);

		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);
		for (middle::Id childId : children) {
			calculateTransforms(gameState, childId, matrixMap);
		}
	}

	void update(middle::GameState* gameState) override {

		Quaternion assumedRotation = { 1,1,1,1 };

		std::unordered_map<int, Matrix>matrixMap;

		for (middle::Id id : posScaleCache->relevantIdVector) {
			matrixMap[id.index] = MatrixIdentity();
		}

		std::vector<middle::Id>topLevelIds;
		for (middle::Id id : posScaleCache->relevantIdVector) {
			if (middle::getParent(gameState, id).index == middle::UNASSIGNED) {
				topLevelIds.push_back(id);
			}
		}

		for (middle::Id id : topLevelIds) {
			calculateTransforms(gameState, id, matrixMap);
		}

		auto globalIt = posScaleCache->begin<components::GlobalTransform>();
		for (middle::Id id : posScaleCache->relevantIdVector) {
			auto globalT = *globalIt;
			Matrix& m = matrixMap[id.index];
			globalT->pos = Vector3Transform(Vector3{0,0,0}, m);
			globalT->scale = Vector3Transform(Vector3{1,1,1}, m);
			globalT->rotation = assumedRotation;
		}
	}
};

static middle::SystemRegistrar<GlobalCoordinateCalculationSystem> reg("GlobalCoordinateCalculationSystem");
