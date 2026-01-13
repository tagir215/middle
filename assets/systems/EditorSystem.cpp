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
#include "ComponentRefParent.h"
#include "Text.h"
#include "MouseIntersectable.h"

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
		float initAngle = PI * 0.1f;
		std::vector<Vector3> positions;
		const float r = 200;
		positions.resize(systemCount);
		for (int i = 0; i < systemCount; ++i) {
			float angle = initAngle + angleBetween * i;
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

			auto componentRefParent = middle::getComponent<components::ComponentRefParent>(shape);

			// if already has componentRef while selected skip TODO
			if (selectable->selected && componentRefParent) {
				return;
			}

			// if unselecteced while having a component skip and delete if theres componentrefparent
			if (!selectable->selected) {
				if (componentRefParent) {
					// delete child shapes
					for (int memberI : componentRefParent->indicatorChildren) {
						middle::deleteShape(gameState, memberI);
					}
					middle::deleteComponent<components::ComponentRefParent>(shape);
				}
				return;
			}

			// else create component ref
			auto newComponentRefParent = middle::addComponent<components::ComponentRefParent>(shape);

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

			// component Refs setup
			int componentCount = shape.componentMap.size();
			float angleBetween = 2 * PI / componentCount;
			std::vector<Vector3> positions;
			const float r = 40;
			const float compR = 3;
			float initAngle = PI / 10;

			positions.resize(componentCount);
			newComponentRefParent->indicatorChildren.resize(componentCount);

			int index = 0;
			for (auto pair : shape.componentMap) {
				int componentTypeId = pair.first;
				float angle = initAngle + angleBetween * index;
				float x = std::cosf(angle) * r;
				float z = std::sinf(angle) * r;
				Vector3 pos = { x,0,z };
				pos += entpos;
				std::string componentName = middle::componentNameMap[componentTypeId];

				// create entity
				int nextFreeIndex = middle::findNextFreeGhostIndex(gameState);
				middle::addShape(gameState, nextFreeIndex, middle::Shape());
				auto& newShape = middle::getShape(gameState, nextFreeIndex);
				middle::addComponent<components::MouseIntersectable>(newShape);
				auto newRef = middle::addComponent<components::ComponentReference>(newShape);
				auto newPos = middle::addComponent<components::Position>(newShape);
				auto newText = middle::addComponent<components::Text>(newShape);
				newRef->componentName = componentName;
				newPos->posX = pos.x;
				newPos->posY = pos.y;
				newPos->posZ = pos.z;
				newText->fontColorR = WHITE.r;
				newText->fontColorG = WHITE.g;
				newText->fontColorB = WHITE.b;
				newText->fontColorA = WHITE.a;
				newText->offsetX = 2;
				newText->offsetY = 0;
				newText->offsetZ = 10;
				newText->fontSize = middle::REF_TEXT_SIZE;
				newText->text = componentName;

				newComponentRefParent->indicatorChildren[index] = nextFreeIndex;
				++index;
			}
			});

	}
};

static middle::SystemRegistrar<EditorSystem> reg("EditorSystem");
