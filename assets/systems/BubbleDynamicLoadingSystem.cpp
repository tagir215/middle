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

class BubbleDynamicLoadingSystem : public middle::MiddleGameplaySystem {
	components::CompCache* intersectingBubbleCache;

	void init(middle::GameState* gameState) override {
		intersectingBubbleCache = middle::newCompCache(gameState, systemName);
		intersectingBubbleCache->addType<components::BubbleComponent>();
		intersectingBubbleCache->addType<components::LocalScale>();
		intersectingBubbleCache->addType<components::GlobalTransform>();
		intersectingBubbleCache->addType<components::GlobalRect>();
		intersectingBubbleCache->addType<components::IntersectingTag>();
	}
	void update(middle::GameState* gameState) override {
		const float screenWidthInWorldCoords = 
			gameState->nearPlaneAxisX / gameState->nearPlaneDistance * (-gameState->activeCamera.position.y)  * 0.2f;

		// find current position id
		middle::Id loadPositionId;
		float minWidth = std::numeric_limits<float>::max();
		auto globalRectIt = intersectingBubbleCache->begin<components::GlobalRect>();
		for (middle::Id id : intersectingBubbleCache->relevantIdVector) {
			auto globalRect = *globalRectIt;

			if (globalRect->width > screenWidthInWorldCoords && globalRect->width < minWidth) {
				minWidth = globalRect->width;
				loadPositionId = id;
			}
		}

		if (loadPositionId.index == middle::UNASSIGNED) {
			return;
		}

		// find ids of the path
		std::stack<middle::Id>pathStack;
		pathStack.push(loadPositionId);
		while (true) {
			middle::Id currentId = pathStack.top();
			middle::Id parentId = middle::getParent(gameState, currentId);
			if (parentId.index != middle::UNASSIGNED) {
				pathStack.push(parentId);
			}
			else {
				break;
			}
		}
		std::vector<middle::Id>pathIds;
		while (pathStack.size() > 0) {
			pathIds.push_back(pathStack.top());
			pathStack.pop();
		}

		// store indexes of children
		std::vector<int>resultPath;
		for (int i = 0; i < pathIds.size() - 1; ++i) {
			middle::Id id = pathIds[i];
			middle::Id nextId = pathIds[i + 1];
			std::vector<middle::Id>children;
			middle::getChildren(gameState, id, children);
			for (int childIndex = 0; childIndex < children.size(); ++childIndex) {
				middle::Id childId = children[childIndex];
				if (childId == nextId) {
					resultPath.push_back(childIndex);
					break;
				}
			}
		}

		auto& traversePath = gameState->bubbleAlgebraState.traversePath;
		int travelledLength = traversePath.size();

		// free
		if (resultPath.size() > 1) {
			traversePath.push_back(resultPath.front());
			middle::Id parentId = middle::getParent(gameState, loadPositionId);
			auto freeParentAction = std::make_shared<equlab::FreeParent>(parentId);
			middle::queueAction(gameState, freeParentAction);

		}

		// load
		else if (resultPath.size() < 1 && gameState->bubbleAlgebraState.traversePath.size() > 0) {
			auto loadParentAction = std::make_shared<equlab::LoadParent>(loadPositionId);
			middle::queueAction(gameState, loadParentAction);
		}


		middle::drawImGuiIntVector(gameState, "localPath", resultPath);

		middle::drawImGuiIntVector(gameState, "traversePath", gameState->bubbleAlgebraState.traversePath);
	}
};

static middle::SystemRegistrar<BubbleDynamicLoadingSystem> reg("BubbleDynamicLoadingSystem");
