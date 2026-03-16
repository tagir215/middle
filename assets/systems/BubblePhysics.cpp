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
#include "BubbleMultiplyComponent.h"
#include "FractionalComponent.h"
#include "Rectangle.h"
#include "TopDogBubbleTag.h"

class BubblePhysics : public middle::MiddleGameplaySystem {
public:
	components::CompCache* unitCache;
	components::CompCache* bubbleCache;
	components::CompCache* mulCache;
	components::CompCache* fractionCache;
	components::CompCache* rectCache;
	components::CompCache* topDogBubbleCache;

	void init(middle::GameState* gameState) override {
		unitCache = middle::newCompCache(gameState);
		unitCache->addType<components::Position>();
		unitCache->addType<components::PhysicsData>();
		unitCache->addType<components::BubbleUnit>();

		bubbleCache = middle::newCompCache(gameState);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::Position>();
		bubbleCache->addType<components::PhysicsData>();
		bubbleCache->addType<components::Circle>();
		bubbleCache->addType<components::LoopSociety>();

		mulCache = middle::newCompCache(gameState);
		mulCache->addType<components::BubbleMultiplyComponent>();
		mulCache->addType<components::LoopSociety>();

		fractionCache = middle::newCompCache(gameState);
		fractionCache->addType<components::FractionalComponent>();
		fractionCache->addType<components::LoopSociety>();

		rectCache = middle::newCompCache(gameState);
		rectCache->addType<components::Rectangle>();
		rectCache->addType<components::Position>();
		rectCache->addType<components::PhysicsData>();

		topDogBubbleCache = middle::newCompCache(gameState);
		topDogBubbleCache->addType<components::TopDogBubbleTag>();
		topDogBubbleCache->addType<components::Position>();
		topDogBubbleCache->addType<components::PhysicsData>();
		topDogBubbleCache->addType<components::Circle>();
	}

	struct Body {
		middle::Id id;
		components::Position* pos;
		components::PhysicsData* physicsData;
		float radius;
		float width;
		float height;
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

	struct MoleculeConstraint {
		std::vector<Body>bodies;
		std::vector<float>targetDistances;
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
					collision.bodyA = bubbleBody;
					collision.bodyB = body;
					collision.penetration = dist - bubbleBody.radius + body.radius;
					results.push_back(collision);
				}
			}
		}
	}

	void findCollisionsWithGreatCenterLine(std::vector<Body>& topDogs, Body& greatCenterLine, std::vector<Collision>& results) {
		// great line assumed to be vertical
		Vector3 axis = { 1,0,0 };
		Vector3 greatCenterLinePos = { greatCenterLine.pos->posX, greatCenterLine.pos->posY, greatCenterLine.pos->posZ };
		greatCenterLine.physicsData->infiniteMass = true;
		greatCenterLine.physicsData->invMass = 0;
		for (Body& body : topDogs) {
			Vector3 bodyPos = { body.pos->posX, body.pos->posY, body.pos->posZ };
			float dir = Vector3DotProduct(Vector3Subtract(bodyPos, greatCenterLinePos), axis);
			float dist = std::abs(dir);
			float outlineRadius = greatCenterLine.width * 0.5f + body.radius;
			if (dist < outlineRadius) {
				Collision collision;
				collision.axis = Vector3Normalize(Vector3Scale(axis, dir));
				collision.bodyA = greatCenterLine;
				collision.bodyB = body;
				collision.penetration = outlineRadius - dist;
				results.push_back(collision);
			}
		}
	}

	void solveVelocity(Collision& collision, float frameTime, float inverseTime) {
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
		Vector3 impulse = Vector3Scale(axis, impulseMag);
		const float biasFactor = 0.2f;
		Vector3 bias = Vector3Scale(axis, collision.penetration * inverseTime * biasFactor * eMass);
		impulse += bias;

		Vector3 acc = Vector3Scale(impulse, frameTime);
		bodyA.physicsData->velX -= acc.x * bodyA.physicsData->invMass;
		bodyA.physicsData->velZ -= acc.z * bodyA.physicsData->invMass;
		bodyB.physicsData->velX += acc.x * bodyB.physicsData->invMass;
		bodyB.physicsData->velZ += acc.z * bodyB.physicsData->invMass;
	}

	void solveMoleculeConstraint(MoleculeConstraint& constraint, float frameTime, float inverseTime) {
		for (int i = 1; i < constraint.bodies.size(); ++i) {
			Body& bodyA = constraint.bodies[i - 1];
			Body& bodyB = constraint.bodies[i];
			Vector3 posA = { bodyA.pos->posX, bodyA.pos->posY, bodyA.pos->posZ };
			Vector3 posB = { bodyB.pos->posX, bodyB.pos->posY, bodyB.pos->posZ };
			Vector3 velA = { bodyA.physicsData->velX, bodyA.physicsData->velY, bodyA.physicsData->velZ };
			Vector3 velB = { bodyB.physicsData->velX, bodyB.physicsData->velY, bodyB.physicsData->velZ };
			Vector3 axis = Vector3Normalize(Vector3Subtract(posB, posA));
			float dist = Vector3Distance(posA, posB);
			float relVel = Vector3DotProduct(Vector3Subtract(velB, velA), axis);
			float error = constraint.targetDistances[i - 1] - dist;
			float eMass = 1.0f / (bodyA.physicsData->invMass + bodyB.physicsData->invMass);
			float targetRelVel = 0;
			float impulseMag = (targetRelVel - relVel) * eMass;

			Vector3 impulse = Vector3Scale(axis, impulseMag);
			const float stiffness = 0.8f;
			Vector3 bias = Vector3Scale(axis, error * inverseTime * stiffness * eMass);
			impulse += bias;

			Vector3 acc = Vector3Scale(impulse, frameTime);
			bodyA.physicsData->velX -= acc.x * bodyA.physicsData->invMass;
			bodyA.physicsData->velZ -= acc.z * bodyA.physicsData->invMass;
			bodyB.physicsData->velX += acc.x * bodyB.physicsData->invMass;
			bodyB.physicsData->velZ += acc.z * bodyB.physicsData->invMass;
		}
	}

	void integrate(float frameTime, components::CompCache* cache) {
		// integrating
		const float damping = 0.2f;
		auto posIt = cache->begin<components::Position>();
		auto physicsIt = cache->begin<components::PhysicsData>();
		for (int i = 0; i < cache->getSize(); ++i) {
			auto pos = *posIt;
			auto physics = *physicsIt;
			pos->posX += physics->velX * frameTime;
			pos->posY += physics->velY * frameTime;
			pos->posZ += physics->velZ * frameTime;
			physics->velX -= physics->velX * damping;
			physics->velY -= physics->velY * damping;
			physics->velZ -= physics->velZ * damping;
		}

	}

	void collectMoleculeConstraints(middle::GameState* gameState, components::CompCache* cache, std::vector<MoleculeConstraint>& constraints) {
		auto mulLoopIt = cache->begin<components::LoopSociety>();
		for (int i = 0; i < cache->getSize(); ++i) {
			auto loop = *mulLoopIt;
			MoleculeConstraint moleculeConstraint;
			Body prevBody;
			for (int j = 0; j < loop->loopMemberIds.size(); ++j) {
				auto& childShape = middle::getShape(gameState, loop->loopMemberIds[j].index);
				auto position = middle::getComponent<components::Position>(childShape);
				auto physics = middle::getComponent<components::PhysicsData>(childShape);
				auto childCircle = middle::getComponent<components::Circle>(childShape);
				auto mul = middle::getComponent<components::BubbleMultiplyComponent>(childShape);
				auto fraction = middle::getComponent<components::FractionalComponent>(childShape);
				assert(physics);

				Body body;
				body.id = childShape.id;
				body.physicsData = physics;
				body.pos = position;
				body.radius = 1;
				if (childCircle) {
					body.radius = childCircle->radius;
				}
				moleculeConstraint.bodies.push_back(body);

				const float targetSeparation = 10;
				if (j > 0 && childCircle) {
					moleculeConstraint.targetDistances.push_back(prevBody.radius + targetSeparation + childCircle->radius);
				}
				if(j > 0 && !childCircle) {
					moleculeConstraint.targetDistances.push_back(targetSeparation);
				}
				prevBody = body;
			}
			constraints.push_back(moleculeConstraint);
		}
	}

	const float attractionForce = 20;
	const float fieldMargin = 5.0f;
	bool debugField = false;
	bool inverses = true;

	void update(middle::GameState* gameState) override {
		if (debugField) {
			for (int i = 0; i < unitCache->getSize(); ++i) {
				auto& shape = middle::getShape(gameState, unitCache->relevantIdVector[i].index);
				if (!middle::getComponent<components::BubbleUnit>(shape)) {
					continue;
				}
				auto circle = middle::attachComponent<components::Circle>(gameState, shape.id);
				circle->radius = fieldMargin;
			}
			debugField = false;
		}

		if (inverses) {
			auto inverseIt = unitCache->begin<components::PhysicsData>();
			for (int i = 0; i < unitCache->getSize(); ++i) {
				auto physics = *inverseIt;
				physics->invMass = 1.0f / physics->mass;
			}
		}


		// Collect bubbles

		std::vector<Bubble>bubbles;
		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		auto bubblePosIt = bubbleCache->begin<components::Position>();
		auto circleIt = bubbleCache->begin<components::Circle>();
		auto physicsIt = bubbleCache->begin<components::PhysicsData>();
		auto bubbleLoopIt = bubbleCache->begin<components::LoopSociety>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto bubble = *bubbleIt;
			auto bubblePos = *bubblePosIt;
			auto circle = *circleIt;
			auto bubblePhysics = *physicsIt;
			auto loop = *bubbleLoopIt;
			auto& bubbleShape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);

			std::vector<middle::Id> interactingChildren;
			for (middle::Id& id : loop->loopMemberIds) {
				auto& childShape = middle::getShape(gameState, id.index);
				if (middle::getComponent<components::BubbleMultiplyComponent>(childShape) 
					|| middle::getComponent<components::FractionalComponent>(childShape)) {
					std::vector<middle::Id>mulChildren;
					middle::getChildrenWithComp(gameState, id, mulChildren, middle::getTypeId<components::PhysicsData>());
					interactingChildren.insert(interactingChildren.end(), mulChildren.begin(), mulChildren.end());
				}
				else if (middle::getComponent<components::PhysicsData>(childShape)) {
					interactingChildren.push_back(id);
				}
			}

			std::vector<Body>bodies;
			for (middle::Id& childId : interactingChildren) {
				auto& childShape = middle::getShape(gameState, childId.index);
				auto position = middle::getComponent<components::Position>(childShape);
				auto physics = middle::getComponent<components::PhysicsData>(childShape);
				auto childCircle = middle::getComponent<components::Circle>(childShape);
				auto childUnit = middle::getComponent<components::BubbleUnit>(childShape);
				assert(physics);
				// units use a field radius instead
				float radius = childCircle ? childCircle->radius + fieldMargin : fieldMargin;
				Body body;
				body.id = childId;
				body.pos = position;
				body.physicsData = physics;
				body.radius = radius;
				bodies.push_back(body);
			}
			Body body;
			body.id = bubbleShape.id;
			body.pos = bubblePos;
			body.physicsData = bubblePhysics;
			body.radius = circle->radius;
			bubbles.push_back({
				body,
				bodies,
				});
		}

		// Collect great rectangles
		auto rectIt = rectCache->begin<components::Rectangle>();
		auto rectPosIt = rectCache->begin<components::Position>();
		auto rectPhysicsIt = rectCache->begin<components::PhysicsData>();
		std::vector<Body>greatCenterLines;
		for (int i = 0; i < rectCache->getSize(); ++i) {
			auto rect = *rectIt;
			auto rectPos = *rectPosIt;
			auto rectPhysics = *rectPhysicsIt;
			Body body;
			body.id = rectCache->relevantIdVector[i];
			body.pos = rectPos;
			body.physicsData = rectPhysics;
			body.width = rect->width;
			body.height = rect->height;
			greatCenterLines.push_back(body);
		}

		// Collect top bubbles
		auto topBubbleIt = topDogBubbleCache->begin<components::TopDogBubbleTag>();
		auto topBubblePosIt = topDogBubbleCache->begin<components::Position>();
		auto topBubblePhysicsIt = topDogBubbleCache->begin<components::PhysicsData>();
		auto topBubbleCircleIt = topDogBubbleCache->begin<components::Circle>();
		std::vector<Body>topDogBubbles;
		for (int i = 0; i < topDogBubbleCache->getSize(); ++i) {
			auto topBubble = *topBubbleIt;
			auto topBubblePos = *topBubblePosIt;
			auto topBubblePhysics = *topBubblePhysicsIt;
			auto circle = *topBubbleCircleIt;
			Body body;
			body.id = topDogBubbleCache->relevantIdVector[i];
			body.pos = topBubblePos;
			body.physicsData = topBubblePhysics;
			body.radius = circle->radius + fieldMargin;
			topDogBubbles.push_back(body);
		}

		// COLLECT MOLECULE CONSTRAINTS
		std::vector<MoleculeConstraint>moleculeConstraints;
		collectMoleculeConstraints(gameState, mulCache, moleculeConstraints);
		collectMoleculeConstraints(gameState, fractionCache, moleculeConstraints);

		// CREATE COLLISION PAIRS

		// pairs within bubbles
		std::vector<std::vector<BodyPair>>pairVectors;
		for (auto& bubble : bubbles) {
			std::vector<BodyPair>pairs;
			for (int x = 0; x < bubble.bodies.size(); ++x) {
				for (int y = x + 1; y < bubble.bodies.size(); ++y) {
					pairs.push_back({ bubble.bodies[x], bubble.bodies[y] });
				}
			}
			pairVectors.push_back(pairs);
		}


		// attraction forces
		const float attractionForce = 200;
		for (Bubble& bubble : bubbles) {
			Body& bubbleBody = bubble.bubbleBody;
			for (Body& unit : bubble.bodies) {
				float dirX = bubbleBody.pos->posX - unit.pos->posX;
				float dirY = bubbleBody.pos->posY - unit.pos->posY;
				float dirZ = bubbleBody.pos->posZ - unit.pos->posZ;
				Vector3 attractionDir = Vector3Normalize({ dirX, dirY, dirZ });
				Vector3 acc = Vector3Scale(attractionDir, attractionForce);
				unit.physicsData->velX += acc.x * gameState->frameTime;
				unit.physicsData->velY += acc.y * gameState->frameTime;
				unit.physicsData->velZ += acc.z * gameState->frameTime;
			}
		}

		// attraction Forces
		//for (std::vector<BodyPair>& pairVector : pairVectors) {
		//	for (BodyPair& pair : pairVector) {
		//		auto& unitA = pair.bodyA;
		//		auto& unitB = pair.bodyB;
		//		Vector3 posA = { unitA.pos->posX, unitA.pos->posY, unitA.pos->posZ };
		//		Vector3 posB = { unitB.pos->posX, unitB.pos->posY, unitB.pos->posZ };
		//		Vector3 axis = Vector3Normalize(Vector3Subtract(posB, posA));
		//		Vector3 acc = Vector3Scale(axis, -attractionForce * gameState->frameTime);
		//		unitA.physicsData->velX -= acc.x;
		//		unitA.physicsData->velZ -= acc.z;
		//		unitB.physicsData->velX += acc.x;
		//		unitB.physicsData->velZ += acc.z;
		//	}
		//}

		std::vector<Collision>collisions;
		findSiblingCollisions(pairVectors, collisions);
		findCollisionsWithOutline(bubbles, collisions);

		if (greatCenterLines.size() == 1) {
			findCollisionsWithGreatCenterLine(topDogBubbles, greatCenterLines[0], collisions);
		}

		const float inverseTime = 1.0f / gameState->frameTime;
		// forces between units
		for (int iteration = 0; iteration < 8; ++iteration) {
			for (Collision& collision : collisions) {
				solveVelocity(collision, gameState->frameTime, inverseTime);
			}
			for (MoleculeConstraint& constraint : moleculeConstraints) {
				solveMoleculeConstraint(constraint, gameState->frameTime, inverseTime);
			}
		}

		integrate(gameState->frameTime, bubbleCache);
		integrate(gameState->frameTime, unitCache);

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
