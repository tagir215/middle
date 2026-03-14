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
	const float fieldRadius = 20.0f;
	bool debugField = true;
	void update(middle::GameState* gameState) override {
		if (debugField) {
			auto unitIt = unitCache->begin<components::BubbleUnit>();
			for (int i = 0; i < unitCache->getSize(); ++i) {
				auto& shape = middle::getShape(gameState, unitCache->relevantIdVector[i].index);
				auto circle = middle::attachComponent<components::Circle>(gameState, shape.id);
				circle->radius = fieldRadius;
			}
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
		const float inverseTime = 1.0f / gameState->frameTime;
		// forces between units
		for (int iteration = 0; iteration < 30; ++iteration) {
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

					// attraction force
					impulse += Vector3Scale(axis, -attractionForce);

					// repulsive force
					if (dist < fieldRadius * 2) {
						float targetRelVel = 0;
						float relVelDiff = targetRelVel - relVel;
						impulse += Vector3Scale(axis, relVelDiff * 0.5f * inverseTime);
					}

					Vector3 acc = Vector3Scale(impulse, gameState->frameTime);
					unitA.physicsData->velX -= acc.x;
					unitA.physicsData->velY -= acc.y;
					unitA.physicsData->velZ -= acc.z;
					unitB.physicsData->velX += acc.x;
					unitB.physicsData->velY += acc.y;
					unitB.physicsData->velZ += acc.z;

						velA = { unitA.physicsData->velX, unitA.physicsData->velY, unitA.physicsData->velZ };
						velB = { unitB.physicsData->velX, unitB.physicsData->velY, unitB.physicsData->velZ };
						axis = Vector3Normalize(Vector3Subtract(posB, posA));
						relVel = Vector3DotProduct(Vector3Subtract(velB, velA), axis);
						int a = 0;
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

		for (int iteration = 0; iteration < 1; ++iteration) {
			for (std::vector<UnitPair>& pairVector : pairVectors) {
				for (UnitPair& pair : pairVector) {
					auto& unitA = pair.unitA;
					auto& unitB = pair.unitB;
					Vector3 posA = { unitA.pos->posX, unitA.pos->posY, unitA.pos->posZ };
					Vector3 posB = { unitB.pos->posX, unitB.pos->posY, unitB.pos->posZ };
					Vector3 axis = Vector3Normalize(Vector3Subtract(posB, posA));
					float dist = Vector3Distance(posA, posB);
					float penetration = (fieldRadius - dist) * 0.5f;
					if (dist < fieldRadius) {
						float penetration = fieldRadius - dist;
						Vector3 correction = Vector3Scale(axis, penetration * 0.5f);

						unitA.pos->posX -= correction.x;
						unitA.pos->posY -= correction.y;
						unitA.pos->posZ -= correction.z;
						unitB.pos->posX += correction.x;
						unitB.pos->posY += correction.y;
						unitB.pos->posZ += correction.z;
					}

				}
			}
		}

	}
};

static middle::SystemRegistrar<BubblePhysics> reg("BubblePhysics");
