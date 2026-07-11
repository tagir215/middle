#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "Position.h"
#include "Rotation.h"
#include "Scale.h"
#include "MouseSelectable.h"
#include "middle_shape_utils.h"
#include "middle_math.h"
#include "DragStart.h"
#include "component_utils.h"

class GizmoSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* cache;
	components::CompCache* draggedCache;

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
		Vector3 axis;
	};

	AxisTestResult axisTest(middle::GameState* gameState, const Vector3& spherePos, float radius) {
		Vector3 rayStart = gameState->input.mouseNearPlanePos;
		Vector3 rayDir = gameState->input.mouseDir;
		Vector3 collisionPosX = middle::RayCastLinePlane(spherePos, { 0,1,0 }, rayStart, rayDir);
		Vector3 collisionPosY = middle::RayCastLinePlane(spherePos, { 0,0,1 }, rayStart, rayDir);
		Vector3 collisionPosZ = middle::RayCastLinePlane(spherePos, { 1,0,0 }, rayStart, rayDir);
		std::vector<AxisTestResult>candidates = {
			{ 0, collisionPosX, {0,1,0} },
			{ 1, collisionPosY, {0,0,1} },
			{ 2, collisionPosZ, {1,0,0} }
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

	void renderAxis(middle::GameState* gameState, const Vector3& gizmoPos, const Color& color, const Vector3& axis, float gizmoRadius) {
		middle::RenderItem cyl;
		cyl.type = middle::RenderItemType::CYLINDER;
		cyl.center = { 0,0,0 };
		cyl.color = color;
		cyl.ringRadius = gizmoRadius;
		cyl.transform.translation = gizmoPos;
		cyl.length = 0.1f;
		cyl.transform.scale = { 1,1,1 };
		cyl.transform.rotation = QuaternionFromVector3ToVector3({ 0,1,0 }, axis);
		cyl.radius = gizmoRadius;
		gameState->renderData.push_back(cyl);
	}

	void init(middle::GameState* gameState) override {
		cache = middle::newCompCache(gameState, systemName);
		cache->addType<components::Position>();
		cache->addType<components::Rotation>();
		cache->addType<components::Scale>();
		cache->addType<components::MouseSelectable>();
		draggedCache = middle::newCompCache(gameState, systemName);
		draggedCache->addType<components::DragStart>();
		draggedCache->addType<components::Rotation>();
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

			if (!selectable->selected) {
				continue;
			}

			const float gizmoRadius = 30;

			Vector3 gizmoPos = { position->posX, position->posY, position->posZ };
			if (!broadTest(gameState, gizmoPos, gizmoRadius)) {
				continue;
			}

			AxisTestResult axisResult = axisTest(gameState, gizmoPos, gizmoRadius);
			middle::RenderItem indicator;
			indicator.type = middle::RenderItemType::SPHERE;
			indicator.center = axisResult.collisionPos;
			indicator.radius = 3;
			Color color;
			if (axisResult.resultAxis == 0) {
				color = GREEN;
			}
			else if (axisResult.resultAxis == 1) {
				color = RED;
			}
			else if (axisResult.resultAxis == 2) {
				color = BLUE;
			}
			else {
				return;
			}
			color.a = 30;
			indicator.color = color;
			gameState->renderData.push_back(indicator);

			renderAxis(gameState, gizmoPos, color, axisResult.axis, gizmoRadius);

			if (gameState->input.rotatePressed) {
				auto& shape = middle::getShape(gameState, cache->relevantIdVector[i].index);
				auto comp = middle::attachComponent<components::DragStart>(gameState, shape.id);
				comp->dragStartPos = gameState->input.mouseXZ_PlanePos;
				comp->axis = axisResult.axis;
				comp->axisId = axisResult.resultAxis;
				comp->gizmoPos = gizmoPos;
				comp->initRotation = rotation->rotation;
			}

			bool intersectsWithX;
		}

		auto draggedIt = draggedCache->begin<components::DragStart>();
		auto rotationIt2 = draggedCache->begin<components::Rotation>();
		for (int i = 0; i < draggedCache->getSize(); ++i) {
			auto drag = *draggedIt;
			auto rotation = *rotationIt2;
			Color color;
			Vector3 perpAxis = { 1,0,0 };
			if (drag->axisId == 0) {
				color = GREEN;
			}
			else if (drag->axisId == 1) {
				color = RED;
			}
			else if (drag->axisId == 2) {
				color = BLUE;
			}
			color.a = 60;
			renderAxis(gameState, drag->gizmoPos, color, drag->axis, 30);

			if (gameState->input.rotateReleased) {
				middle::queueComponentDeletion<components::DragStart>(gameState, draggedCache->relevantIdVector[i]);
			}

			// rotation
			float rotateDelta = Vector3Subtract(gameState->input.mouseXZ_PlanePos, drag->dragStartPos).x;
			float scalor = 0.1f;
			Quaternion axisQuat = QuaternionFromAxisAngle(drag->axis, rotateDelta * scalor);
			rotation->rotation = QuaternionMultiply(drag->initRotation, axisQuat);
		}

	}
};

static middle::SystemRegistrar<GizmoSystem> reg("GizmoSystem");
