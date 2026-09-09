#pragma once

#include <iostream>
#include "game.h"
#include "descart_loop.h"
#include "SystemReference.h"
#include "Position.h"
#include "middle_shape_utils.h"
#include "engine_system_names.h"
#include "bubble_paths.h"
#include "profiler_helpers.h"

using namespace middle;


namespace middle{

	void sortSystems(std::vector<std::unique_ptr<middle::MiddleGameplaySystem>>& systems) {
		std::vector<std::unique_ptr<middle::MiddleGameplaySystem>>tempVec;
		for (auto& s : systems) {
			tempVec.push_back(std::move(s));
		}
		systems.clear();
		for (int i = 0; i < middle::updatePriorityMax; ++i) {
			for (auto& s : tempVec) {
				if (!s) {
					continue;
				}
				if (s->updatePriority == i) {
					systems.push_back(std::move(s));
				}
			}
		}
	}


	void registerSystems(middle::GameState* gameState) {
		auto& systemMap = middle::getSystemMap();

		// register gameplay systems
		for (auto& pair : systemMap) {
			std::string name = pair.first;
			auto& sysptr = pair.second;

			sysptr->init(gameState);

			if (sysptr->systemUpdateType == SystemUpdateType::INITFRAME) {
				gameState->engineSystemInitFrame.push_back(std::move(sysptr));
			}
			else if (sysptr->systemUpdateType == SystemUpdateType::PREFRAME) {
				gameState->engineSystemsFrameStart.push_back(std::move(sysptr));
			}
			else if (sysptr->systemUpdateType == SystemUpdateType::GAMEPLAY_MIDFRAME) {
				gameState->gameplaySystems[name] = std::move(sysptr);
			}
			else if (sysptr->systemUpdateType == SystemUpdateType::GAMEPLAY_POSTFRAME) {
				gameState->gameplaySystemsPostFrame[name] = std::move(sysptr);
			}
			else if (sysptr->systemUpdateType == SystemUpdateType::POSTFRAME) {
				gameState->enginePostFrameSystems.push_back(std::move(sysptr));
			}
			else if (sysptr->systemUpdateType == SystemUpdateType::RENDERING) {
				gameState->engineRendererSystems.push_back(std::move(sysptr));
			}

		}

		sortSystems(gameState->engineSystemInitFrame);
		sortSystems(gameState->engineSystemsFrameStart);
		sortSystems(gameState->enginePostFrameSystems);
		sortSystems(gameState->engineRendererSystems);

		gameState->systemsRegistered = true;
	}

	void updateSystems(GameState* gameState, const std::vector<std::unique_ptr<MiddleGameplaySystem>>& systems) {
		for (auto& system : systems) {
			if (gameState->applicationMode == ApplicationMode::GAME_MODE
				&& system->systemModeType == SystemModeType::EDITOR) {
				continue;
			}

			if (gameState->applicationMode == ApplicationMode::EDITOR_MODE
				&& system->systemModeType == SystemModeType::GAMEPLAY) {
				continue;
			}

			gameState->activeSystemName = system->systemName;
			system->recordTimeUpdate(gameState);

			middleProfiling::reviewSystemTime(gameState, system.get());
		}
	}

	void physicsUpdate(GameState* gameState) {
		updateSystems(gameState, gameState->engineSystemInitFrame);
		updateSystems(gameState, gameState->engineSystemsFrameStart);

		if (!gameState->loaded) {
			return;
		}

		// run gameplay systems
		loopInstances(gameState, [gameState](int i, Shape& shape) {

			auto sysRef = getComponent<components::SystemReference>(shape);
			if (sysRef != nullptr) {
				auto systemName = sysRef->systemName;
				auto& system = gameState->gameplaySystems[systemName];

				if (!system)
					return true;

				if (gameState->applicationMode == ApplicationMode::GAME_MODE
					&& system->systemModeType == SystemModeType::EDITOR) {
					return true;
				}

				if (gameState->applicationMode == ApplicationMode::EDITOR_MODE
					&& system->systemModeType == SystemModeType::GAMEPLAY) {
					return true;
				}

				gameState->activeSystemName = system->systemName;
				system->recordTimeUpdate(gameState);

				middleProfiling::reviewSystemTime(gameState, system.get());
			}
			return true;
			});

		// run gameplay systems post frmae
		loopInstances(gameState, [gameState](int i, Shape& shape) {

			auto sysRef = getComponent<components::SystemReference>(shape);
			if (sysRef != nullptr) {
				auto systemName = sysRef->systemName;
				auto& system = gameState->gameplaySystemsPostFrame[systemName];

				if (!system)
					return true;

				if (gameState->applicationMode == ApplicationMode::GAME_MODE
					&& system->systemModeType == SystemModeType::EDITOR) {
					return true;
				}

				if (gameState->applicationMode == ApplicationMode::EDITOR_MODE
					&& system->systemModeType == SystemModeType::GAMEPLAY) {
					return true;
				}

				gameState->activeSystemName = system->systemName;
				system->recordTimeUpdate(gameState);

				middleProfiling::reviewSystemTime(gameState, system.get());
			}
			return true;
			});

		updateSystems(gameState, gameState->enginePostFrameSystems);

		// Clear input blockers at the end of physics update
		gameState->inputBlockers.clear();

	}
}

static bool initialized = false;

extern "C" {

	__declspec(dllexport) void UpdateGame(GameState* gameState)
	{

		if (!initialized) {
			for (auto& shape : gameState->shapes) {
				shape = middle::createShape(gameState);
			}
			initialized = true;
		}

		if (gameState->closeGame) {
			closeGame(gameState);
			return;
		}

		// TODO HARDCODED
		if (gameState->reset) {
			resetScene(gameState);
			loadScene(gameState, bubblePaths::SCENES_FOLDER, gameState->activeSceneName, false);
			gameState->reset = false;
		}

		if (!gameState->systemsRegistered) {
			registerSystems(gameState);
		}

		if (gameState->frameTimeAccumulator >= gameState->frameTime)
		{
			gameState->frameTimeAccumulator -= gameState->frameTime;
			if (gameState->frameTimeAccumulator > gameState->frameTime * 2) {
				gameState->frameTimeAccumulator = 0;
			}
			physicsUpdate(gameState);
		}

		for (auto& renderSystem : gameState->engineRendererSystems) {

			if (gameState->applicationMode == ApplicationMode::GAME_MODE
				&& renderSystem->systemModeType == SystemModeType::EDITOR) {
				continue;
			}

			if (gameState->applicationMode == ApplicationMode::EDITOR_MODE
				&& renderSystem->systemModeType == SystemModeType::GAMEPLAY) {
				continue;
			}

			gameState->activeSystemName = renderSystem->systemName;
			renderSystem->recordTimeUpdate(gameState);
		}

		while (gameState->actionQueue.size() > 0) {
			auto actionStart = std::chrono::high_resolution_clock::now();

			gameState->actionQueue.front()->execute(gameState);
			std::string caller = gameState->actionQueue.front()->callerSystem;

			auto actionEnd = std::chrono::high_resolution_clock::now();
			auto actionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(actionEnd - actionStart);
			float actionMs = actionDuration.count();
			if (actionMs > middleProfiling::slowActionThreshold) {
				gameState->slowActions.push_back("action from: " + caller + ": " + std::to_string(actionMs));
			}
			gameState->actionQueue.pop();
		}
		while (gameState->undoQueue.size() > 0) {
			gameState->undoQueue.front()->undo(gameState);
			gameState->undoQueue.pop();
		}

	}

}

void closeGame(GameState* gameState)
{
	saveEditorState(gameState);
}


