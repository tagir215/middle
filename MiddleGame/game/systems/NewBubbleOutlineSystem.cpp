#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MidComp/BubbleComponent.h"
#include "MidComp/Circle.h"
#include "component_utils.h"
#include "MidComp/Position.h"
#include "MidComp/PhysicsData.h"
#include "MidComp/BubbleUnit.h"
#include "MidComp/Sphere.h"
#include "MidComp/FractionalComponent.h"
#include "bubble_utils.h"
#include "MidComp/TopDogBubbleTag.h"
#include "MidComp/BubbleVariable.h"
#include "MidComp/GlobalTransform.h"
#include "MidComp/LocalScale.h"
#include "MidComp/GlobalRadius.h"


class NewBubbleOutlineSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* circlessCache;
	components::CompCache* circfullCache;

	NewBubbleOutlineSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
	}

	void init(middle::GameState* gameState) override {
		circfullCache = middle::newCompCache(gameState, systemName);
		circfullCache->addType<components::BubbleComponent>();
		circfullCache->addType<components::Circle>();
		circfullCache->addType<components::LocalScale>();
		circfullCache->addType<components::PhysicsData>();
		circfullCache->addType<components::GlobalTransform>();
		circfullCache->addType<components::GlobalRadius>();
	}

	void updateMasses(components::CompCache* cache) {
		// update bubble masses based on area
		auto circleIt = cache->begin<components::Circle>();
		auto physicsIt = cache->begin<components::PhysicsData>();
		auto transformIt = cache->begin<components::GlobalTransform>();
		for (int i = 0; i < cache->getSize(); ++i) {
			auto circle = *circleIt;
			auto physics = *physicsIt;
			auto transform = *transformIt;
			float globalR = circle->radius * transform->scale.x;
			if (globalR != 0) {
				physics->mass = globalR * globalR * PI;
				physics->invMass = 1.0f / physics->mass;
			}
		}
	}

	const float minBubbleRadius = bubble::bubbleAxis;

	void update(middle::GameState* gameState) override {
		//todo ddelete
	}
};

static middle::SystemRegistrar<NewBubbleOutlineSystem> reg("NewBubbleOutlineSystem");
