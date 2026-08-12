#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "PhysicsData.h"
#include "BubbleComponent.h"
#include "component_utils.h"
#include "Circle.h"
#include "LoopSociety.h"
#include "Rectangle.h"
#include "TopDogBubbleTag.h"
#include "DeleteComponent.h" 
#include "IdRef.h"
#include "GlobalTransform.h"
#include "LocalPosition.h"
#include "GlobalRadius.h"

class BubblePhysics : public middle::MiddleGameplaySystem {
public:

	BubblePhysics() {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;
	}

	components::CompCache* bubbleCache;
	components::CompCache* mulCache;
	components::CompCache* rectCache;
	components::CompCache* topDogBubbleCache;
	components::CompCache* equalsCache;

	void init(middle::GameState* gameState) override {

		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::GlobalTransform>();
		bubbleCache->addType<components::GlobalRadius>();
		bubbleCache->addType<components::LocalPosition>();
		bubbleCache->addType<components::PhysicsData>();
		bubbleCache->addType<components::Circle>();
		bubbleCache->addType<components::LoopSociety>();
		bubbleCache->addType<components::IdRef>(components::NOTINTERESTED);

		topDogBubbleCache = middle::newCompCache(gameState, systemName);
		topDogBubbleCache->addType<components::TopDogBubbleTag>();
		topDogBubbleCache->addType<components::GlobalTransform>();
		topDogBubbleCache->addType<components::GlobalRadius>();
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
		Matrix m;
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

	struct CircleCircleCollisionResult {
		float penetration;
		Vector3 normal;
		bool collided = false;
	};

	CircleCircleCollisionResult circlesCollisionTest(const Vector3& posA, float rA, const Vector3& posB, float rB) {
		CircleCircleCollisionResult result;
		float distSqr = Vector3DistanceSqr(posA, posB);
		float radiusesSqr = (rA + rB) * (rA + rB);
		if (distSqr < radiusesSqr) {
			result.collided = true;
			result.normal = Vector3Normalize(posB - posA);
			if (distSqr == 0) {
				result.normal = { 1,0,0 };
			}
			float dist = std::sqrt(distSqr);
			float radiuses = std::sqrt(radiusesSqr);
			result.penetration = radiuses - dist;

		}
		return result;
	}

	void findSiblingCollisions(std::vector<std::vector<BodyPair>>& pairVectors, std::vector<Collision>& results) {
		for (std::vector<BodyPair>& pairVector : pairVectors) {
			for (BodyPair& pair : pairVector) {
				Vector3 posA = pair.bodyA.transform->pos;
				Vector3 posB = pair.bodyB.transform->pos;
				float rA = pair.bodyA.radius;
				float rB = pair.bodyB.radius;
				auto circleResult = circlesCollisionTest(posA, rA, posB, rB);
				if (circleResult.collided) {
					Collision collision;
					collision.axis = circleResult.normal;
					collision.bodyA = pair.bodyA;
					collision.bodyB = pair.bodyB;
					collision.penetration = circleResult.penetration;
					results.push_back(collision);
				}
			}
		}
	}

	void findCollisionsWithOutline(std::vector<Bubble>& bubbles, std::vector<Collision>& results) {
		for (Bubble& bubble : bubbles) {
			Body& bubbleBody = bubble.bubbleBody;
			float bubbleGlobalR = bubbleBody.radius;

			Vector3 bubblePos = bubbleBody.transform->pos;
			for (Body& body : bubble.bodies) {
				Vector3 bodyPos = body.transform->pos;
				float dist = Vector3Distance(bubblePos, bodyPos);
				float bodyGlobalR = body.radius;
				if (dist > bubbleGlobalR - bodyGlobalR) {
					Collision collision;
					collision.axis = Vector3Normalize(Vector3Subtract(bubblePos, bodyPos));
					collision.bodyA = bubbleBody;
					collision.bodyB = body;
					collision.penetration = dist - bubbleGlobalR + bodyGlobalR;
					results.push_back(collision);
				}
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

	const float attractionForce = 20;
	const float fieldMargin = 10.0f;
	bool debugField = false;
	bool inverses = true;


	void update(middle::GameState* gameState) override {
		// Collect bubbles

		std::vector<Bubble>bubbles;
		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		auto bubbleTransformIt = bubbleCache->begin<components::GlobalTransform>();
		auto globalRIt = bubbleCache->begin<components::GlobalRadius>();
		auto physicsIt = bubbleCache->begin<components::PhysicsData>();
		for (middle::Id id : bubbleCache->relevantIdVector) {
			auto bubble = *bubbleIt;
			auto bubbleTransform = *bubbleTransformIt;
			auto bubblePhysics = *physicsIt;
			auto globalR = *globalRIt;
			auto& bubbleShape = middle::getShape(gameState, id.index);

			std::vector<middle::Id> interactingChildren;
			std::vector<middle::Id> children;
			middle::getChildren(gameState, id, children);
			for (middle::Id& id : children) {
				auto& childShape = middle::getShape(gameState, id.index);
				if(middle::getComponent<components::PhysicsData>(childShape)) {
					interactingChildren.push_back(id);
				}
			}


			std::vector<Body>bodies;
			for (middle::Id& childId : interactingChildren) {
				auto& childShape = middle::getShape(gameState, childId.index);
				auto transform = middle::getComponent<components::GlobalTransform>(childShape);
				auto physics = middle::getComponent<components::PhysicsData>(childShape);
				auto childGlobalR = middle::getComponent<components::GlobalRadius>(childShape);
				assert(physics);
				Body body;
				body.id = childId;
				body.transform = transform;
				body.physicsData = physics;
				body.radius = childGlobalR->radius + fieldMargin * body.transform->scale.x;
				bodies.push_back(body);
			}

			Body body;
			body.id = bubbleShape.id;
			body.transform = bubbleTransform;
			body.physicsData = bubblePhysics;
			body.radius = globalR->radius;
			bubbles.push_back({
				body,
				bodies,
				});
		}

		// Collect top bubbles
		auto topBubbleIt = topDogBubbleCache->begin<components::TopDogBubbleTag>();
		auto topBubbleTransformIt = topDogBubbleCache->begin<components::GlobalTransform>();
		auto topBubblePhysicsIt = topDogBubbleCache->begin<components::PhysicsData>();
		auto topBubbleCircleIt = topDogBubbleCache->begin<components::GlobalRadius>();
		std::vector<Body>topDogBubbles;
		for (middle::Id id : topDogBubbleCache->relevantIdVector) {
			auto topBubble = *topBubbleIt;
			auto topBubbleTransform = *topBubbleTransformIt;
			auto topBubblePhysics = *topBubblePhysicsIt;
			auto globalR = *topBubbleCircleIt;
			Body body;
			body.id = id;
			body.transform = topBubbleTransform;
			body.physicsData = topBubblePhysics;
			body.radius = globalR->radius + fieldMargin * body.transform->scale.x;
			topDogBubbles.push_back(body);
		}


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

		const float inverseTime = 1.0f / gameState->frameTime;
		// forces between units
		for (int iteration = 0; iteration < 8; ++iteration) {
			for (Collision& collision : collisions) {
				solveVelocity(collision, gameState->frameTime, inverseTime);
			}
		}

		integrate(gameState->frameTime, bubbleCache);

	}
};

static middle::SystemRegistrar<BubblePhysics> reg("BubblePhysics");
