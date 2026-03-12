#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "comp_cache.h"
#include "middle_shape_utils.h"
#include "ModelComponent.h"
#include "MouseSelectable.h"
#include "editor_actions.h"
#include "component_utils.h"

class ModelLoadingSystem : public middle::MiddleGameplaySystem {
public:
	ModelLoadingSystem() {
		systemModeType = middle::SystemModeType::EDITOR;
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
	}

	components::CompCache* selectableCache;
	components::CompCache* modelCache;

	void init(middle::GameState* gameState) override {
		selectableCache = middle::newCompCache(gameState);
		selectableCache->addType<components::MouseSelectable>();
		modelCache = middle::newCompCache(gameState);
		modelCache->addType<components::ModelComponent>();
	}
	void update(middle::GameState* gameState) override {
		if (gameState->loadedModels.size() > 0) {

			auto selectableIt = selectableCache->begin<components::MouseSelectable>();
			for (int i = 0; i < selectableCache->getSize(); ++i) {
				auto selectable = *selectableIt;
				if (selectable->selected) {
					middle::ModelContainer& modelContainer = gameState->loadedModels.back();
					Model model = modelContainer.model;
					std::string path = modelContainer.path;
					middle::Id id = selectableCache->relevantIdVector[i];
					auto customAction = std::make_shared<middle::CustomActionWithUndo>(
						[id, model, path](middle::GameState* gameState) {
							auto& selectableShape = middle::getShape(gameState, id.index);
							auto comp = middle::getComponent<components::ModelComponent>(selectableShape);
							if (!comp) {
								comp = middle::attachComponent<components::ModelComponent>(gameState, selectableShape.id);
							}
							comp->model = model;
							comp->path = path;
							comp->initialized = true;
						},
						[id](middle::GameState* gameState) {
							auto& selectableShape = middle::getShape(gameState, id.index);
							middle::queueComponentDeletion<components::ModelComponent>(gameState, selectableShape.id);
						});
					middle::queueEditorAction(gameState, customAction);
					gameState->loadedModels.pop_back();
					break;
				}
			}

		}

		auto modelIt = modelCache->begin<components::ModelComponent>();
		for (int i = 0; i < modelCache->getSize(); ++i) {
			auto model = *modelIt;
			if (!model->initialized) {
				gameState->modelsToLoadQueue.push(model->path);
			}
		}

		modelIt = modelCache->begin<components::ModelComponent>();
		if (gameState->loadedModels.size() > 0) {
			middle::ModelContainer& container = gameState->loadedModels.back();
			for (int i = 0; i < modelCache->getSize(); ++i) {
				auto modelComp = *modelIt;
				if (!modelComp->initialized && container.path == modelComp->path) {
					modelComp->model = container.model;
					modelComp->initialized = true;
					gameState->loadedModels.pop_back();
					break;
				}
			}
		}
	}
};

static middle::SystemRegistrar<ModelLoadingSystem> reg("ModelLoadingSystem");
