#pragma once
#include "game_state.h"
#include "registrars.h"
#include "editor_actions.h"
#include "editor_file_utils.h"
#include "middle_shape_utils.h"
#include "middle_gameplay_script_map.h"
#include "LoopTag.h"
#include "SystemEntity.h"
#include "ComponentReference.h"

class EditorSystem : public middle::MiddleGameplaySystem {

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
		std::vector<std::string>systemNames;
		systemNames.insert(systemNames.end(), middle::engineSystemNamesFrameStart.begin(), middle::engineSystemNamesFrameStart.end());
		systemNames.insert(systemNames.end(), middle::engineSystemNamesFrameEnd.begin(), middle::engineSystemNamesFrameEnd.end());
		systemNames.insert(systemNames.end(), middle::engineRendererSystemNames.begin(), middle::engineRendererSystemNames.end());

		int systemCount = systemNames.size();
		float angleBetween = PI / systemCount;
		std::vector<Vector3> positions;
		const float r = 50;
		positions.resize(systemCount);
		for (int i = 0; i < systemCount; ++i) {
			float angle = angleBetween * i;
			float x = std::cosf(angle) * r;
			float z = std::sinf(angle) * r;
			Vector3 pos = { x,0,z };
			entities::initSystem(gameState, index + i, pos, systemNames[i]);
		}
	}


	void update(middle::GameState* gameState) override {
		processEditorActions(gameState);

		if (gameState->startGame) {
			if (gameState->applicationMode == middle::ApplicationMode::EDITOR_MODE) {
				middle::loadEditorState(gameState);
			}
			gameState->startGame = false;
		}

		// update
		if (gameState->reload) {
			reset(gameState);
			loadSceneNames(gameState);
			loadSystemNames(gameState);
			importEngineSystemReferences(gameState);
			loadComponentNames(gameState);
			if (gameState->sceneNames.size() > 0) {
				loadScene(gameState, gameState->sceneNames[gameState->activeScene], false);
			}
			gameState->reload = false;
		}


		// camera controls
		const float maxCameraSpeed = 60;
		float mouseCamRatio = gameState->input.mousePos.y / gameState->screenHeight;
		const float cameraSpeed = mouseCamRatio * mouseCamRatio * mouseCamRatio * maxCameraSpeed;
		Vector3 cameraMovementDir = { 0,0,0 };
		if (gameState->input.w)
			cameraMovementDir += Vector3Normalize(gameState->editorState.camera.target - gameState->editorState.camera.position);
		if (gameState->input.s)
			cameraMovementDir += Vector3Negate(Vector3Normalize(gameState->editorState.camera.target - gameState->editorState.camera.position));
		if (gameState->input.e)
			cameraMovementDir += { 0, 0, 1 };
		if (gameState->input.q)
			cameraMovementDir += { 0, 0, -1 };
		if (gameState->input.d)
			cameraMovementDir += Vector3Negate(Vector3Normalize(Vector3CrossProduct(gameState->editorState.camera.up, gameState->editorState.camera.target - gameState->editorState.camera.position)));
		if (gameState->input.a)
			cameraMovementDir += Vector3Normalize(Vector3CrossProduct(gameState->editorState.camera.up, gameState->editorState.camera.target - gameState->editorState.camera.position));

		gameState->editorState.camera.position += cameraMovementDir * cameraSpeed;
		gameState->editorState.camera.target += cameraMovementDir * cameraSpeed;


		// COMPONENT REFERENCE PALACE
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto selectable = middle::getComponent<components::MouseSelectable>(shape);

			if (!selectable) {
				return;
			}

			auto componentRef = middle::getComponent<components::ComponentReference>(shape);

			// if already has componentRef while selected skip
			if (selectable->selected && componentRef) {
				return;
			}

			// if unselecteced while having a component skip
			if (!selectable->selected) {
				if (componentRef) {
					middle::deleteComponent<components::ComponentReference>(shape);
				}
				return;
			}

			// else create component ref

			auto newComponentRef = middle::addComponent<components::ComponentReference>(shape);

			// get position
			auto position = middle::getComponent<components::Position>(shape);
			auto loop = middle::getComponent<components::LoopTag>(shape);
			Vector3 entpos;
			if (position) {
				entpos = { position->posX, position->posY, position->posZ };
			}
			if (loop) {
				entpos = middle::getLoopCentroid(gameState, i);
			}


			// component Ref setup
			int componentCount = shape.componentMap.size();
			float angleBetween = 2 * PI / componentCount;
			std::vector<Vector3> positions;
			const float r = 30;
			const float compR = 3;

			positions.resize(componentCount);
			newComponentRef->componentNames.resize(componentCount);
			newComponentRef->positionsX.resize(componentCount);
			newComponentRef->positionsY.resize(componentCount);
			newComponentRef->positionsZ.resize(componentCount);

			int index = 0;
			for (auto pair : shape.componentMap) {
				int componentTypeId = pair.first;
				float angle = angleBetween * index;
				float x = std::cosf(angle) * r;
				float z = std::sinf(angle) * r;
				Vector3 pos = { x,0,z };
				pos += entpos;
				std::string componentName = middle::componentNameMap[componentTypeId];
				newComponentRef->componentNames[index] = componentName;
				newComponentRef->positionsX[index] = pos.x;
				newComponentRef->positionsY[index] = pos.y;
				newComponentRef->positionsZ[index] = pos.z;
				++index;
			}
			});

	}
};

static middle::SystemRegistrar<EditorSystem> reg("EditorSystem");
