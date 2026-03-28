#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "LevelReference.h"
#include "MouseClickComponent.h"
#include "editor_actions.h"

class LevelNavigationSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* cache;

	void init(middle::GameState* gameState) {
		cache = middle::newCompCache(gameState);
		cache->addType<components::LevelReference>();
		cache->addType<components::MouseClickComponent>();
	}

	void saveState(middle::GameState* gameState) {
		std::string name = gameState->activeSceneName;
		if (name == "LevelSelect") {
			int a = 0;
		}
	}

	void update(middle::GameState* gameState) override {

		auto levelIt = cache->begin<components::LevelReference>();
		for (int i = 0; i < cache->getSize(); ++i) {
			auto levelRef = *levelIt;
			std::string name = levelRef->levelName;
			middle::queueAction(gameState, std::make_shared<middle::CustomAction>(
				[name,this](middle::GameState* gameState) {
					saveState(gameState);
					middle::resetScene(gameState);
					middle::loadScene(gameState, "../assets/scenes/", name, false);
				}));
		}
	}
};

static middle::SystemRegistrar<LevelNavigationSystem> reg("LevelNavigationSystem");
