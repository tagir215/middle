#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_component_table.h"
#include "middle_shape_utils.h"
#include "TimerComponent.h"
#include "component_utils.h"

class TimerSystem : public middle::MiddleGameplaySystem {

public:
	components::CompCache* cache;

	void init(middle::GameState* gameState) {
		cache = middle::newCompCache(gameState);
		cache->addType<components::TimerComponent>();
	}
	void update(middle::GameState* gameState) override {
		auto timerIt = cache->begin<components::TimerComponent>();
		for (int i = 0; i < cache->getSize(); ++i) {
			auto timer = *timerIt;
			middle::Id& id = cache->relevantIdVector[i];
			timer->timeLeft -= gameState->frameTime;
			if (timer->timeLeft < 0) {
				middle::queueComponentDeletion<components::TimerComponent>(gameState, id);
			}
		}
	}
};

static middle::SystemRegistrar<TimerSystem> reg("TimerSystem");
