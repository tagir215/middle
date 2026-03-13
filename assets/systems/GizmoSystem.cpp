#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "Position.h"
#include "Rotation.h"
#include "Scale.h"
#include "MouseSelectable.h"
#include "middle_shape_utils.h"
#include "middle_math.h"

class GizmoSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* cache;

	GizmoSystem() {
		systemModeType = middle::SystemModeType::EDITOR;
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
	}

	bool broadTest(middle::GameState* gameState, const Vector3& spherePos, float radius) {
		Vector3 rayStart = gameState->input.mouseNearPlanePos;
		Vector3 rayEnd = rayStart + Vector3Scale(gameState->input.mouseDir, 10000);
		Vector3 intersectPos;
		return middle::RayCastLineSphere(spherePos, radius, rayStart, rayEnd, intersectPos);
	}

	struct AxisTestResult {
		// 0 = X, 1 = Y, 2 = Z
		int resultAxis = -1;
		Vector3 collisionPos;
	};

	AxisTestResult axisTest(middle::GameState* gameState, const Vector3& spherePos, float radius) {
		Vector3 rayStart = gameState->input.mouseNearPlanePos;
		Vector3 rayDir = gameState->input.mouseDir;
		Vector3 collisionPosX = middle::RayCastLinePlane(spherePos, { 0,1,0 }, rayStart, rayDir);
		Vector3 collisionPosY = middle::RayCastLinePlane(spherePos, { 0,0,1 }, rayStart, rayDir);
		Vector3 collisionPosZ = middle::RayCastLinePlane(spherePos, { 1,0,0 }, rayStart, rayDir);
		std::vector<AxisTestResult>candidates = {
			{ 0, collisionPosX },
			{ 1, collisionPosY },
			{ 2, collisionPosZ }
		};
		// filter non collisions
		float radiusSq = radius * radius;
		for (int i = 2; i >= 0; --i) {
			float distSq = Vector3DistanceSqr(candidates[i].collisionPos, spherePos);
			if (distSq - 0.4f> radiusSq) {
				candidates.erase(candidates.begin() + i);
			}
		}

		if(candidates.size() == 0){
			return { -1,{0,0,0} };
		}

		float minDist = std::numeric_limits<float>::max();
		int bestIndex = 0;
		for(int i=0; i<candidates.size(); ++i){
			float distSq = Vector3DistanceSqr(candidates[i].collisionPos, rayStart);
			if (distSq < minDist) {
				bestIndex = i;
				minDist = distSq;
			}
		}

		return candidates[bestIndex];
	}


	void init(middle::GameState* gameState) override {
		cache = middle::newCompCache(gameState);
		cache->addType<components::Position>();
		cache->addType<components::Rotation>();
		cache->addType<components::Scale>();
		cache->addType<components::MouseSelectable>();
	}
	void update(middle::GameState* gameState) override {
		auto positionIt = cache->begin<components::Position>();
		auto rotationIt = cache->begin<components::Rotation>();
		auto scaleIt = cache->begin<components::Scale>();
		auto selectableIt = cache->begin<components::MouseSelectable>();

		for (int i = 0; i < cache->getSize(); ++i) {
			auto position = *positionIt;
			auto rotation = *rotationIt;
			auto scale = *scaleIt;
			auto selectable = *selectableIt;

			if (selectable->selected) {
				continue;
			}

			Vector3 gizmoPos = { position->posX, position->posY, position->posZ };
			if (!broadTest(gameState, gizmoPos, 30)) {
				continue;
			}

			AxisTestResult axisResult = axisTest(gameState, gizmoPos, 30);
			middle::RenderItem indicator;
			indicator.type = middle::RenderItemType::SPHERE;
			indicator.center = axisResult.collisionPos;
			indicator.radius = 3;
			if (axisResult.resultAxis == 0) {
				indicator.color = GREEN;
			}
			if (axisResult.resultAxis == 1) {
				indicator.color = RED;
			}
			if (axisResult.resultAxis == 2) {
				indicator.color = BLUE;
			}
			gameState->renderData.push_back(indicator);

			bool intersectsWithX;
		}
	}
};

static middle::SystemRegistrar<GizmoSystem> reg("GizmoSystem");
