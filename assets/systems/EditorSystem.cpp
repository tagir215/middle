#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "editor_actions.h"
#include "editor_file_utils.h"
#include "middle_shape_utils.h"
#include "middle_gameplay_script_map.h"
#include "LoopTag.h"
#include "SystemEntity.h"
#include "ComponentReference.h"
#include "ComponentRefParent.h"
#include "Text.h"
#include "MouseIntersectable.h"
#include "engine_system_names.h"

class EditorSystem : public middle::MiddleGameplaySystem {
public:
	EditorSystem() {
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
		systemModeType = middle::SystemModeType::EDITOR;
	}
	void init(middle::GameState* gameState) {

	}

	void reset(middle::GameState* gameState) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			middle::deleteShape(gameState, i);
		}
	}

	// for mental palace. Palace is superior filesystem! TRUST THE PALACE
	void importEngineSystemReferences(middle::GameState* gameState) {
		int highestUsedIndex = middle::findHighestUsedIndex(gameState);
		int index = highestUsedIndex + 1;
		index = highestUsedIndex > middle::GHOST_INDEX_OFFSET ? highestUsedIndex : middle::GHOST_INDEX_OFFSET;
		std::vector<std::string>systemNames = middle::engineSystemNames;

		int systemCount = systemNames.size();
		float angleBetween = PI / systemCount;
		float initAngle = PI * 0.1f;
		float yCoord = 200;
		std::vector<Vector3> positions;
		const float r = 200;
		positions.resize(systemCount);
		for (int i = 0; i < systemCount; ++i) {
			float angle = initAngle + angleBetween * i;
			float x = std::cosf(angle) * r;
			float z = std::sinf(angle) * r;
			Vector3 pos = { x,yCoord,z };
			entities::initSystem(gameState, index + i, pos, systemNames[i]);
		}
	}

	void update(middle::GameState* gameState) override {

		if (gameState->startGame) {
			if (gameState->applicationMode == middle::ApplicationMode::EDITOR_MODE) {
				middle::loadEditorState(gameState);
			}
			loadSceneAndShapeNames(gameState);
			loadSystemNames(gameState);
			loadComponentNames(gameState);
			gameState->startGame = false;

			middle::queueAction(gameState, std::make_shared<middle::CustomAction>(
				[](middle::GameState* gameState) { gameState->loaded = true; })
			);
		}


		// update
		if (gameState->reload) {
			reset(gameState);
			importEngineSystemReferences(gameState);
			if (gameState->sceneNames.size() > 0) {
				loadScene(gameState, "../assets/scenes/", gameState->activeSceneName, false);
			}
			gameState->reload = false;
		}




	}
};

static middle::SystemRegistrar<EditorSystem> reg("EditorSystem");
