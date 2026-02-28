#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "middle_component_table.h"
#include "Position.h"
#include "LoopSociety.h"
#include "BubbleComponent.h"
#include "BubbleUnit.h"
#include "PhysicsData.h"
#include "MouseGrabbable.h"
#include "BubbleMultiplyComponent.h"

class BubbleCollisionSystem : public middle::MiddleGameplaySystem {
	struct CollisionPair {
		middle::Id idA;
		middle::Id idB;
	};

	struct CollisionManifold {
		middle::Id idA;
		middle::Id idB;
		float depth;
		float maxDepth;
		Vector3 normal;
	};

	void update(middle::GameState* gameState) override {

		// get bubbles
		std::vector<middle::Id> containerList;
		middle::loopInstances(gameState, [gameState, &containerList](int i, middle::Shape& shape) {

			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(shape);
			if (bubble || mulComp) {
				containerList.push_back(shape.id);
			}
			return true;
			});

		// create pairs of bubbles children
		std::vector<CollisionPair>pairs;
		for (middle::Id id : containerList) {
			auto bubbleShape = middle::getShape(gameState, id.index);
			auto bubbleLoop = middle::getComponent<components::LoopSociety>(bubbleShape);
			std::vector<middle::Id>candidates;
			middle::getChildrenWithComp(gameState, id, candidates, middle::getTypeId<components::BubbleComponent>());

			int size = candidates.size();
			for (int i = 0; i < size; ++i) {
				for (int j = i + 1; j < size; ++j) {
					auto& idA = candidates[i];
					auto& idB = candidates[j];
					auto& childA = middle::getShape(gameState, idA.index);
					auto& childB = middle::getShape(gameState, idB.index);
					auto grabbableA = middle::getComponent<components::MouseGrabbable>(childA);
					auto grabbableB = middle::getComponent<components::MouseGrabbable>(childB);
					if (grabbableA && grabbableA->grabbing)
						continue;
					if (grabbableB && grabbableB->grabbing)
						continue;
					pairs.push_back({ idA, idB });
					continue;
				}
			}
		}

		// detect collisions
		std::vector<CollisionManifold>manifolds;
		for (const CollisionPair& pair : pairs) {
			auto shapeA = middle::getShape(gameState, pair.idA.index);
			auto shapeB = middle::getShape(gameState, pair.idB.index);
			auto bubbleA = middle::getComponent<components::BubbleComponent>(shapeA);
			auto bubbleB = middle::getComponent<components::BubbleComponent>(shapeB);
			auto posA = middle::getComponent<components::Position>(shapeA);
			auto posB = middle::getComponent<components::Position>(shapeB);
			Vector3 positionA = { posA->posX, posA->posY, posA->posZ };
			Vector3 positionB = { posB->posX, posB->posY, posB->posZ };

			if (bubbleA && bubbleB) {
				const float bubbleDistanceMargin = 5;
				float radiusA = bubbleA->length + bubbleDistanceMargin;
				float radiusB = bubbleB->length + bubbleDistanceMargin;
				float distSqr = Vector3DistanceSqr(positionA, positionB);
				float totalRadius = radiusA + radiusB;
				Vector3 normal = Vector3Normalize(positionB - positionA);
				if (Vector3LengthSqr(normal) == 0) {
					normal = { 1,0,0 };
				}
				if (distSqr < totalRadius* totalRadius) {
					CollisionManifold manifold;
					manifold.idA = shapeA.id;
					manifold.idB = shapeB.id;
					manifold.normal = normal;
					manifold.maxDepth = radiusA > radiusB ? radiusA : radiusB;
					float dist = std::sqrtf(distSqr);
					manifold.depth = radiusA + radiusB - dist;
					manifolds.push_back(manifold);
				}
			}
		}


		for (const CollisionManifold& manifold : manifolds) {
			middle::Shape& shapeA = middle::getShape(gameState, manifold.idA.index);
			middle::Shape& shapeB = middle::getShape(gameState, manifold.idB.index);
			const float speed = 1.2f;
			Vector3 velA = Vector3Scale(manifold.normal, -speed);
			Vector3 velB = Vector3Scale(manifold.normal, speed);

			auto bubbleA = middle::getComponent<components::BubbleComponent>(shapeA);
			auto bubbleB = middle::getComponent<components::BubbleComponent>(shapeB);


			if (!bubbleA->infiniteMass)
				moveShape(gameState, manifold.idA.index, velA);
			if (!bubbleB->infiniteMass)
				moveShape(gameState, manifold.idB.index, velB);
		}
	}
};

static middle::SystemRegistrar<BubbleCollisionSystem> reg("BubbleCollisionSystem");
