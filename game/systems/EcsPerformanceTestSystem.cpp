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

	components::CompCache* testCache;
	components::CompCache* configCache;

	void init(middle::GameState* gameState) {
		testCache = middle::newCompCache(gameState, systemName);
		testCache->addType<components::TestComponent>();
		configCache = middle::newCompCache(gameState, systemName);
		configCache->addType<components::EcsPerformanceTestConfigs>();
	}
	void update(middle::GameState* gameState) override {

		components::EcsPerformanceTestConfigs* config = nullptr;
		auto configIt = configCache->begin<components::EcsPerformanceTestConfigs>();
		if (configCache->getSize() > 0) {
			config = *configIt;
		}
		if (!config) {
			return;
		}


		// calculate entity count with testComp
		int testCompCount = testCache->getSize();

		// add new entities with test comps until target count
		while (testCompCount < config->entityCount) {
			auto& newShape = middle::addGhostShape(gameState);
			middle::attachComponent<components::TestComponent>(gameState, newShape.id);
			++testCompCount;
		}

		// delete entities with test comps until target count
		if (testCompCount > config->entityCount) {
			auto testIt = testCache->begin<components::TestComponent>();

			for (int i = 0; i < testCache->getSize(); ++i) {
				auto testComp = *testIt;
				middle::Id& id = testCache->relevantIdVector[i];
				middle::queueAction(gameState, std::make_shared<middle::EditorActionDeleteSingle>(id));
				--testCompCount;
			}
		}

	}
};

static middle::SystemRegistrar<EcsPerformanceTestSystem> reg("EcsPerformanceTestSystem");
