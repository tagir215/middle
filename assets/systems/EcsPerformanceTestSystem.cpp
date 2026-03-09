#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "EcsPerformanceTestConfigs.h"
#include "TestComponent.h"
#include "component_utils.h"

class EcsPerformanceTestSystem : public middle::MiddleGameplaySystem {
public:
	EcsPerformanceTestSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
	}

	void init(middle::GameState* gameState) {

	}
	void update(middle::GameState* gameState) override {

		middle::Id& shapeId = middle::findFirstShapeWithComp(gameState, middle::getTypeId<components::EcsPerformanceTestConfigs>());
		auto& shape = middle::getShape(gameState, shapeId.index);
		auto config = middle::getComponent<components::EcsPerformanceTestConfigs>(shape);

		if (!config) {
			return;
		}

		// calculate entity count with testComp
		int testCompCount = 0;
		middle::loopInstances(gameState, [&testCompCount](int i, middle::Shape& shape) {
			auto testComp = middle::getComponent<components::TestComponent>(shape);
			if (testComp) {
				++testCompCount;
			}
			return true;
			});

		// add new entities with test comps until target count
		while (testCompCount < config->entityCount) {
			auto& newShape = middle::addGhostShape(gameState);
			middle::attachComponent<components::TestComponent>(gameState, newShape.id);
			++testCompCount;
		}

		// delte entiteis with test comps until target count
		if (testCompCount > config->entityCount) {
			middle::loopInstances(gameState, [config, gameState, &testCompCount](int i, middle::Shape& shape) {
				auto testComp = middle::getComponent<components::TestComponent>(shape);
				if (testComp) {
					middle::deleteShape(gameState, i);
					--testCompCount;
				}
				if (testCompCount <= config->entityCount) {
					return false;
				}
				return true;
			});
		}

	}
};

static middle::SystemRegistrar<EcsPerformanceTestSystem> reg("EcsPerformanceTestSystem");
