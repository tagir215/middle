#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "Rotation.h" 
#include "MouseSelectable.h"
#include "GlobalTransform.h"

class GizmoRenderSetupSystem : public middle::MiddleGameplaySystem {
public:
	GizmoRenderSetupSystem() {
		systemModeType = middle::SystemModeType::EDITOR;
		systemUpdateType = middle::SystemUpdateType::RENDERING;
	}
	components::CompCache* cache;

	void init(middle::GameState* gameState) override {
		cache = middle::newCompCache(gameState, systemName);
		cache->addType<components::Rotation>();
		cache->addType<components::MouseSelectable>();
		cache->addType<components::GlobalTransform>();
	}
	void update(middle::GameState* gameState) override {
		auto rotationIt = cache->begin<components::Rotation>();
		auto transformIt = cache->begin<components::GlobalTransform>();
		auto selectableIt = cache->begin<components::MouseSelectable>();
		for (int i = 0; i < cache->getSize(); ++i) {
			auto rotation = *rotationIt;
			auto transform = *transformIt;
			auto selectable = *selectableIt;
			if (!selectable->selected) {
				continue;
			}
			Vector3 pos = transform->pos;
			Vector3 forward = Vector3RotateByQuaternion(middle::ROTATION_FORWARD, rotation->rotation);
			middle::RenderItem rotItem;
			rotItem.type = middle::RenderItemType::VECTOR;
			rotItem.center = { 0,0,0 };
			rotItem.color = WHITE;
			rotItem.color.a = 50.0f;
			rotItem.transform.translation = pos;
			rotItem.transform.rotation = rotation->rotation;
			rotItem.transform.scale = { 1,1,1 };
			rotItem.length = 20;
			rotItem.radius = 2;
			gameState->renderData.push_back(rotItem);

			middle::RenderItem circleX;
			circleX.type = middle::RenderItemType::CIRCLE;
			circleX.center = { 0,0,0 };
			circleX.color = BLUE;
			circleX.ringRadius = 0.2f;
			circleX.transform.translation = pos;
			circleX.transform.scale = { 1,1,1 };
			circleX.transform.rotation = QuaternionFromVector3ToVector3({ 0,1,0 }, { 1,0,0 });
			circleX.radius = 30;
			gameState->renderData.push_back(circleX);

			middle::RenderItem circleY;
			circleY.type = middle::RenderItemType::CIRCLE;
			circleY.center = { 0,0,0 };
			circleY.color = GREEN;
			circleY.ringRadius = 0.2f;
			circleY.transform.translation = pos;
			circleY.transform.scale = { 1,1,1 };
			circleY.transform.rotation = QuaternionFromVector3ToVector3({ 0,1,0 }, { 0,1,0 });
			circleY.radius = 30;
			gameState->renderData.push_back(circleY);

			middle::RenderItem circleZ;
			circleZ.type = middle::RenderItemType::CIRCLE;
			circleZ.center = { 0,0,0 };
			circleZ.color = RED;
			circleZ.ringRadius = 0.2f;
			circleZ.transform.translation = pos;
			circleZ.transform.scale = { 1,1,1 };
			circleZ.transform.rotation = QuaternionFromVector3ToVector3({ 0,1,0 }, { 0,0,1 });
			circleZ.radius = 30;
			gameState->renderData.push_back(circleZ);
		}
	}
};

static middle::SystemRegistrar<GizmoRenderSetupSystem> reg("GizmoRenderSetupSystem");
