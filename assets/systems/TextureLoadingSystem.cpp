#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "TextureComponent.h"
#include "MouseSelectable.h"
#include "middle_shape_utils.h"
#include "editor_actions.h"
#include "component_utils.h"


class TextureLoadingSystem : public middle::MiddleGameplaySystem {
public:
	TextureLoadingSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
	}

	components::CompCache* selectableCache;
	components::CompCache* textureCache;

	void init(middle::GameState* gameState) override {
		selectableCache = middle::newCompCache(gameState);
		selectableCache->addType<components::MouseSelectable>();

	}
	void update(middle::GameState* gameState) override {

		if (gameState->loadedTextures.size() > 0) {
			auto selectableIt = selectableCache->begin<components::MouseSelectable>();
			for (int i = 0; i < selectableCache->getSize(); ++i) {
				auto selectable = *selectableIt;
				if (selectable->selected) {
					middle::TextureContainer& textureContainer = gameState->loadedTextures.back();
					Texture2D texture = textureContainer.texture;
					std::string path = textureContainer.path;
					middle::Id id = selectableCache->relevantIdVector[i];
					auto customAction = std::make_shared<middle::CustomActionWithUndo>(
						[id, texture, path](middle::GameState* gameState) {
							auto& selectableShape = middle::getShape(gameState, id.index);
							auto comp = middle::getComponent<components::TextureComponent>(selectableShape);
							if (!comp) {
								comp = middle::attachComponent<components::TextureComponent>(gameState, selectableShape.id);
							}
							comp->texture = texture;
							comp->path = path;
							comp->scale = 30;
						},
						[id](middle::GameState* gameState) {
							auto& selectableShape = middle::getShape(gameState, id.index);
							middle::queueComponentDeletion<components::TextureComponent>(gameState, selectableShape.id);
						});
					middle::queueEditorAction(gameState, customAction);
					gameState->loadedTextures.pop_back();
					break;
				}
			}
		}
	}
};

static middle::SystemRegistrar<TextureLoadingSystem> reg("TextureLoadingSystem");
