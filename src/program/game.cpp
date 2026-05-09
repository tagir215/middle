#pragma once

#include <iostream>
#include "game.h"
#include "descart_loop.h"
#include "SystemReference.h"
#include "Position.h"
#include "middle_shape_utils.h"
#include "engine_system_names.h"

using namespace middle;

namespace middle{

	void registerSystems(middle::GameState* gameState) {
		auto& systemMap = middle::getSystemMap();

		// register gameplay systems
		for (auto& pair : systemMap) {
			std::string name = pair.first;
			auto& sysptr = pair.second;

			sysptr->init(gameState);

			if (sysptr->systemUpdateType == SystemUpdateType::PREFRAME) {
				gameState->engineSystemsFrameStart.push_back(std::move(sysptr));
			}
			else if (sysptr->systemUpdateType == SystemUpdateType::GAMEPLAY_MIDFRAME) {
				gameState->gameplaySystems[name] = std::move(sysptr);
			}
			else if (sysptr->systemUpdateType == SystemUpdateType::GAMEPLAY_POSTFRAME) {
				gameState->gameplaySystemsPostFrame[name] = std::move(sysptr);
			}
			else if (sysptr->systemUpdateType == SystemUpdateType::RENDERING) {
				gameState->engineRendererSystems.push_back(std::move(sysptr));
			}

		}

		gameState->systemsRegistered = true;
	}


	void physicsUpdate(GameState* gameState) {

		for (auto& system : gameState->engineSystemsFrameStart) {

			if (gameState->applicationMode == ApplicationMode::GAME_MODE
				&& system->systemModeType == SystemModeType::EDITOR) {
				continue;
			}

			if (gameState->applicationMode == ApplicationMode::EDITOR_MODE
				&& system->systemModeType == SystemModeType::GAMEPLAY) {
				continue;
			}

			system->update(gameState);
		}

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

				system->update(gameState);
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

				system->update(gameState);
			}
			return true;
			});


		// Clear input blockers at the end of physics update
		gameState->inputBlockers.clear();

	}

}

extern "C" {

	__declspec(dllexport) void UpdateGame(GameState* gameState)
	{
		if (gameState->closeGame) {
			closeGame(gameState);
			return;
		}

		// TODO HARDCODED
		if (gameState->activeSceneName == "") {
			gameState->activeSceneName = "LevelSelect";
			loadScene(gameState, "../assets/scenes/", gameState->activeSceneName, false);
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

			renderSystem->update(gameState);
		}

		while (gameState->actionQueue.size() > 0) {
			gameState->actionQueue.front()->execute(gameState);
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


