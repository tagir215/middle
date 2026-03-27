#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "BubbleComponent.h"
#include "Circle.h"
#include "component_utils.h"
#include "Position.h"
#include "PhysicsData.h"
#include "BubbleUnit.h"
#include "Sphere.h"
#include "FractionalComponent.h"
#include "bubble_utils.h"


class NewBubbleOutlineSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* circlessCache;
	components::CompCache* circfullCache;
	components::CompCache* circlessCache2;
	components::CompCache* circfullCache2;
	components::CompCache* fractionCache;

	NewBubbleOutlineSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
	}

	void init(middle::GameState* gameState) override {
		circlessCache = middle::newCompCache(gameState);
		circlessCache->addType<components::BubbleComponent>();
		circlessCache->addType<components::Circle>(components::NOTINTERESTED);
		circfullCache = middle::newCompCache(gameState);
		circfullCache->addType<components::BubbleComponent>();
		circfullCache->addType<components::Circle>();
		circfullCache->addType<components::PhysicsData>();

		circlessCache2 = middle::newCompCache(gameState);
		circlessCache2->addType<components::BubbleUnit>();
		circlessCache2->addType<components::Sphere>();
		circlessCache2->addType<components::Circle>(components::NOTINTERESTED);
		circfullCache2 = middle::newCompCache(gameState);
		circfullCache2->addType<components::BubbleUnit>();
		circfullCache2->addType<components::Circle>();
		circfullCache2->addType<components::PhysicsData>();

		fractionCache = middle::newCompCache(gameState);
		fractionCache->addType<components::FractionalComponent>();
		fractionCache->addType<components::LoopSociety>();
	}

	void updateMasses(components::CompCache* cache) {
		// update bubble masses based on area
		auto circleIt = cache->begin<components::Circle>();
		auto physicsIt = cache->begin<components::PhysicsData>();
		for (int i = 0; i < cache->getSize(); ++i) {
			auto circle = *circleIt;
			auto physics = *physicsIt;
			physics->mass = circle->radius * circle->radius * PI;
			physics->invMass = 1.0f / physics->mass;
		}
	}

	const float minBubbleRadius = 10;
	void update(middle::GameState* gameState) override {
		// add circles
		for (int i = 0; i < circlessCache->getSize(); ++i) {
			auto& circleId = circlessCache->relevantIdVector[i];
			auto circle = middle::attachComponent<components::Circle>(gameState, circleId);
			circle->radius = minBubbleRadius;
		}
		auto sphereIt = circlessCache2->begin<components::Sphere>();
		for (int i = 0; i < circlessCache2->getSize(); ++i) {
			auto& circleId = circlessCache2->relevantIdVector[i];
			auto sphere = *sphereIt;
			auto circle = middle::attachComponent<components::Circle>(gameState, circleId);
			circle->radius = sphere->radius;
		}
		auto fractionIt = fractionCache->begin<components::LoopSociety>();
		for (int i = 0; i < fractionCache->getSize(); ++i) {
			auto fractionLoop = *fractionIt;
			middle::Id quotientId = bubble::fractionQuotient(gameState, fractionCache->relevantIdVector[i]);
			auto& quotientShape = middle::getShape(gameState, quotientId.index);
			auto circle = middle::getComponent<components::Circle>(quotientShape);
			float targetRadius = circle->radius;
			for (middle::Id& childId : fractionLoop->loopMemberIds) {
				if (childId == quotientId)
					continue;
				auto& childShape = middle::getShape(gameState, childId.index);
				auto childCircle = middle::getComponent<components::Circle>(childShape);
				childCircle->radius = circle->radius;
			}
		}

		// calculate bubble size
		auto circleIt = circfullCache->begin<components::Circle>();
		for (int i = 0; i < circfullCache->getSize(); ++i) {
			auto circle = *circleIt;
			auto& shape = middle::getShape(gameState, circfullCache->relevantIdVector[i].index);
			std::vector<middle::Id>children;
			middle::getAllChildrenWithComp(gameState, shape.id, children, middle::getTypeId<components::Circle>());
			float totalArea = 0;
			for (middle::Id& childId : children) {
				auto& childShape = middle::getShape(gameState, childId.index);
				auto childCircle = middle::getComponent<components::Circle>(childShape);
				if (childCircle) {
					const float margin = 10;
					float r = childCircle->radius + margin;
					totalArea += r * r * PI;
				}
				auto bubbleUnit = middle::getComponent<components::BubbleUnit>(childShape);
				if(bubbleUnit){
					const float fieldRadius = 10;
					totalArea += fieldRadius;
				}
			}

			float radius = std::sqrt(totalArea / PI);
			if (radius < minBubbleRadius) {
				radius = minBubbleRadius;
			}
			circle->radius = radius;
		}

	
		updateMasses(circfullCache);
		updateMasses(circfullCache2);

	}
};

static middle::SystemRegistrar<NewBubbleOutlineSystem> reg("NewBubbleOutlineSystem");
