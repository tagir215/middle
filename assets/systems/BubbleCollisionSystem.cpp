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
		std::vector<middle::Id> bubbleList;
		middle::loopInstances(gameState, [gameState, &bubbleList](int i, middle::Shape& shape) {

			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			auto grabbable = middle::getComponent<components::MouseGrabbable>(shape);
			if (bubble && grabbable && !grabbable->grabbing) {
				bubbleList.push_back(shape.id);
			}
			});

		// create pairs of bubbles children
		std::vector<CollisionPair>pairs;
		for (middle::Id id : bubbleList) {
			auto bubbleShape = middle::getShape(gameState, id.index);
			auto bubbleLoop = middle::getComponent<components::LoopSociety>(bubbleShape);

			int size = bubbleLoop->loopMemberIds.size();
			for (int i = 0; i < size; ++i) {
				for (int j = i + 1; j < size; ++j) {
					auto& idA = bubbleLoop->loopMemberIds[i];
					auto& idB = bubbleLoop->loopMemberIds[j];
					pairs.push_back({ idA, idB });
					continue;
				}
			}
		}

		// detect collisions
		std::vector<CollisionManifold>manifolds;
		float inversePi2 = 1.0f / (2 * PI);
		for (const CollisionPair& pair : pairs) {
			auto shapeA = middle::getShape(gameState, pair.idA.index);
			auto shapeB = middle::getShape(gameState, pair.idB.index);
			auto bubbleA = middle::getComponent<components::BubbleComponent>(shapeA);
			auto bubbleB = middle::getComponent<components::BubbleComponent>(shapeB);
			auto loopA = middle::getComponent<components::LoopSociety>(shapeA);
			auto loopB = middle::getComponent<components::LoopSociety>(shapeB);
			auto unitA = middle::getComponent<components::BubbleUnit>(shapeA);
			auto unitB = middle::getComponent<components::BubbleUnit>(shapeB);
			auto posA = middle::getComponent<components::Position>(shapeA);
			auto posB = middle::getComponent<components::Position>(shapeB);
			Vector3 positionA = { posA->posX, posA->posY, posA->posZ };
			Vector3 positionB = { posB->posX, posB->posY, posB->posZ };

			if (bubbleA && bubbleB) {
				const float bubbleDistanceMargin = 5;
				float radiusA = bubbleA->length + bubbleDistanceMargin;
				float radiusB = bubbleB->length + bubbleDistanceMargin;
				float distSqr = Vector3DistanceSqr(positionA, positionB);
				float superRadius = radiusA + radiusB;
				if (distSqr < superRadius * superRadius) {
					CollisionManifold manifold;
					manifold.idA = shapeA.id;
					manifold.idB = shapeB.id;
					manifold.normal = Vector3Normalize(positionB - positionA);
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

			moveShape(gameState, manifold.idA.index, velA);
			moveShape(gameState, manifold.idB.index, velB);
		}
	}
};

static middle::SystemRegistrar<BubbleCollisionSystem> reg("BubbleCollisionSystem");
