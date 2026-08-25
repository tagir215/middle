#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "LoopSociety.h"
#include "BubbleComponent.h"
#include "bubble_utils.h"
#include "MouseGrabbable.h"
#include "MouseIntersectable.h"
#include "BubbleUnit.h"
#include "FractionalComponent.h"
#include "middle_math.h"
#include "Sphere.h"
#include "Rectangle.h"
#include "PlacementComponent.h"
#include "InventoryItem.h"
#include "UiComponent.h"
#include "UnIntersectableWindowComponent.h"
#include "component_utils.h"
#include "GlobalTransform.h"
#include "GlobalRect.h"
#include "IntersectingTag.h"

class BubbleIntersectSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* bubbleCache;
	components::CompCache* intersectingBubbleCache;
	components::CompCache* unIntersectableBubbleCache;

	void init(middle::GameState* gameState) {
		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::MouseIntersectable>();
		bubbleCache->addType<components::Rectangle>();
		bubbleCache->addType<components::GlobalRect>();
		bubbleCache->addType<components::GlobalTransform>();
		bubbleCache->addType<components::UnIntersectableWindowComponent>(components::NOTINTERESTED);

		intersectingBubbleCache = middle::newCompCache(gameState, systemName);
		intersectingBubbleCache->addType<components::BubbleComponent>();
		intersectingBubbleCache->addType<components::MouseIntersectable>();
		intersectingBubbleCache->addType<components::Rectangle>();
		intersectingBubbleCache->addType<components::GlobalRect>();
		intersectingBubbleCache->addType<components::GlobalTransform>();
		intersectingBubbleCache->addType<components::IntersectingTag>();
		intersectingBubbleCache->addType<components::UnIntersectableWindowComponent>(components::NOTINTERESTED);

		unIntersectableBubbleCache = middle::newCompCache(gameState, systemName);
		unIntersectableBubbleCache->addType<components::BubbleComponent>();
		unIntersectableBubbleCache->addType<components::UnIntersectableWindowComponent>();
	}

	bool isPlacedRecursive(middle::GameState* gameState, middle::Id& id) {
		auto& shape = middle::getShape(gameState, id.index);
		auto placement = middle::getComponent<components::PlacementComponent>(shape);
		if (placement) {
			return true;
		}
		middle::Id parentId = middle::getParent(gameState, shape.id);
		if (parentId.index != middle::UNASSIGNED) {
			return isPlacedRecursive(gameState, parentId);
		}
		return false;
	}

	void childrenIntersecting(middle::GameState* gameState, middle::Shape& shape) {
		auto intersectingTag = middle::getComponent<components::IntersectingTag>(shape);
		// check that not intersecting children as well
		if (intersectingTag) {
			intersectingTag->intersectingTop = true;
			std::vector<middle::Id>children;
			middle::getAllChildren(gameState, shape.id, children);
			for (middle::Id childId : children) {
				auto& child = middle::getShape(gameState, childId.index);
				auto childIntersectingTag = middle::getComponent<components::IntersectingTag>(child);
				if (childIntersectingTag) {
					intersectingTag->intersectingTop = false;
					break;
				}
			}
		}
	}


	void update(middle::GameState* gameState) override {

		auto unintersectableIt = unIntersectableBubbleCache->begin<components::UnIntersectableWindowComponent>();
		for (middle::Id& id : unIntersectableBubbleCache->relevantIdVector) {
			auto unIntersectable = *unintersectableIt;
			unIntersectable->timeLeft -= gameState->frameTime;
			if (unIntersectable->timeLeft <= 0) {
				middle::queueComponentDeletion<components::UnIntersectableWindowComponent>(gameState, id);
			}
		}

		// delete tag if not itersecting anymore
		auto intersectingTransformIt = intersectingBubbleCache->begin<components::GlobalTransform>();
		auto intersectingGlobalRectIt = intersectingBubbleCache->begin<components::GlobalRect>();
		for (middle::Id id : intersectingBubbleCache->relevantIdVector) {
			auto transform = *intersectingTransformIt;
			auto globalR = *intersectingGlobalRectIt;
			Vector3 pos = transform->pos;
			Vector3 mousePos = middle::RayCastLinePlane(pos, { 0,1,0 }, gameState->activeCamera.position, gameState->input.mouseDir);
			bool intersecting = mousePos.x > pos.x - globalR->width * 0.5f
				&& mousePos.x < pos.x + globalR->width * 0.5f
				&& mousePos.z > pos.z - globalR->height * 0.5f 
				&& mousePos.z < pos.z + globalR->height * 0.5f;
			if (!intersecting) {
				middle::queueComponentDeletion<components::IntersectingTag>(gameState, id);
			}
		}

		// add tag to newly intersecting shapes, and update intersecting top value 
		auto bubbleIntersectableIt = bubbleCache->begin<components::MouseIntersectable>();
		auto bubbleTransformIt = bubbleCache->begin<components::GlobalTransform>();
		auto bubbleGlobalRadiusIt = bubbleCache->begin<components::GlobalRect>();
		for (middle::Id id : bubbleCache->relevantIdVector) {
			auto intersectable = *bubbleIntersectableIt;
			auto transform = *bubbleTransformIt;
			auto globalR = *bubbleGlobalRadiusIt;

			Vector3 pos = transform->pos;
			auto tag = middle::getComp<components::IntersectingTag>(gameState, id);
			bool intersecting = tag != nullptr;

			if (!intersecting) {
				Vector3 mousePos = middle::RayCastLinePlane(pos, { 0,1,0 }, gameState->activeCamera.position, gameState->input.mouseDir);
				intersecting = mousePos.x > pos.x - globalR->width * 0.5f
					&& mousePos.x < pos.x + globalR->width * 0.5f
					&& mousePos.z > pos.z - globalR->height * 0.5f
					&& mousePos.z < pos.z + globalR->height * 0.5f;
			}

			if (intersecting) {
				// check that children are not already intersecting or grabbing 
				bool alreadyIntersecting = false;
				std::vector<middle::Id>children;
				middle::getAllChildren(gameState, id, children);
				for (auto& childId : children) {
					auto intersecting = middle::getComp<components::IntersectingTag>(gameState, childId);
					if (intersecting) {
						alreadyIntersecting = true;
					}
				}
				if (!tag) {
					tag = middle::attachComponent<components::IntersectingTag>(gameState, id);
				}
				if (tag) {
					tag->intersectingTop = !alreadyIntersecting;
				}
			}
		}


	}
};

static middle::SystemRegistrar<BubbleIntersectSystem> reg("BubbleIntersectSystem");
