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

class BubbleDynamicLoadingSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* intersectingBubbleCache;
	components::CompCache* activeCache;
	BubbleDynamicLoadingSystem() {
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;
	}

	void init(middle::GameState* gameState) override {
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


		const float screenWidthInWorldCoords = 
			gameState->nearPlaneAxisX / gameState->nearPlaneDistance * (-gameState->activeCamera.position.y)  * 0.2f;

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
			return;
		}

		// find ids of the path
		std::stack<middle::Id>pathStack;
		pathStack.push(localPathEndId);
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
		std::queue<int>localPathIndexQueue;
		for (int i = 0; i < pathIds.size() - 1; ++i) {
			middle::Id id = pathIds[i];
			middle::Id nextId = pathIds[i + 1];

			std::vector<middle::Id>children;
			middle::getChildren(gameState, id, children);
			for (int childIndex = 0; childIndex < children.size(); ++childIndex) {
				middle::Id childId = children[childIndex];
				if (childId == nextId) {

					localPathIndexQueue.push(childIndex);
					break;
				}
			}
		}

		auto& traversePath = gameState->bubbleAlgebraState.traversePath;
		middle::Id& backgroundId = gameState->bubbleAlgebraState.backgroundBubbleId;
		int travelledLength = traversePath.size();

		// free
		if (localPathIndexQueue.size() > 1) {

			int scaleReferenceIndex = localPathIndexQueue.back();
			middle::Id scaleReferenceId = pathIds.back();

			while (localPathIndexQueue.size() > 1) {
				traversePath.push_back(localPathIndexQueue.front());
				localPathIndexQueue.pop();
			}

			if (traversePath.size() > 0) {
				auto loadParentAction = std::make_shared<equlab::LoadBubbleSection>(
					scaleReferenceId, scaleReferenceIndex, traversePath);
				middle::queueAction(gameState, loadParentAction);
			}
		}

		// load
		else if (localPathIndexQueue.size() < 1 && traversePath.size() > 0) {

			int scaleReferenceIndex = traversePath.back();
			middle::Id scaleReferenceId = gameState->bubbleAlgebraState.backgroundBubbleId;

			gameState->bubbleAlgebraState.traversePath.pop_back();

			auto loadParentAction = std::make_shared<equlab::LoadBubbleSection>(
				scaleReferenceId, scaleReferenceIndex, gameState->bubbleAlgebraState.traversePath);

			middle::queueAction(gameState, loadParentAction);
		}


		//middle::drawImGuiIntVector(gameState, "localPath", resultPath);

		middle::drawImGuiIntVector(gameState, "traversePath", gameState->bubbleAlgebraState.traversePath);
	}
};

static middle::SystemRegistrar<BubbleDynamicLoadingSystem> reg("BubbleDynamicLoadingSystem");
