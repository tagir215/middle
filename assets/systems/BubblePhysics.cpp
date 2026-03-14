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

class BubblePhysics : public middle::MiddleGameplaySystem {
public:
	components::CompCache* unitCache;
	components::CompCache* bubbleCache;

	void init(middle::GameState* gameState) override {
		unitCache = middle::newCompCache(gameState);
		unitCache->addType<components::BubbleUnit>();
		unitCache->addType<components::Position>();
		unitCache->addType<components::PhysicsData>();
		bubbleCache = middle::newCompCache(gameState);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::Position>();
	}

	struct Unit {
		middle::Id id;
		components::Position* pos;
		components::PhysicsData* physicsData;
	};

	struct UnitPair {
		Unit unitA;
		Unit unitB;
	};

	struct Bubble {
		middle::Id id;
		components::Position* pos;
		std::vector<Unit>units;
	};

	const float attractionForce = 20;
	const float fieldRadius = 10.0f;
	bool debugField = false;
	void update(middle::GameState* gameState) override {
		if (debugField) {
			auto unitIt = unitCache->begin<components::BubbleUnit>();
			for (int i = 0; i < unitCache->getSize(); ++i) {
				auto& shape = middle::getShape(gameState, unitCache->relevantIdVector[i].index);
				auto circle = middle::attachComponent<components::Circle>(gameState, shape.id);
				circle->radius = fieldRadius;
			}
			debugField = false;
		}

		std::vector<Bubble>bubbles;
		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		auto bubblePosIt = bubbleCache->begin<components::Position>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto bubble = *bubbleIt;
			auto bubblePos = *bubblePosIt;
			auto& bubbleShape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);
			std::vector<middle::Id>children;
			middle::getChildrenWithComp(gameState, bubbleShape.id, children, middle::getTypeId<components::BubbleUnit>());
			std::vector<Unit>units;
			for (middle::Id& childId : children) {
				auto& childShape = middle::getShape(gameState, childId.index);
				auto position = middle::getComponent<components::Position>(childShape);
				auto physics = middle::getComponent<components::PhysicsData>(childShape);
				units.push_back({
					childId,
					position,
					physics
					});

			}
			bubbles.push_back({
				bubbleShape.id,
				bubblePos,
				units
				});
		}

		std::vector<std::vector<UnitPair>>pairVectors;
		for (auto& bubble : bubbles) {
			std::vector<UnitPair>pairs;
			for (int i = 0; i < bubble.units.size(); ++i) {
				for (int j = i + 1; j < bubble.units.size(); ++j) {
					pairs.push_back({ bubble.units[i], bubble.units[j] });
				}
			}
			pairVectors.push_back(pairs);
		}

		// attraction forces
		//const float attractionForce = 200;
		//for (Bubble& bubble : bubbles) {
		//	for (Unit& unit : bubble.units) {
		//		float dirX = bubble.pos->posX - unit.pos->posX;
		//		float dirY = bubble.pos->posY - unit.pos->posY;
		//		float dirZ = bubble.pos->posZ - unit.pos->posZ;
		//		Vector3 attractionDir = Vector3Normalize({ dirX, dirY, dirZ });
		//		Vector3 acc = Vector3Scale(attractionDir, attractionForce);
		//		unit.physicsData->velX += acc.x * gameState->frameTime;
		//		unit.physicsData->velY += acc.y * gameState->frameTime;
		//		unit.physicsData->velZ += acc.z * gameState->frameTime;
		//	}
		//}

		// attraction Forces
		for (std::vector<UnitPair>& pairVector : pairVectors) {
			for (UnitPair& pair : pairVector) {
				auto& unitA = pair.unitA;
				auto& unitB = pair.unitB;
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

		const float inverseTime = 1.0f / gameState->frameTime;
		// forces between units
		for (int iteration = 0; iteration < 8; ++iteration) {
			for (std::vector<UnitPair>& pairVector : pairVectors) {
				for (UnitPair& pair : pairVector) {
					auto& unitA = pair.unitA;
					auto& unitB = pair.unitB;
					Vector3 posA = { unitA.pos->posX, unitA.pos->posY, unitA.pos->posZ };
					Vector3 posB = { unitB.pos->posX, unitB.pos->posY, unitB.pos->posZ };
					Vector3 velA = { unitA.physicsData->velX, unitA.physicsData->velY, unitA.physicsData->velZ };
					Vector3 velB = { unitB.physicsData->velX, unitB.physicsData->velY, unitB.physicsData->velZ };
					Vector3 axis = Vector3Normalize(Vector3Subtract(posB, posA));
					float relVel = Vector3DotProduct(Vector3Subtract(velB, velA), axis);
					float dist = Vector3Distance(posA, posB);

					Vector3 impulse = { 0,0,0 };

					// repulsive force
					if (dist < fieldRadius * 2) {
						float targetRelVel = 0;
						float relVelDiff = targetRelVel - relVel;
						impulse += Vector3Scale(axis, relVelDiff * 0.5f * inverseTime);
					}

					Vector3 acc = Vector3Scale(impulse, gameState->frameTime);
					unitA.physicsData->velX -= acc.x;
					unitA.physicsData->velZ -= acc.z;
					unitB.physicsData->velX += acc.x;
					unitB.physicsData->velZ += acc.z;
				}
			}
		}

		// integrating
		auto unitIt = unitCache->begin<components::BubbleUnit>();
		auto unitPosIt = unitCache->begin<components::Position>();
		auto unitPhysicsIt = unitCache->begin<components::PhysicsData>();
		for (int i = 0; i < unitCache->getSize(); ++i) {
			auto unit = *unitIt;
			auto pos = *unitPosIt;
			auto physics = *unitPhysicsIt;
			pos->posX += physics->velX * gameState->frameTime;
			pos->posY += physics->velY * gameState->frameTime;
			pos->posZ += physics->velZ * gameState->frameTime;
		}

		const float stiffness = 0.2f;
		for (int iteration = 0; iteration < 8; ++iteration) {
			for (std::vector<UnitPair>& pairVector : pairVectors) {
				for (UnitPair& pair : pairVector) {
					auto& unitA = pair.unitA;
					auto& unitB = pair.unitB;
					Vector3 posA = { unitA.pos->posX, unitA.pos->posY, unitA.pos->posZ };
					Vector3 posB = { unitB.pos->posX, unitB.pos->posY, unitB.pos->posZ };
					Vector3 axis = Vector3Normalize(Vector3Subtract(posB, posA));
					float dist = Vector3Distance(posA, posB);
					float penetration = (fieldRadius * 2 - dist);
					if (penetration > 0) {
						Vector3 correction = Vector3Scale(axis, penetration * 0.5f * stiffness);
						unitA.pos->posX -= correction.x;
						unitA.pos->posZ -= correction.z;
						unitB.pos->posX += correction.x;
						unitB.pos->posZ += correction.z;
					}

				}
			}
		}

	}
};

static middle::SystemRegistrar<BubblePhysics> reg("BubblePhysics");
