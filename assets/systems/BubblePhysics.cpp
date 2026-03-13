#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "PhysicsData.h"
#include "BubbleUnit.h"
#include "BubbleComponent.h"
#include "Position.h"

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

	void update(middle::GameState* gameState) override {
		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		std::vector<std::vector<Unit>>unitVectors;

		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto bubble = *bubbleIt;
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
				unitVectors.push_back(units);
			}
		}

		std::vector<std::vector<UnitPair>>pairVectors;
		for (auto& unitVector : unitVectors) {
			std::vector<UnitPair>pairs;
			for (int i = 0; i < unitVector.size(); ++i) {
				for (int j = i + 1; j < unitVector.size(); ++j) {
					pairs.push_back({ unitVector[i], unitVector[j] });
				}
			}
			pairVectors.push_back(pairs);
		}

		const float attractionForceMagnitude = 20.4f;
		const float fieldRadius = 14.0f;
		const float inverseTime = 1.0f / gameState->frameTime;
		const float inverseFieldRadius = 1.0f / fieldRadius;
		for (std::vector<UnitPair>& pairVector : pairVectors) {
			for (UnitPair& pair : pairVector) {
				auto& unitA = pair.unitA;
				auto& unitB = pair.unitB;
				Vector3 posA = { unitA.pos->posX, unitA.pos->posY, unitA.pos->posZ };
				Vector3 posB = { unitB.pos->posX, unitB.pos->posY, unitB.pos->posZ };
				Vector3 velA = { unitA.physicsData->velX, unitA.physicsData->velY, unitA.physicsData->velZ };
				Vector3 velB = { unitB.physicsData->velX, unitB.physicsData->velY, unitB.physicsData->velZ };
				float dist = Vector3Distance(posA, posB);
				Vector3 dir = Vector3Normalize(Vector3Subtract(posB, posA));

				Vector3 force = { 0,0,0 };
				float relVel = Vector3DotProduct(Vector3Subtract(velB, velA), dir);

				if (dist < fieldRadius) {
					float penetration = fieldRadius - dist;
					float targetRelVel = penetration * 0.5f * inverseTime;
					float relVelDiff = targetRelVel - relVel;
					force -= Vector3Scale(dir, relVelDiff);
				}
				else {
					force = Vector3Scale(dir, attractionForceMagnitude);
				}

				Vector3 acc = Vector3Scale(force, gameState->frameTime);
				unitA.physicsData->velX += acc.x;
				unitA.physicsData->velY += acc.y;
				unitA.physicsData->velZ += acc.z;
				unitB.physicsData->velX -= acc.x;
				unitB.physicsData->velY -= acc.y;
				unitB.physicsData->velZ -= acc.z;
			}
		}

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
	}
};

static middle::SystemRegistrar<BubblePhysics> reg("BubblePhysics");
