#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "BubbleComponent.h"
#include "LocalScale.h"
#include "GlobalTransform.h"
#include "GlobalRect.h"
#include "IntersectingTag.h"
#include <stack>
#include "bubble_utils.h"
#include "middle_debug_utils.h"
#include "bubble_actions.h"
#include "equlab_actions.h"
#include "ActiveSceneEditableTag.h"
#include <queue>
#include "InViewTag.h"
#include "Layer.h"

class BubbleDynamicLoadingSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* intersectingBubbleCache;
	components::CompCache* activeCache;

	void init(middle::GameState* gameState) override {
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
		// run after topdog system

		intersectingBubbleCache = middle::newCompCache(gameState, systemName);
		intersectingBubbleCache->addType<components::BubbleComponent>();
		intersectingBubbleCache->addType<components::LocalScale>();
		intersectingBubbleCache->addType<components::GlobalTransform>();
		intersectingBubbleCache->addType<components::GlobalRect>();
		intersectingBubbleCache->addType<components::IntersectingTag>();

		activeCache = middle::newCompCache(gameState, systemName);
		activeCache->addType<components::BubbleComponent>();
		activeCache->addType<components::ActiveSceneSelectableTag>();
	}

	void update(middle::GameState* gameState) override {

		// update background id if not assigned to anything
		if (gameState->bubbleAlgebraState.backgroundBubbleId.index == middle::UNASSIGNED) {
			if (activeCache->relevantIdVector.size() == 1) {
				gameState->bubbleAlgebraState.backgroundBubbleId = activeCache->relevantIdVector[0];
			}
		}

		// prevent pushing and popping stuff into path continuously when at edge of pushing or popping..
		float thisIsImportantScalor = gameState->bubbleAlgebraState.worldScalarRate > 1 ? 1 : 1.3f;

		const float disappearingBubbleHideFactor = 20.2f;
		float screenWidthInWorldCoords = 
			gameState->middleState.nearPlaneAxisX / gameState->middleState.nearPlaneDistance * (-gameState->middleState.activeCamera.position.y)  * disappearingBubbleHideFactor * thisIsImportantScalor;

		// find current position id
		middle::Id localPathEndId;
		float minWidth = std::numeric_limits<float>::max();
		auto globalRectIt = intersectingBubbleCache->begin<components::GlobalRect>();
		for (middle::Id id : intersectingBubbleCache->relevantIdVector) {
			auto globalRect = *globalRectIt;

			if (globalRect->width > screenWidthInWorldCoords && globalRect->width < minWidth) {
				minWidth = globalRect->width;
				localPathEndId = id;
			}
		}

		if (localPathEndId.index == middle::UNASSIGNED) {
			if (gameState->bubbleAlgebraState.traversePath.size() > 0) {
				gameState->bubbleAlgebraState.traversePath.pop_back();
				gameState->bubbleAlgebraState.traversePathIds.pop_back();
			}
			return;
		}

		std::stack<int>indexes;
		std::stack<middle::Id>ids;
		middle::Id end = localPathEndId;

		while (end.index != middle::UNASSIGNED) {
			middle::Id parentId = middle::getParent(gameState, end);
			if (parentId.index != middle::UNASSIGNED) {
				indexes.push(middle::getLoopIndex(gameState, end));
				ids.push(end);
			}
			end = parentId;
		}

		auto& traversePath = gameState->bubbleAlgebraState.traversePath;
		auto& traversePathIds = gameState->bubbleAlgebraState.traversePathIds;
		std::vector<int>path;
		std::vector<middle::Id>pathIds;
		traversePath.clear();
		traversePathIds.clear();
		while (indexes.size() > 0) {
			traversePath.push_back(indexes.top());
			traversePathIds.push_back(ids.top());
			indexes.pop();
			ids.pop();
		}

		middle::drawImGuiIntVector(gameState, "traversePath", gameState->bubbleAlgebraState.traversePath);
		//middle::drawImGuiInt(gameState, "intersecting count", intersectingBubbleCache->relevantIdVector.size());
	}
};

static middle::SystemRegistrar<BubbleDynamicLoadingSystem> reg("BubbleDynamicLoadingSystem");
