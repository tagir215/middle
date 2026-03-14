#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "PhysicsData.h"
#include "BubbleUnit.h"
#include "BubbleComponent.h"
#include "Position.h"
#include "component_utils.h"
#include "Circle.h"
#include "LoopSociety.h"

class BubblePhysics : public middle::MiddleGameplaySystem {
public:
	components::CompCache* bodyCache;
	components::CompCache* bubbleCache;

	void init(middle::GameState* gameState) override {
		bodyCache = middle::newCompCache(gameState);
		bodyCache->addType<components::Position>();
		bodyCache->addType<components::PhysicsData>();
		bubbleCache = middle::newCompCache(gameState);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::Position>();
		bubbleCache->addType<components::PhysicsData>();
		bubbleCache->addType<components::Circle>();
	}

	struct Body {
		middle::Id id;
		components::Position* pos;
		components::PhysicsData* physicsData;
		float radius;
	};

	struct BodyPair {
		Body bodyA;
		Body bodyB;
	};

	struct Bubble {
		Body bubbleBody;
		std::vector<Body>bodies;
	};

	struct Collision {
		Body bodyA;
		Body bodyB;
		Vector3 axis;
		float penetration;
	};

	void findSiblingCollisions(std::vector<std::vector<BodyPair>>& pairVectors, std::vector<Collision>& results) {
		for (std::vector<BodyPair>& pairVector : pairVectors) {
			for (BodyPair& pair : pairVector) {
				Vector3 posA = { pair.bodyA.pos->posX, pair.bodyA.pos->posY, pair.bodyA.pos->posZ };
				Vector3 posB = { pair.bodyB.pos->posX, pair.bodyB.pos->posY, pair.bodyB.pos->posZ };
				float dist = Vector3Distance(posA, posB);
				if (dist < pair.bodyA.radius + pair.bodyB.radius) {
					Collision collision;
					collision.axis = Vector3Normalize(Vector3Subtract(posB, posA));
					collision.bodyA = pair.bodyA;
					collision.bodyB = pair.bodyB;
					collision.penetration = pair.bodyA.radius + pair.bodyB.radius - dist;
					results.push_back(collision);
				}
			}
		}
	}

	void findCollisionsWithOutline(std::vector<Bubble>& bubbles, std::vector<Collision>& results) {
		for (Bubble& bubble : bubbles) {
			Body& bubbleBody = bubble.bubbleBody;
			Vector3 bubblePos = { bubbleBody.pos->posX, bubbleBody.pos->posY, bubbleBody.pos->posZ };
			for (Body& body : bubble.bodies) {
				Vector3 bodyPos = { body.pos->posX, body.pos->posY, body.pos->posZ };
				float dist = Vector3Distance(bubblePos, bodyPos);
				if (dist > bubbleBody.radius - body.radius) {
					Collision collision;
					collision.axis = Vector3Normalize(Vector3Subtract(bubblePos, bodyPos));
					collision.bodyA = body;
					collision.bodyB = bubbleBody;
					collision.penetration = bubbleBody.radius - dist - body.radius;
					results.push_back(collision);
				}
			}
		}
	}

	void solveVelocity(middle::GameState* gameState, Collision& collision, float inverseTime) {
		Body& bodyA = collision.bodyA;
		Body& bodyB = collision.bodyB;
		Vector3 axis = collision.axis;
		Vector3 posA = { bodyA.pos->posX, bodyA.pos->posY, bodyA.pos->posZ };
		Vector3 posB = { bodyB.pos->posX, bodyB.pos->posY, bodyB.pos->posZ };
		Vector3 velA = { bodyA.physicsData->velX, bodyA.physicsData->velY, bodyA.physicsData->velZ };
		Vector3 velB = { bodyB.physicsData->velX, bodyB.physicsData->velY, bodyB.physicsData->velZ };

		float relVel = Vector3DotProduct(Vector3Subtract(velB, velA), axis);
		float eMass = 1.0f / (bodyA.physicsData->invMass + bodyB.physicsData->invMass);
		float targetRelVel = 0;
		float impulseMag = (targetRelVel - relVel) * eMass;
		Vector3 impulse = Vector3Scale(axis, impulseMag * inverseTime);
		const float biasFactor = 0.2f;
		Vector3 bias = Vector3Scale(axis, collision.penetration * inverseTime * inverseTime * biasFactor);
		impulse += bias;

		Vector3 acc = Vector3Scale(impulse, gameState->frameTime);
		bodyA.physicsData->velX -= acc.x * bodyA.physicsData->invMass;
		bodyA.physicsData->velZ -= acc.z * bodyA.physicsData->invMass;
		bodyB.physicsData->velX += acc.x * bodyB.physicsData->invMass;
		bodyB.physicsData->velZ += acc.z * bodyB.physicsData->invMass;
	}

	const float attractionForce = 20;
	const float fieldRadius = 10.0f;
	bool debugField = true;

	void update(middle::GameState* gameState) override {
		if (debugField) {
			for (int i = 0; i < bodyCache->getSize(); ++i) {
				auto& shape = middle::getShape(gameState, bodyCache->relevantIdVector[i].index);
				if (!middle::getComponent<components::BubbleUnit>(shape)) {
					continue;
				}
				auto circle = middle::attachComponent<components::Circle>(gameState, shape.id);
				circle->radius = fieldRadius;
			}
			debugField = false;
		}

		std::vector<Bubble>bubbles;
		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		auto bubblePosIt = bubbleCache->begin<components::Position>();
		auto circleIt = bubbleCache->begin<components::Circle>();
		auto physicsIt = bubbleCache->begin<components::PhysicsData>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto bubble = *bubbleIt;
			auto bubblePos = *bubblePosIt;
			auto circle = *circleIt;
			auto bubblePhysics = *physicsIt;
			auto& bubbleShape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);
			std::vector<middle::Id>children;
			middle::getChildrenWithComp(gameState, bubbleShape.id, children, middle::getTypeId<components::PhysicsData>());
			std::vector<Body>bodies;
			for (middle::Id& childId : children) {
				auto& childShape = middle::getShape(gameState, childId.index);
				auto position = middle::getComponent<components::Position>(childShape);
				auto physics = middle::getComponent<components::PhysicsData>(childShape);
				auto childCircle = middle::getComponent<components::Circle>(childShape);
				float radius = childCircle != nullptr ? childCircle->radius : fieldRadius;
				bodies.push_back({
					childId,
					position,
					physics,
					radius,
					});

			}
			bubbles.push_back({
				{
					bubbleShape.id,
					bubblePos,
					bubblePhysics,
					circle->radius
				},
				bodies,
				});
		}

		std::vector<std::vector<BodyPair>>pairVectors;
		for (auto& bubble : bubbles) {
			std::vector<BodyPair>pairs;
			for (int i = 0; i < bubble.bodies.size(); ++i) {
				for (int j = i + 1; j < bubble.bodies.size(); ++j) {
					pairs.push_back({ bubble.bodies[i], bubble.bodies[j] });
				}
			}
			pairVectors.push_back(pairs);
		}

		// attraction forces
		//const float attractionForce = 200;
		//for (Bubble& bubble : bubbles) {
		//	Body& bubbleBody = bubble.bubbleBody;
		//	for (Body& unit : bubble.bodies) {
		//		float dirX = bubbleBody.pos->posX - unit.pos->posX;
		//		float dirY = bubbleBody.pos->posY - unit.pos->posY;
		//		float dirZ = bubbleBody.pos->posZ - unit.pos->posZ;
		//		Vector3 attractionDir = Vector3Normalize({ dirX, dirY, dirZ });
		//		Vector3 acc = Vector3Scale(attractionDir, attractionForce);
		//		unit.physicsData->velX += acc.x * gameState->frameTime;
		//		unit.physicsData->velY += acc.y * gameState->frameTime;
		//		unit.physicsData->velZ += acc.z * gameState->frameTime;
		//	}
		//}

		// attraction Forces
		for (std::vector<BodyPair>& pairVector : pairVectors) {
			for (BodyPair& pair : pairVector) {
				auto& unitA = pair.bodyA;
				auto& unitB = pair.bodyB;
				Vector3 posA = { unitA.pos->posX, unitA.pos->posY, unitA.pos->posZ };
				Vector3 posB = { unitB.pos->posX, unitB.pos->posY, unitB.pos->posZ };
				Vector3 axis = Vector3Normalize(Vector3Subtract(posB, posA));
				Vector3 acc = Vector3Scale(axis, -attractionForce * gameState->frameTime);
				unitA.physicsData->velX -= acc.x;
				unitA.physicsData->velZ -= acc.z;
				unitB.physicsData->velX += acc.x;
				unitB.physicsData->velZ += acc.z;
			}
		}

		std::vector<Collision>collisions;
		findSiblingCollisions(pairVectors, collisions);
		findCollisionsWithOutline(bubbles, collisions);

		const float inverseTime = 1.0f / gameState->frameTime;
		// forces between units
		for (int iteration = 0; iteration < 8; ++iteration) {
			for (Collision& collision : collisions) {
				solveVelocity(gameState, collision, inverseTime);
			}
		}

		// integrating
		auto unitPosIt = bodyCache->begin<components::Position>();
		auto unitPhysicsIt = bodyCache->begin<components::PhysicsData>();
		for (int i = 0; i < bodyCache->getSize(); ++i) {
			auto pos = *unitPosIt;
			auto physics = *unitPhysicsIt;
			pos->posX += physics->velX * gameState->frameTime;
			pos->posY += physics->velY * gameState->frameTime;
			pos->posZ += physics->velZ * gameState->frameTime;
		}

		//const float stiffness = 0.2f;
		//for (int iteration = 0; iteration < 8; ++iteration) {
		//	for (std::vector<BodyPair>& pairVector : pairVectors) {
		//		for (BodyPair& pair : pairVector) {
		//			auto& unitA = pair.bodyA;
		//			auto& unitB = pair.bodyB;
		//			Vector3 posA = { unitA.pos->posX, unitA.pos->posY, unitA.pos->posZ };
		//			Vector3 posB = { unitB.pos->posX, unitB.pos->posY, unitB.pos->posZ };
		//			Vector3 axis = Vector3Normalize(Vector3Subtract(posB, posA));
		//			float dist = Vector3Distance(posA, posB);
		//			float penetration = (unitA.radius + unitB.radius - dist);
		//			if (penetration > 0) {
		//				Vector3 correction = Vector3Scale(axis, penetration * 0.5f * stiffness);
		//				unitA.pos->posX -= correction.x;
		//				unitA.pos->posZ -= correction.z;
		//				unitB.pos->posX += correction.x;
		//				unitB.pos->posZ += correction.z;
		//			}

		//		}
		//	}
		//}

	}
};

static middle::SystemRegistrar<BubblePhysics> reg("BubblePhysics");
