#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "SceneObjectComponent.h"
#include "TopDogBubbleTag.h"
#include "BubbleComponent.h"
#include "component_utils.h"
#include "ActiveSceneEditableTag.h"

class SceneEditorSelectionSystem : public middle::MiddleGameplaySystem {
	components::CompCache* topDogCache;
	components::CompCache* sceneObjCache;
	components::CompCache* activeBubbleCache;

	void init(middle::GameState* gameState) override {
		topDogCache = middle::newCompCache(gameState, systemName);
		topDogCache->addType<components::TopDogBubbleTag>();
		sceneObjCache = middle::newCompCache(gameState, systemName);
		sceneObjCache->addType<components::SceneObjectComponent>();
		activeBubbleCache = middle::newCompCache(gameState, systemName);
		activeBubbleCache->addType<components::ActiveSceneSelectableTag>();
	}

	void update(middle::GameState* gameState) override {

		// ACTIVITY UPDATE
		float minDistance = std::numeric_limits<float>::max();
		middle::Id closestId;
		// FIND CLOSEST FROM BUBBLES
		for (middle::Id& id : topDogCache->relevantIdVector) {
			middle::Id parentId = middle::getParent(gameState, id);
			middle::Id targetId;
			Vector3 center;
			if (parentId.index != middle::UNASSIGNED) {
				std::vector<middle::Id>children;
				middle::getChildren(gameState, parentId, children);
				assert(children.size() == 2);
				middle::Id idA = children[0];
				middle::Id idB = children[1];
				center = (middle::getShapePosition(gameState, idA.index) +
					middle::getShapePosition(gameState, idB.index)) * 0.5f;
				targetId = parentId;
			}
			else {
				center = middle::getShapePosition(gameState, id.index);
				targetId = id;
			}

			float distSqr = Vector3DistanceSqr(center, gameState->activeCamera.position);
			if (distSqr < minDistance) {
				minDistance = distSqr;
				closestId = targetId;
			}
		}
		// FIND CLOSEST FROM SCENE OBJECTS
		for (middle::Id& id : sceneObjCache->relevantIdVector) {
			Vector3 pos = middle::getShapePosition(gameState, id.index);
			float distSqr = Vector3DistanceSqr(pos, gameState->activeCamera.position);
			if (distSqr < minDistance) {
				minDistance = distSqr;
				closestId = id;
			}
		}
		// UPDATE ACTIVE TO CLOSEST TO CAMERA
		if (closestId.index != middle::UNASSIGNED) {

			if (activeBubbleCache->relevantIdVector.size() > 0) {
				bool needUpdate = false;
				for (middle::Id& activeId : activeBubbleCache->relevantIdVector) {
					if (activeId != closestId) {
						middle::queueComponentDeletion<components::ActiveSceneSelectableTag>(gameState, activeId);
						needUpdate = true;
					}
				}
				if (needUpdate) {
					middle::queueComponentAttachment<components::ActiveSceneSelectableTag>(gameState, closestId);
				}
			}
			else if (activeBubbleCache->relevantIdVector.size() == 0) {
				middle::queueComponentAttachment<components::ActiveSceneSelectableTag>(gameState, closestId);
			}
		}

	}
};

static middle::SystemRegistrar<SceneEditorSelectionSystem> reg("SceneEditorSelectionSystem");
