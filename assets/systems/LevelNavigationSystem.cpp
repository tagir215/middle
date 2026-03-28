#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "LevelReference.h"
#include "MouseClickComponent.h"
#include "editor_actions.h"
#include "Position.h"
#include "Circle.h"

class LevelNavigationSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* clickedCache;
	components::CompCache* levelCache;

	void init(middle::GameState* gameState) {
		clickedCache = middle::newCompCache(gameState);
		clickedCache->addType<components::LevelReference>();
		clickedCache->addType<components::MouseClickComponent>();
		levelCache = middle::newCompCache(gameState);
		levelCache->addType<components::LevelReference>();
		levelCache->addType<components::Position>();
		levelCache->addType<components::Circle>();
	}

	void saveState(middle::GameState* gameState) {
		std::string name = gameState->activeSceneName;
		if (name == "LevelSelect") {
			middle::saveScene(gameState, name);
		}
	}

	void update(middle::GameState* gameState) override {

		auto clickedLevelIt = clickedCache->begin<components::LevelReference>();
		for (int i = 0; i < clickedCache->getSize(); ++i) {
			auto levelRef = *clickedLevelIt;
			std::string name = levelRef->levelName;
			middle::queueAction(gameState, std::make_shared<middle::CustomAction>(
				[name,this](middle::GameState* gameState) {
					saveState(gameState);
					middle::resetScene(gameState);
					middle::loadScene(gameState, "../assets/scenes/", name, false);
					gameState->activeSceneName = name;
				}));
		}

		auto levelIt = levelCache->begin<components::LevelReference>();
		auto levelPosIt = levelCache->begin<components::Position>();
		auto circleIt = levelCache->begin<components::Circle>();
		for (int i = 0; i < levelCache->getSize(); ++i) {
			auto levelRef = *levelIt;
			auto pos = *levelPosIt;
			auto circle = *circleIt;

			if (gameState->bubbleAlgebraState.justCompletedLevel) {
				if (levelRef->levelName == gameState->bubbleAlgebraState.completedLevelName) {
					gameState->bubbleAlgebraState.justCompletedLevel = false;
					gameState->bubbleAlgebraState.completedLevelName = "";
					levelRef->complete = true;
				}
			}

			if (levelRef->complete) {
				middle::RenderItem completeInd;
				completeInd.type = middle::RenderItemType::CYLINDER;
				completeInd.radius = circle->radius;
				completeInd.ringRadius = circle->radius;
				const float offsetY = 0.5f;
				completeInd.transform.translation = { pos->posX, pos->posY + offsetY, pos->posZ };
				completeInd.color = GREEN;
				completeInd.color.a = 40;
				completeInd.length = 0.1f;
				completeInd.center = { 0,0,0 };
				gameState->renderData.push_back(completeInd);
			}
		}
	}
};

static middle::SystemRegistrar<LevelNavigationSystem> reg("LevelNavigationSystem");
