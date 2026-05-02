#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MouseClickComponent.h"
#include "Button.h"
#include "BubbleAlgebraProblem.h"
#include "bubble_actions.h"
#include "bubble_utils.h"
#include "BubbleAlgebraProblemContainer.h"
#include "Rectangle.h"
#include "BubbleAlgebraLevelConfigs.h"
#include "CameraComponent.h"
#include "Position.h"
#include "ProcedureContainer.h"
#include "bubble_constants.h"
#include "ProcedureInputVariable.h"
#include "editor_file_utils.h"


class AlgebraProblemSystem : public middle::MiddleGameplaySystem {
public:


	components::CompCache* cache = nullptr;
	components::CompCache* problemCache = nullptr;
	components::CompCache* containerCache = nullptr;
	components::CompCache* levelCache = nullptr;
	components::CompCache* cameraCache = nullptr;
	components::CompCache* procContainerCache = nullptr;

	void init(middle::GameState* gameState) {
		cache = middle::newCompCache(gameState);
		cache->addType<components::MouseClickComponent>();
		cache->addType<components::Button>();
		containerCache = middle::newCompCache(gameState);
		containerCache->addType<components::BubbleAlgebraProblemContainer>();
		containerCache->addType<components::Position>();
		problemCache = middle::newCompCache(gameState);
		problemCache->addType<components::BubbleAlgebraProblem>();
		problemCache->addType<components::Position>();
		levelCache = middle::newCompCache(gameState);
		levelCache->addType<components::BubbleAlgebraLevelConfigs>();
		cameraCache = middle::newCompCache(gameState);
		cameraCache->addType<components::CameraComponent>();
		cameraCache->addType<components::Position>();
		procContainerCache = middle::newCompCache(gameState);
		procContainerCache->addType<components::ProcedureContainer>();
	}

	void undo(middle::GameState* gameState) {

		// return if procedure executing
		if (procContainerCache->getSize() > 0) {
			auto containerIt = procContainerCache->begin<components::ProcedureContainer>();
			auto container = *containerIt;
			if (container->targetActionStackSize > 0) {
				return;
			}
		}

		if (gameState->bubbleAlgebraState.bubbleActions.size() > 0) {
			middle::queueAction(gameState, std::make_shared<middle::CustomAction>([](middle::GameState* gameState) {
				gameState->bubbleAlgebraState.bubbleActions.back()->undo(gameState);
				gameState->bubbleAlgebraState.bubbleActions.pop_back();
				}));

			if (levelCache->getSize() > 0) {
				auto configsIt = levelCache->begin<components::BubbleAlgebraLevelConfigs>();
				auto configs = *configsIt;
				++configs->allowedMoves;
			}
		}
		queueSound(gameState, bubbleSounds::UNDO_SOUND);
	}

	bool initialized = false;

	void update(middle::GameState* gameState) override {

		if (gameState->gameInput.undo) {
			undo(gameState);
		}

		auto procedureIt = procContainerCache->begin<components::ProcedureContainer>();
		middle::Id procContainerId;
		if (procContainerCache->getSize() == 1) {
			procContainerId = procContainerCache->relevantIdVector[0];
		}


		auto buttonIt = cache->begin<components::Button>();
		int size = cache->getSize();
		for (int i = 0; i < size; ++i) {
			auto button = *buttonIt;
			if (button->function == bubbleButton::DONE || button->function == bubbleButton::SAVE_PROCEDURE_BUTTON) {
				std::vector<middle::Id> formulas;
				assert(problemCache->getSize() == 2);
				middle::Id& formulaA = problemCache->relevantIdVector[0];
				middle::Id& formulaB = problemCache->relevantIdVector[1];
				bool matching = bubble::matchingBubbles(gameState, formulaA, formulaB);

				if (matching) {
					middle::loadShape(gameState, "../assets/shapes/", "ScoreScreen", true);
					gameState->bubbleAlgebraState.justCompletedLevel = true;
					gameState->bubbleAlgebraState.completedLevelName = gameState->activeSceneName;

					if (button->function == bubbleButton::SAVE_PROCEDURE_BUTTON) {
						auto& procShape = middle::getShape(gameState, procContainerId.index);
						auto text = middle::getComponent<components::Text>(procShape);

						auto levelConfigsIt = levelCache->begin<components::BubbleAlgebraLevelConfigs>();
						auto configsComp = *levelConfigsIt;
						assert(configsComp);

						middle::saveShape(gameState, procContainerId, "../bubbleData/procedures/", configsComp->levelName);
					}
					queueSound(gameState, bubbleSounds::VICTORY_SOUND);
				}
			}


			if (button->function == bubbleButton::UNDO) {
				undo(gameState);
			}
		}


		auto levelConfigsIt = levelCache->begin<components::BubbleAlgebraLevelConfigs>();
		if (levelCache->getSize() > 0) {
			auto configs = *levelConfigsIt;
			if (!configs->initialized) {

				float problemCenterX = 0;

				if (containerCache->getSize() == 1) {
					auto containerPosIt = containerCache->begin<components::Position>();
					auto containerPos = *containerPosIt;

					const float minMargin = 40;

					int problemIndex = -1;
					auto posIt = problemCache->begin<components::Position>();
					for (int i = 0; i < problemCache->getSize(); ++i) {
						auto pos = *posIt;
						if (pos->posX > containerPos->posX - minMargin) {
							problemIndex = i;
						}

						problemCenterX += pos->posX;

						//float deltaZ = pos->posZ;
						float deltaZ = 0;
						//middle::moveShape(gameState, problemCache->relevantIdVector[i].index, { 0,0, -deltaZ });
					}
				}


				//problemCenterX /= 2.0f;
				//auto cameraPosIt = cameraCache->begin<components::Position>();
				//auto camPos = *cameraPosIt;
				//camPos->posX = problemCenterX;
				//camPos->posZ = 0;
				configs->initialized = true;
			}
		}
	}

};

static middle::SystemRegistrar<AlgebraProblemSystem> reg("AlgebraProblemSystem");
