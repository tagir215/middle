#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "DeleteComponent.h"
#include "middle_shape_utils.h"

class DeletionSystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto deleteComp = middle::getComponent<components::DeleteComponent>(shape);
			if (!deleteComp)
				return true;

			if (deleteComp->framesUntilDelete <= 0) {
				middle::deleteShapeRecursive(gameState, shape.id.index);
			}
			--deleteComp->framesUntilDelete;
			});
	}
};

static middle::SystemRegistrar<DeletionSystem> reg("DeletionSystem");
