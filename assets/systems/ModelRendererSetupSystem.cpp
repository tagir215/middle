#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "ModelComponent.h"

class ModelRendererSetupSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* cache;

	ModelRendererSetupSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::RENDERING;
	}
	void init(middle::GameState* gameState) override {
		cache = middle::newCompCache(gameState);
		cache->addType<components::ModelComponent>();
	}
	void update(middle::GameState* gameState) override {
		auto cacheIt = cache->begin<components::ModelComponent>();
		for (int i = 0; i < cache->getSize(); ++i) {
			auto model = *cacheIt;
			middle::RenderItem renderItem;
			renderItem.type = middle::RenderItemType::MODEL;
			renderItem.center = { 0,0,0 };
			renderItem.model = &model->model;
			renderItem.transform = {
				middle::getShapePosition(gameState, cache->relevantIdVector[i].index),
				{0,0,0,0},
				{1,1,1}
			};
			gameState->renderData.push_back(renderItem);
		}
	}
};

static middle::SystemRegistrar<ModelRendererSetupSystem> reg("ModelRendererSetupSystem");
