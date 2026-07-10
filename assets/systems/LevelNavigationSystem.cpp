#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "LevelReference.h"
#include "MouseClickComponent.h"
#include "editor_actions.h"
#include "Position.h"
#include "Circle.h"
#include "bubble_constants.h"
#include "IdRef.h"
#include "component_utils.h"
#include "CameraComponent.h"
#include "InitializedTag.h"
#include "Button.h"


class LevelNavigationSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* clickedCache;
	components::CompCache* levelCache;
	components::CompCache* unInitializedLevelCache;
	components::CompCache* cameraCache;
	components::CompCache* buttonCache;

	void init(middle::GameState* gameState) {
		clickedCache = middle::newCompCache(gameState);
		clickedCache->addType<components::LevelReference>();
		clickedCache->addType<components::MouseClickComponent>();
		levelCache = middle::newCompCache(gameState);
		levelCache->addType<components::LevelReference>();
		levelCache->addType<components::Position>();
		levelCache->addType<components::Circle>();
		unInitializedLevelCache = middle::newCompCache(gameState);
		unInitializedLevelCache->addType<components::LevelReference>();
		unInitializedLevelCache->addType<components::InitializedTag>(components::NOTINTERESTED);
		cameraCache = middle::newCompCache(gameState);
		cameraCache->addType<components::CameraComponent>();
		buttonCache = middle::newCompCache(gameState);
		buttonCache->addType<components::Button>();
		buttonCache->addType<components::MouseClickComponent>();
	}

	void saveState(middle::GameState* gameState) {
		std::string name = gameState->activeSceneName;
		if (name == "LevelSelect") {
			middle::saveScene(gameState, name);
		}
	}

	bool initialized = false;

	void queueLevelNavigation(middle::GameState* gameState, const std::string& name) {
			middle::queueAction(gameState, std::make_shared<middle::CustomAction>(
				[name,this](middle::GameState* gameState) {
					saveState(gameState);
					gameState->bubbleAlgebraState.previousLevelName = gameState->activeSceneName;
					middle::resetScene(gameState);
					middle::loadScene(gameState, "../assets/scenes/", name, false);
					gameState->activeSceneName = name;
					gameState->bubbleAlgebraState.procedureNames.clear();
				}));
			initialized = false;
	}

	void update(middle::GameState* gameState) override {

		auto clickedLevelIt = clickedCache->begin<components::LevelReference>();
		for (int i = 0; i < clickedCache->getSize(); ++i) {
			auto levelRef = *clickedLevelIt;
			std::string name = levelRef->levelName;
			queueLevelNavigation(gameState, name);
		}

		bool reset = false;
		auto buttonIt = buttonCache->begin<components::Button>();
		for (int i = 0; i < buttonCache->relevantIdVector.size() > 0; ++i) {
			auto button = *buttonIt;
			if (button->function == bubbleButton::RESET_PROGRESS) {
				reset = true;
			}
		}

		auto levelIt = levelCache->begin<components::LevelReference>();
		auto levelPosIt = levelCache->begin<components::Position>();
		auto circleIt = levelCache->begin<components::Circle>();
		for (int i = 0; i < levelCache->getSize(); ++i) {
			auto levelRef = *levelIt;
			auto pos = *levelPosIt;
			auto circle = *circleIt;
			if (reset) {
				levelRef->complete = false;
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

		if (reset) {
			saveState(gameState);
		}

		auto unLevelIt = unInitializedLevelCache->begin<components::LevelReference>();
		for (int i = 0; i < unInitializedLevelCache->getSize(); ++i) {
			auto levelRef = *unLevelIt;
			auto& shape = middle::getShape(gameState, unInitializedLevelCache->relevantIdVector[i].index);

			middle::attachComponent<components::InitializedTag>(gameState, shape.id);

			if (gameState->bubbleAlgebraState.previousLevelName == "LevelSelect") {
				continue;
			}

			if (levelRef->levelName == gameState->bubbleAlgebraState.previousLevelName) {

				if (gameState->bubbleAlgebraState.justCompletedLevel) {
					auto idRef = middle::getComponent<components::IdRef>(shape);
					if (idRef && middle::isValidId(gameState, idRef->idRef)) {
						gameState->bubbleAlgebraState.justCompletedLevel = false;
						//std::string completedLevelName = gameState->bubbleAlgebraState.previousLevelName;
						//gameState->bubbleAlgebraState.previousLevelName = "";
						levelRef->complete = true;
						saveState(gameState);

						//auto& nextShape = middle::getShape(gameState, idRef->idRef.index);
						//auto nextLevelRef = middle::getComponent<components::LevelReference>(nextShape);
						//queueLevelNavigation(gameState, nextLevelRef->levelName);
						//break;
					}
				}

				//if (cameraCache->relevantIdVector.size() > 1) {
				//	int indx = cameraCache->relevantIdVector[0].index;
				//	auto action = std::make_shared<middle::CustomAction>([indx](middle::GameState* gameState) {
				//		middle::deleteShapeRecursive(gameState, indx);
				//		});
				//	middle::queueAction(gameState, action);
				//	return;
				//}

				auto camIt = cameraCache->begin<components::CameraComponent>();
				middle::Id cameraId = cameraCache->relevantIdVector[0];
				Vector3 currPos = middle::getShapePosition(gameState, cameraId.index);
				const Vector3 offset = Vector3{ 0, -350, 0 };
				Vector3 targetPos = middle::getShapePosition(gameState, shape.id.index) + offset;
				middle::moveShape(gameState, cameraId.index, targetPos - currPos);

			}

		}
	}
};

static middle::SystemRegistrar<LevelNavigationSystem> reg("LevelNavigationSystem");
