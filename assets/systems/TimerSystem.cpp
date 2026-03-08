#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_component_table.h"
#include "middle_shape_utils.h"
#include "TimerComponent.h"
#include "component_utils.h"

class TimerSystem : public middle::MiddleGameplaySystem {

public:
	void init(middle::GameState* gameState) {

	}
	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto timer = middle::getComponent<components::TimerComponent>(shape);
			if (!timer)
				return true;
			timer->timeLeft -= gameState->frameTime;
			if (timer->timeLeft < 0) {
				middle::queueComponentDeletion<components::TimerComponent>(gameState, shape.id);
			}
			return true;
			});
	}
};

static middle::SystemRegistrar<TimerSystem> reg("TimerSystem");
