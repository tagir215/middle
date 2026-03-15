#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "BubbleComponent.h"
#include "Circle.h"
#include "component_utils.h"
#include "Position.h"
#include "PhysicsData.h"

class NewBubbleOutlineSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* circlessCache;
	components::CompCache* circfullCache;

	void init(middle::GameState* gameState) override {
		circlessCache = middle::newCompCache(gameState);
		circlessCache->addType<components::BubbleComponent>();
		circlessCache->addType<components::Circle>(components::NOTINTERESTED);
		circfullCache = middle::newCompCache(gameState);
		circfullCache->addType<components::BubbleComponent>();
		circfullCache->addType<components::Circle>();
		circfullCache->addType<components::PhysicsData>();
	}

	const float startingRadius = 30;
	void update(middle::GameState* gameState) override {
		// add circles
		for (int i = 0; i < circlessCache->getSize(); ++i) {
			auto& circleId = circlessCache->relevantIdVector[i];
			auto circle = middle::attachComponent<components::Circle>(gameState, circleId);
			circle->radius = startingRadius;
		}

		// calculate bubble size
		auto circleIt = circfullCache->begin<components::Circle>();
		for (int i = 0; i < circfullCache->getSize(); ++i) {
			auto circle = *circleIt;
			auto& shape = middle::getShape(gameState, circfullCache->relevantIdVector[i].index);
			std::vector<middle::Id>children;
			middle::getChildrenWithComp(gameState, shape.id, children, middle::getTypeId<components::Circle>());
			float totalArea = 0;
			for (middle::Id& childId : children) {
				auto& shape = middle::getShape(gameState, childId.index);
				auto childCircle = middle::getComponent<components::Circle>(shape);
				totalArea += childCircle->radius * 4;
			}
			const float margin =10;
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
