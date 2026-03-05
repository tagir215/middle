#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "DeleteComponent.h"
#include "middle_shape_utils.h"
#include "bubble_actions.h"
#include "DependencyComponent.h"

class DeletionSystem : public middle::MiddleGameplaySystem {
public:
	DeletionSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
	}
	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {

			// if dependency is deleted, delete this as well
			auto dependency = middle::getComponent<components::DependencyComponent>(shape);
			if (dependency) {
				if (!middle::isShapeAlive(gameState, dependency->idRef.index)) {
					middle::deleteShapeRecursive(gameState, shape.id.index);
				}
				return true;
			}

			// if delete comp delete when frame count counts to 0
			auto deleteComp = middle::getComponent<components::DeleteComponent>(shape);
			if (deleteComp) {
				if (deleteComp->framesUntilDelete <= 0) {
					middle::deleteShapeRecursive(gameState, shape.id.index);
				}
				--deleteComp->framesUntilDelete;
			}
			return true;
			});
	}
};

static middle::SystemRegistrar<DeletionSystem> reg("DeletionSystem");
