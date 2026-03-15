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

class NewBubbleOutlineSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* circlessCache;
	components::CompCache* circfullCache;
	components::CompCache* circlessCache2;
	components::CompCache* circfullCache2;

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
	}

	const float startingRadius = 30;
	void update(middle::GameState* gameState) override {
		// add circles
		for (int i = 0; i < circlessCache->getSize(); ++i) {
			auto& circleId = circlessCache->relevantIdVector[i];
			auto circle = middle::attachComponent<components::Circle>(gameState, circleId);
			circle->radius = startingRadius;
		}
		auto sphereIt = circlessCache2->begin<components::Sphere>();
		for (int i = 0; i < circlessCache2->getSize(); ++i) {
			auto& circleId = circlessCache2->relevantIdVector[i];
			auto sphere = *sphereIt;
			auto circle = middle::attachComponent<components::Circle>(gameState, circleId);
			circle->radius = sphere->radius;
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
					totalArea += childCircle->radius * 4;
				}
				auto bubbleUnit = middle::getComponent<components::BubbleUnit>(childShape);
				if(bubbleUnit){
					const float fieldRadius = 10;
					totalArea += fieldRadius;
				}
			}
			const float margin = 10;
			circle->radius = totalArea / 4 + margin;
			if (circle->radius < startingRadius) {
				//circle->radius = startingRadius;
			}
		}

	
		// update bubble masses based on area
		circleIt = circfullCache->begin<components::Circle>();
		auto physicsIt = circfullCache->begin<components::PhysicsData>();
		for (int i = 0; i < circfullCache->getSize(); ++i) {
			auto circle = *circleIt;
			auto physics = *physicsIt;
			physics->mass = circle->radius * circle->radius * PI;
			physics->invMass = 1.0f / physics->mass;
		}
	}
};

static middle::SystemRegistrar<NewBubbleOutlineSystem> reg("NewBubbleOutlineSystem");
