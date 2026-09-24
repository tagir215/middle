#pragma once
#include <iostream>
#include "game.h"
#include "SystemReference.h"
#include "Position.h"
#include "middle_shape_utils.h"
#include "engine_system_names.h"
#include "bubble_paths.h"
#include "profiler_helpers.h"
#include "config.h"

namespace middle{

	components::CompCache* cache;

	void sortSystems(std::vector<std::unique_ptr<middle::MiddleGameplaySystem>>& systems) {
		std::vector<std::unique_ptr<middle::MiddleGameplaySystem>>tempVec;
		for (auto& s : systems) {
			tempVec.push_back(std::move(s));
		}
		systems.clear();
		for (int i = 0; i <= middle::updatePriorityMax; ++i) {
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

		cache = middle::newCompCache(gameState, "main loop");
		cache->addType<components::SystemReference>();

		auto& systemMap = middle::getSystemMap();

		//gameState->componentCacheSystem = gameState->

		// register gameplay systems
		for (auto& pair : systemMap) {
			std::string name = pair.first;
			auto& sysptr = pair.second;

			sysptr->init(gameState);

			if (sysptr->systemUpdateType == SystemUpdateType::CACHE) {
				gameState->componentCacheSystem = std::move(sysptr);
			}
			else if (sysptr->systemUpdateType == SystemUpdateType::INITFRAME) {
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
			if (gameState->middleState.applicationMode == ApplicationMode::GAME_MODE
				&& system->systemModeType == SystemModeType::EDITOR) {
				continue;
			}

			if (gameState->middleState.applicationMode == ApplicationMode::EDITOR_MODE
				&& system->systemModeType == SystemModeType::GAMEPLAY) {
				continue;
			}

			gameState->activeSystemName = system->systemName;
			system->recordTimeUpdate(gameState);

			middleProfiling::reviewSystemTime(gameState, system.get());
		}
	}

	void cacheUpdate(GameState* gameState) {
		gameState->componentCacheSystem->recordTimeUpdate(gameState);
	}

	void processAnimations(GameState* gameState) {
		auto& animations = gameState->animations;
		for (int i = 0; i < animations.size(); ++i) {
			auto& animation = animations[i];
			animation->progressAnimation(gameState);
		}
		for (int i = animations.size() - 1; i >= 0; --i) {
			auto& animation = animations[i];
			if (animation->progress > animation->duration) {
				animations.erase(animations.begin() + i);
			}
		}
	}

	void processActionQueues(GameState* gameState) {
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

	void updateGameplaySystems(middle::GameState* gameState, std::unordered_map<std::string, std::unique_ptr<MiddleGameplaySystem>>& systemMap) {
		auto sysRefIt = cache->begin<components::SystemReference>();
		for (middle::Id id : cache->relevantIdVector) {
			auto sysRef = *sysRefIt;

			auto systemName = sysRef->systemName;
			auto& system = systemMap[systemName];

			if (!system)
				continue;

			if (gameState->middleState.applicationMode == ApplicationMode::GAME_MODE
				&& system->systemModeType == SystemModeType::EDITOR) {
				continue;
			}

			if (gameState->middleState.applicationMode == ApplicationMode::EDITOR_MODE
				&& system->systemModeType == SystemModeType::GAMEPLAY) {
				continue;
			}

			gameState->activeSystemName = system->systemName;
			system->recordTimeUpdate(gameState);

			middleProfiling::reviewSystemTime(gameState, system.get());
		}
	}

	void deterministicUpdate(GameState* gameState) {
		// init frame
		updateSystems(gameState, gameState->engineSystemInitFrame);

		processActionQueues(gameState);
		cacheUpdate(gameState);

		// preframe
		updateSystems(gameState, gameState->engineSystemsFrameStart);

		processActionQueues(gameState);
		cacheUpdate(gameState);

		if (!gameState->middleState.loaded) {
			return;
		}

		// gameplay midframe
		updateGameplaySystems(gameState, gameState->gameplaySystems);

		processActionQueues(gameState);
		cacheUpdate(gameState);

		// gameplay postframe
		updateGameplaySystems(gameState, gameState->gameplaySystemsPostFrame);

		processActionQueues(gameState);
		cacheUpdate(gameState);

		// animations
		processAnimations(gameState);

		// postframe
		updateSystems(gameState, gameState->enginePostFrameSystems);

		// Clear input blockers at the end of physics update
		gameState->middleState.inputBlockers.clear();

	}
}

static bool initialized = false;

extern "C" {

	__declspec(dllexport) void UpdateGame(middle::GameState* gameState)
	{

		if (!initialized) {
			for (auto& shape : gameState->shapes) {
				shape = middle::createShape(gameState);
			}
			initialized = true;
		}

		if (gameState->middleState.closeGame) {
			closeGame(gameState);
			return;
		}

		// TODO HARDCODED
		if (gameState->middleState.reset) {
			resetScene(gameState);
			loadScene(gameState, std::string(middlePaths::SCENES_FOLDER), gameState->activeSceneName, false);
			gameState->middleState.reset = false;
		}

		if (!gameState->systemsRegistered) {
			registerSystems(gameState);
		}

		float frameTime = gameState->middleState.frameTime;
		if (gameState->middleState.frameTimeAccumulator >= frameTime)
		{
			gameState->middleState.frameTimeAccumulator -= frameTime;;
			if (gameState->middleState.frameTimeAccumulator > frameTime * 2) {
				gameState->middleState.frameTimeAccumulator = 0;
			}
			deterministicUpdate(gameState);
		}

		processActionQueues(gameState);
		cacheUpdate(gameState);

		for (auto& renderSystem : gameState->engineRendererSystems) {

			if (gameState->middleState.applicationMode == middle::ApplicationMode::GAME_MODE
				&& renderSystem->systemModeType == middle::SystemModeType::EDITOR) {
				continue;
			}

			if (gameState->middleState.applicationMode == middle::ApplicationMode::EDITOR_MODE
				&& renderSystem->systemModeType == middle::SystemModeType::GAMEPLAY) {
				continue;
			}

			gameState->activeSystemName = renderSystem->systemName;
			renderSystem->recordTimeUpdate(gameState);
		}



	}

}

void closeGame(middle::GameState* gameState)
{
	saveEditorState(gameState);
}


