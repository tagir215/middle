#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "ModelComponent.h"
#include "GlobalTransform.h"

class ModelRendererSetupSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* cache;

	ModelRendererSetupSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::RENDERING;
	}
	void init(middle::GameState* gameState) override {
		cache = middle::newCompCache(gameState, systemName);
		cache->addType<components::ModelComponent>();
		cache->addType<components::GlobalTransform>();
	}
	void update(middle::GameState* gameState) override {
		auto cacheIt = cache->begin<components::ModelComponent>();
		auto transformIt = cache->begin<components::GlobalTransform>();
		for (int i = 0; i < cache->getSize(); ++i) {
			auto model = *cacheIt;
			auto transform = *transformIt;
			middle::RenderItem renderItem;
			renderItem.type = middle::RenderItemType::MODEL;
			renderItem.center = { 0,0,0 };
			renderItem.model = &model->model;
			renderItem.transform = {
				transform->pos,
				{0,0,0,0},
				transform->scale
			};
			gameState->renderData.push_back(renderItem);
		}
	}
};

static middle::SystemRegistrar<ModelRendererSetupSystem> reg("ModelRendererSetupSystem");
