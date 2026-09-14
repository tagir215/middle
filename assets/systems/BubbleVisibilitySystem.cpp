#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "InViewTag.h"
#include "BubbleComponent.h"
#include "Layer.h"
#include "bubble_utils.h"
#include "TopDogInViewTag.h"
#include "TopDogBubbleTag.h"
#include "BubblePathMark.h"
#include "IntersectingTag.h"

class BubbleVisibilitySystem : public middle::MiddleGameplaySystem {
	components::CompCache* visibleCache;
	components::CompCache* invisibleCache;
	components::CompCache* topDogCache;

	const int visibilityDepth = 15;
	int bubbleVisitStamp = 1;

	void init(middle::GameState* gameState) override {

		systemUpdateType = middle::SystemUpdateType::POSTFRAME;
		systemModeType = middle::SystemModeType::ENGINE;
		// update before global coordinate calculation system
		updatePriority = 0;

		visibleCache = middle::newCompCache(gameState, systemName);
		visibleCache->addType<components::InViewTag>();
		visibleCache->addType<components::BubbleComponent>();
		visibleCache->addType<components::BubblePathMark>();
		visibleCache->addType<components::Layer>();
		invisibleCache = middle::newCompCache(gameState, systemName);
		invisibleCache->addType<components::InViewTag>(components::NOTINTERESTED);
		invisibleCache->addType<components::BubbleComponent>();
		invisibleCache->addType<components::BubblePathMark>();
		invisibleCache->addType<components::Layer>();
		topDogCache = middle::newCompCache(gameState, systemName);
		topDogCache->addType<components::TopDogBubbleTag>();
	}

	// update disappearing transform to avoid running out of floating point precision when zooming 
	void disappearTransforms(middle::GameState* gameState, middle::Id id) const{
		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);
		std::vector<Vector3>positions(children.size());
		std::vector<Vector3>scales(children.size());

		for (int i = 0; i < children.size(); ++i) {
			middle::Id childId = children[i];
			positions[i] = middle::getGlobalPosition(gameState, childId.index);
			scales[i] = middle::getGlobalScale(gameState, childId);
		}
		// set disappearing bubbles position to neutral
		Vector3 localScale = middle::getLocalScale(gameState, id);
		Vector3 localPos = middle::getLocalPosition(gameState, id);
		middle::setLocalPosition(gameState, id, { 0,0,0 });
		middle::setLocalScale(gameState, id, { 1,1,1 });

		// keep children global positions same
		for (int i = 0; i < children.size(); ++i) {
			middle::Id childId = children[i];
			middle::setLocalPosition(gameState, childId, positions[i]);
			middle::setLocalScale(gameState, childId, scales[i]);
		}

		// shouldn't be called,, but for safety
		middle::Id parentId = middle::getParent(gameState, id);
		if (parentId.index != middle::UNASSIGNED) {
			Vector3 parentPos = middle::getLocalPosition(gameState, parentId);
			Vector3 parentScale = middle::getLocalScale(gameState, parentId);
			disappearTransforms(gameState, parentId);
		}
		gameState->bubbleAlgebraState.worldScale = 1;
	}


	void appearingTransforms(middle::GameState* gameState, middle::Id id) {
		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);
		if (children.size() == 0) {
			return;
		}
		// random child
		middle::Id referenceId = children[0];
		for (middle::Id childId : children) {
			auto& shape = middle::getShape(gameState, childId.index);
			bool isIntersecting = middle::hasComp(shape, middle::getTypeId<components::IntersectingTag>());
			if (isIntersecting) {
				referenceId = childId;
				break;
			}
		}
		
		// childs global transform should stay same after transforming its parent
		Vector3 referenceGlobalPos = middle::getGlobalPosition(gameState, referenceId.index);
		Vector3 referenceGlobalScale = middle::getGlobalScale(gameState, referenceId);
		bubble::recursiveBubbleLayoutScaleUpdate(gameState, id);
		bubble::recursiveBubbleLayoutUpdate(gameState, id);

		Vector3 newGlobalScale = middle::getGlobalScale(gameState, referenceId);
		Vector3 ratio = Vector3Divide(referenceGlobalScale, newGlobalScale);
		Vector3 localScale = middle::getLocalScale(gameState, id);
		middle::setLocalScale(gameState, id, Vector3Multiply(localScale, ratio));

		Vector3 newGlobalPos = middle::getGlobalPosition(gameState, referenceId.index);
		Vector3 displacement = referenceGlobalPos - newGlobalPos;
		Vector3 localPos = middle::getLocalPosition(gameState, id);
		middle::setLocalPosition(gameState, id, localPos + displacement);

		gameState->bubbleAlgebraState.worldScale = 1;
	}

	void markPath(middle::GameState* gameState) {

		auto& traversePath = gameState->bubbleAlgebraState.traversePath;
		auto& traversePathIds = gameState->bubbleAlgebraState.traversePathIds;

		for (middle::Id id : topDogCache->relevantIdVector) {
			middle::Id currentId = id;
			auto pos = middle::getLocalPosition(gameState, id);
			auto sclae = middle::getLocalScale(gameState, id);

			// mark all on path
			for (int pathIndex : traversePath) {
				std::vector<middle::Id>children;
				middle::getChildren(gameState, currentId, children);
				currentId = children[pathIndex];
				auto markComp = middle::getComp<components::BubblePathMark>(gameState, currentId);
				markComp->stamp = bubbleVisitStamp;
			}
			// mark all branches under path
			std::stack<middle::Id>idStack;
			if (traversePathIds.size() > 0)
				idStack.push(gameState->bubbleAlgebraState.traversePathIds.back());
			else
				idStack.push(id);
			while (idStack.size() > 0) {
				currentId = idStack.top();
				idStack.pop();
				auto markComp = middle::getComp<components::BubblePathMark>(gameState, currentId);
				markComp->stamp = bubbleVisitStamp;
				std::vector<middle::Id>children;
				middle::getChildren(gameState, currentId, children);
				for (middle::Id childId : children) {
					idStack.push(childId);
				}
			}
		}
	}

	void update(middle::GameState* gameState) override {

		markPath(gameState);

		auto& traversePath = gameState->bubbleAlgebraState.traversePath;

		int firstVisibleLayer = 0;
		if (traversePath.size() > 0) {
			firstVisibleLayer = traversePath.size();
		}
		int lastVisibleLayer = firstVisibleLayer + visibilityDepth;

		{
			auto layerIt = visibleCache->begin<components::Layer>();
			auto markIt = visibleCache->begin<components::BubblePathMark>();
			for (middle::Id id : visibleCache->relevantIdVector) {
				auto layer = *layerIt;
				auto mark = *markIt;
				if (layer->layer < firstVisibleLayer || layer->layer > lastVisibleLayer || mark->stamp != bubbleVisitStamp) {
					middle::queueComponentDeletion<components::InViewTag>(gameState, id);
				}
				if (layer->layer == firstVisibleLayer -1) {
					auto& shape = getShape(gameState, id.index);
					disappearTransforms(gameState, id);
				}
			}
		}
		{
			auto layerIt = invisibleCache->begin<components::Layer>();
			auto markIt = invisibleCache->begin<components::BubblePathMark>();
			for (middle::Id id : invisibleCache->relevantIdVector) {
				auto layer = *layerIt;
				auto mark = *markIt;
				if (layer->layer > firstVisibleLayer && layer->layer <= lastVisibleLayer && mark->stamp == bubbleVisitStamp) {
					middle::attachComponent<components::InViewTag>(gameState, id);
				}
				if (layer->layer == firstVisibleLayer) {
					if (mark->stamp == bubbleVisitStamp) {
						middle::attachComponent<components::InViewTag>(gameState, id);
						appearingTransforms(gameState, id);
					}
				}
			}
		}

		++bubbleVisitStamp;
	}
};

static middle::SystemRegistrar<BubbleVisibilitySystem> reg("BubbleVisibilitySystem");
