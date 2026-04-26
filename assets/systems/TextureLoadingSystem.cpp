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

		textureCache = middle::newCompCache(gameState);
		textureCache->addType<components::TextureComponent>();
	}


	void update(middle::GameState* gameState) override {

		if (gameState->loadedTextureMap.size() > 0) {
			auto selectableIt = selectableCache->begin<components::MouseSelectable>();

			auto it = gameState->loadedTextureMap.begin();
			middle::TextureContainer& textureContainer = it->second;

			for (int i = 0; i < selectableCache->getSize(); ++i) {
				auto selectable = *selectableIt;
				if (selectable->selected) {
					Texture2D texture = textureContainer.texture;
					std::string filename = textureContainer.filename;
					middle::Id id = selectableCache->relevantIdVector[i];
					auto customAction = std::make_shared<middle::CustomActionWithUndo>(
						[id, texture, filename](middle::GameState* gameState) {
							auto& selectableShape = middle::getShape(gameState, id.index);
							auto comp = middle::getComponent<components::TextureComponent>(selectableShape);
							if (!comp) {
								comp = middle::attachComponent<components::TextureComponent>(gameState, selectableShape.id);
							}
							comp->texture = texture;
							comp->filename = filename;
							comp->scale = 30;
							comp->initialized = true;
						},
						[id](middle::GameState* gameState) {
							auto& selectableShape = middle::getShape(gameState, id.index);
							middle::queueComponentDeletion<components::TextureComponent>(gameState, selectableShape.id);
						});
					middle::queueEditorAction(gameState, customAction);
					gameState->loadedTextureMap.clear();
					break;
				}
			}
		}

		// TODO REFACTOR SLOW
		if (gameState->loadedTextureMap.size() > 0) {
			auto textureIt = textureCache->begin<components::TextureComponent>();
			for (int i = 0; i < textureCache->getSize(); ++i) {
				auto textureComp = *textureIt;
				if (!textureComp->initialized && gameState->loadedTextureMap.find(textureComp->filename) != gameState->loadedTextureMap.end()) {
					textureComp->texture = gameState->loadedTextureMap[textureComp->filename].texture;
					textureComp->initialized = true;
				}
			}
			gameState->loadedTextureMap.clear();
		}

		auto textureIt = textureCache->begin<components::TextureComponent>();
		for (int i = 0; i < textureCache->getSize(); ++i) {
			auto texture = *textureIt;
			if (!texture->initialized) {
				gameState->texturesToLoadQueue.push(texture->filename);
			}
		}

	}
};

static middle::SystemRegistrar<TextureLoadingSystem> reg("TextureLoadingSystem");
