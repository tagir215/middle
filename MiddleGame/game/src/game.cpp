#pragma once
#include <iostream>
#include "game.h"
#include "MidComp/SystemReference.h"
#include "MidComp/Position.h"
#include "middle_shape_utils.h"
#include "engine_system_names.h"
#include "bubble_paths.h"
#include "profiler_helpers.h"
#include "midconfig.h"

namespace middle{

	components::CompCache* cache;


	//TODO move
	void updateMouseStuff(middle::GameState* gameState) {
		// TODO MOVE THESE
		// CAMERA POSITION UPDATE
		auto& inputState = gameState->middleInputState;
		auto& mouseState = gameState->mouseState;
		int cameraPosX = inputState.screenWidth / 2;
		int cameraPosY = inputState.screenHeight / 2;

		midMath::Matrix scalorM = midMath::MatrixScale(1, -1, 1);
		midMath::Matrix translatorM = midMath::MatrixTranslate(0, inputState.screenHeight, 0);
		midMath::Matrix screenOrientorM = midMath::MatrixMultiply(scalorM, translatorM);
		midMath::Vector3 invertedMouse = midMath::Vector3Transform({inputState.editorInput.mouseX, inputState.editorInput.mouseY, 0}, screenOrientorM);
		mouseState.mousePos.x = invertedMouse.x;
		mouseState.mousePos.y = invertedMouse.y;

		// MOUSE POSITION UPDATE
		int relativeX = mouseState.mousePos.x - cameraPosX;
		int relativeY = mouseState.mousePos.y - cameraPosY;
		mouseState.mouseNormalizedPos.x = (float)relativeX / (float)cameraPosX;
		mouseState.mouseNormalizedPos.y = (float)relativeY / (float)cameraPosX;
		gameState->aspectRatio = inputState.screenWidth / inputState.screenHeight;
		float angle = gameState->middleState.activeCamera.fovy * (PI / 180)  * 0.5f;

		// todo move {
		float nearAxisY = tan(angle) * gameState->middleState.nearPlaneDistance;
		float nearAxisX = nearAxisY * gameState->aspectRatio;
		gameState->nearPlaneAxisY = nearAxisY;
		gameState->nearPlaneAxisX = nearAxisX;
		// }

		float nearPlanePos2dX = nearAxisX * mouseState.mouseNormalizedPos.x;
		float nearPlanePos2dY = nearAxisX * mouseState.mouseNormalizedPos.y;

		auto& camera = gameState->middleState.activeCamera;
		midMath::Vector3 cameraDir = Vector3Normalize(camera.target - camera.position);
		midMath::Vector3 cameraRight = Vector3Normalize(Vector3CrossProduct(cameraDir, camera.up));
		midMath::Vector3 cameraUp = Vector3CrossProduct(cameraRight, cameraDir);
		midMath::Vector3 nearPlanePos = camera.position
			+ cameraDir * gameState->middleState.nearPlaneDistance
			+ cameraRight * nearPlanePos2dX
			+ cameraUp * nearPlanePos2dY;
		midMath::Vector3 mouseDir = midMath::Vector3Normalize(mouseState.mouseNearPlanePos - gameState->middleState.activeCamera.position);
		mouseState.mouseNearPlanePos = { nearPlanePos.x, nearPlanePos.y, nearPlanePos.z };
		mouseState.mouseDir = { mouseDir.x, mouseDir.y, mouseDir.z };

		midMath::Vector3 xzPlanePos = { 0,0,0 };
		midMath::Vector3 xzPlaneNormal = { 0,-1,0 };
		midMath::Vector3 previousXZ_PlanePos = gameState->mouseState.mouseXZ_PlanePos;
		midMath::Vector3 nextXZ_PlanePos = midMath::RayCastLinePlane(xzPlanePos, xzPlaneNormal, mouseState.mouseNearPlanePos, mouseState.mouseDir);
		mouseState.mouseXZ_PlanePos = nextXZ_PlanePos;
		mouseState.mouseXZ_PlaneVelocity = (nextXZ_PlanePos - previousXZ_PlanePos) / inputState.frameTime;
	}

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

	void deterministicUpdate(GameState* gameState) {
		// init frame
		updateSystems(gameState, gameState->engineSystemInitFrame);

		processActionQueues(gameState);
		cacheUpdate(gameState);

		// preframe
		updateSystems(gameState, gameState->engineSystemsFrameStart);

		processActionQueues(gameState);
		cacheUpdate(gameState);

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

static bool gameStateInitialized = false;
std::unique_ptr<middle::GameState>gameState;

extern "C" {

	__declspec(dllexport) void UpdateGame(const middle::MiddleInputState& inputState, middle::MiddleOutputState** outputState)
	{
		if (!gameStateInitialized) {
			gameState = std::make_unique<middle::GameState>();
			for (auto& shape : gameState->shapes) {
				shape = middle::createShape(gameState.get());
			}
			gameStateInitialized = true;
		}

		gameState->middleInputState = inputState;

		gameState->middleState.renderData.clear();
		gameState->middleState.uiSetups.clear();
		gameState->debugInfo.clear();

		updateMouseStuff(gameState.get());

		if (inputState.closeGame) {
			closeGame(gameState.get());
			return;
		}

		// TODO HARDCODED
		// NOTE EDITOR SYSTEM DOES SOME INITIALIZATION CURRENTLY
		if (gameState->reset) {
			resetScene(gameState.get());
			loadScene(gameState.get(), std::string(middlePaths::SCENES_FOLDER), gameState->activeSceneName, false);
			gameState->loaded = true;
		}

		if (!gameState->systemsRegistered) {
			registerSystems(gameState.get());
		}

		float frameTime = inputState.frameTime;
		if (inputState.frameTimeAccumulator >= frameTime)
		{
			gameState->middleState.frameTimeAccumulator = inputState.frameTimeAccumulator - frameTime;
			if (inputState.frameTimeAccumulator > frameTime * 2) {
				gameState->middleState.frameTimeAccumulator = 0;
			}
			deterministicUpdate(gameState.get());
		}

		processActionQueues(gameState.get());
		cacheUpdate(gameState.get());

		for (auto& renderSystem : gameState->engineRendererSystems) {

			if (gameState->applicationMode == middle::ApplicationMode::GAME_MODE
				&& renderSystem->systemModeType == middle::SystemModeType::EDITOR) {
				continue;
			}

			if (gameState->applicationMode == middle::ApplicationMode::EDITOR_MODE
				&& renderSystem->systemModeType == middle::SystemModeType::GAMEPLAY) {
				continue;
			}

			gameState->activeSystemName = renderSystem->systemName;
			renderSystem->recordTimeUpdate(gameState.get());
		}


		// UPDATE OUTPUT FOR FRONT END
		*outputState = &gameState->middleState;
	}

}

void closeGame(middle::GameState* gameState)
{
	saveEditorState(gameState);
}


