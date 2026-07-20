#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "PhysicsData.h"
#include "BubbleComponent.h"
#include "component_utils.h"
#include "Circle.h"
#include "LoopSociety.h"
#include "BubbleMultiplyComponent.h"
#include "FractionalComponent.h"
#include "Rectangle.h"
#include "TopDogBubbleTag.h"
#include "BubbleEqualsComponent.h"
#include "DeleteComponent.h" 
#include "IdRef.h"
#include "GlobalTransform.h"
#include "LocalPosition.h"

class BubblePhysics : public middle::MiddleGameplaySystem {
public:

	BubblePhysics() {
		systemModeType = middle::SystemModeType::ENGINE;
	}

	components::CompCache* bubbleCache;
	components::CompCache* mulCache;
	components::CompCache* fractionCache;
	components::CompCache* rectCache;
	components::CompCache* topDogBubbleCache;
	components::CompCache* equalsCache;

	void init(middle::GameState* gameState) override {

		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::GlobalTransform>();
		bubbleCache->addType<components::LocalPosition>();
		bubbleCache->addType<components::PhysicsData>();
		bubbleCache->addType<components::Circle>();
		bubbleCache->addType<components::LoopSociety>();
		bubbleCache->addType<components::IdRef>(components::NOTINTERESTED);

		mulCache = middle::newCompCache(gameState, systemName);
		mulCache->addType<components::BubbleMultiplyComponent>();
		mulCache->addType<components::LoopSociety>();

		fractionCache = middle::newCompCache(gameState, systemName);
		fractionCache->addType<components::FractionalComponent>();
		fractionCache->addType<components::LoopSociety>();

		equalsCache = middle::newCompCache(gameState, systemName);
		equalsCache->addType<components::BubbleEqualsComponent>();
		equalsCache->addType<components::LoopSociety>();

		rectCache = middle::newCompCache(gameState, systemName);
		rectCache->addType<components::Rectangle>();
		rectCache->addType<components::GlobalTransform>();
		rectCache->addType<components::LocalPosition>();
		rectCache->addType<components::PhysicsData>();

		topDogBubbleCache = middle::newCompCache(gameState, systemName);
		topDogBubbleCache->addType<components::TopDogBubbleTag>();
		topDogBubbleCache->addType<components::GlobalTransform>();
		topDogBubbleCache->addType<components::LocalPosition>();
		topDogBubbleCache->addType<components::PhysicsData>();
		topDogBubbleCache->addType<components::Circle>();
		topDogBubbleCache->addType<components::LoopSociety>();
		topDogBubbleCache->addType<components::IdRef>(components::NOTINTERESTED);

	}

	struct Body {
		middle::Id id;
		components::GlobalTransform* transform;
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
				Vector3 posA = pair.bodyA.transform->pos;
				Vector3 posB = pair.bodyB.transform->pos;
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
			Vector3 bubblePos = bubbleBody.transform->pos;
			for (Body& body : bubble.bodies) {
				Vector3 bodyPos = body.transform->pos;
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
		Vector3 greatCenterLinePos = greatCenterLine.transform->pos;
		greatCenterLine.physicsData->infiniteMass = true;
		greatCenterLine.physicsData->invMass = 0;
		for (Body& body : topDogs) {
			Vector3 bodyPos = body.transform->pos;
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
		Vector3 posA = bodyA.transform->pos;
		Vector3 posB = bodyB.transform->pos;
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
			Vector3 posA = bodyA.transform->pos;
			Vector3 posB = bodyB.transform->pos;
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
		auto posIt = cache->begin<components::LocalPosition>();
		auto physicsIt = cache->begin<components::PhysicsData>();
		for (int i = 0; i < cache->getSize(); ++i) {
			auto pos = *posIt;
			auto physics = *physicsIt;
			pos->pos.x += physics->velX * frameTime;
			pos->pos.y += physics->velY * frameTime;
			pos->pos.z += physics->velZ * frameTime;
			physics->velX -= physics->velX * damping;
			physics->velY -= physics->velY * damping;
			physics->velZ -= physics->velZ * damping;
		}

	}

	void collectMoleculeConstraints(middle::GameState* gameState, components::CompCache* cache, std::vector<MoleculeConstraint>& constraints, float targetSeparation) {
		for (int i = 0; i < cache->getSize(); ++i) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, cache->relevantIdVector[i], children);

			MoleculeConstraint moleculeConstraint;
			Body prevBody;
			for (int j = 0; j < children.size(); ++j) {
				auto& childShape = middle::getShape(gameState, children[j].index);
				auto transform = middle::getComponent<components::GlobalTransform>(childShape);
				auto physics = middle::getComponent<components::PhysicsData>(childShape);
				auto childCircle = middle::getComponent<components::Circle>(childShape);
				auto mul = middle::getComponent<components::BubbleMultiplyComponent>(childShape);
				auto fraction = middle::getComponent<components::FractionalComponent>(childShape);
				assert(physics);

				Body body;
				body.id = childShape.id;
				body.physicsData = physics;
				body.transform = transform;
				body.radius = 1;
				if (childCircle) {
					body.radius = childCircle->radius;
				}
				moleculeConstraint.bodies.push_back(body);

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
	const float fieldMargin = 10.0f;
	bool debugField = false;
	bool inverses = true;


	void update(middle::GameState* gameState) override {

		// Collect bubbles

		std::vector<Bubble>bubbles;
		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		auto bubbleTransformIt = bubbleCache->begin<components::GlobalTransform>();
		auto circleIt = bubbleCache->begin<components::Circle>();
		auto physicsIt = bubbleCache->begin<components::PhysicsData>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto bubble = *bubbleIt;
			auto bubbleTransform = *bubbleTransformIt;
			auto circle = *circleIt;
			auto bubblePhysics = *physicsIt;
			auto& bubbleShape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);

			std::vector<middle::Id> interactingChildren;
			std::vector<middle::Id> children;
			middle::getChildren(gameState, bubbleCache->relevantIdVector[i], children);
			for (middle::Id& id : children) {
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
				auto transform = middle::getComponent<components::GlobalTransform>(childShape);
				auto physics = middle::getComponent<components::PhysicsData>(childShape);
				auto childCircle = middle::getComponent<components::Circle>(childShape);
				assert(physics);
				// units use a field radius instead
				float radius = childCircle ? childCircle->radius + fieldMargin : fieldMargin;
				Body body;
				body.id = childId;
				body.transform = transform;
				body.physicsData = physics;
				body.radius = radius;
				bodies.push_back(body);
			}

			Body body;
			body.id = bubbleShape.id;
			body.transform = bubbleTransform;
			body.physicsData = bubblePhysics;
			body.radius = circle->radius;
			bubbles.push_back({
				body,
				bodies,
				});
		}

		// Collect top bubbles
		auto topBubbleIt = topDogBubbleCache->begin<components::TopDogBubbleTag>();
		auto topBubbleTransformIt = topDogBubbleCache->begin<components::GlobalTransform>();
		auto topBubblePhysicsIt = topDogBubbleCache->begin<components::PhysicsData>();
		auto topBubbleCircleIt = topDogBubbleCache->begin<components::Circle>();
		std::vector<Body>topDogBubbles;
		for (int i = 0; i < topDogBubbleCache->getSize(); ++i) {
			auto topBubble = *topBubbleIt;
			auto topBubbleTransform = *topBubbleTransformIt;
			auto topBubblePhysics = *topBubblePhysicsIt;
			auto circle = *topBubbleCircleIt;
			Body body;
			body.id = topDogBubbleCache->relevantIdVector[i];
			body.transform = topBubbleTransform;
			body.physicsData = topBubblePhysics;
			body.radius = circle->radius + fieldMargin;
			topDogBubbles.push_back(body);
		}

		// COLLECT MOLECULE CONSTRAINTS
		std::vector<MoleculeConstraint>moleculeConstraints;
		collectMoleculeConstraints(gameState, mulCache, moleculeConstraints, 10);
		collectMoleculeConstraints(gameState, fractionCache, moleculeConstraints, 10);
		collectMoleculeConstraints(gameState, equalsCache, moleculeConstraints, 30);

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

		// top dog pairs
		std::vector<BodyPair>topDogPairs;
		for (int x = 0; x < topDogBubbles.size(); ++x) {
			for (int y = x + 1; y < topDogBubbles.size(); ++y) {
				topDogPairs.push_back({ topDogBubbles[x], topDogBubbles[y] });
			}
		}
		std::vector<std::vector<BodyPair>>topDogPairVectors = { topDogPairs };


		std::vector<Collision>collisions;
		findSiblingCollisions(pairVectors, collisions);
		findSiblingCollisions(topDogPairVectors, collisions);
		findCollisionsWithOutline(bubbles, collisions);

		//if (greatCenterLines.size() == 1) {
		//	findCollisionsWithGreatCenterLine(topDogBubbles, greatCenterLines[0], collisions);
		//}

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

	}
};

static middle::SystemRegistrar<BubblePhysics> reg("BubblePhysics");
