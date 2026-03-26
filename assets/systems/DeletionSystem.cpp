#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "DeleteComponent.h"
#include "middle_shape_utils.h"
#include "bubble_actions.h"
#include "DependencyComponent.h"

class DeletionSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* dependencyCache;
	components::CompCache* deleteCache;

	DeletionSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;
	}
	void init(middle::GameState* gameState) {
		dependencyCache = middle::newCompCache(gameState);
		dependencyCache->addType<components::DependencyComponent>();

		deleteCache = middle::newCompCache(gameState);
		deleteCache->addType<components::DeleteComponent>();
	}
	void update(middle::GameState* gameState) override {

		auto dependencyIt = dependencyCache->begin<components::DependencyComponent>();
		auto deleteIt = deleteCache->begin<components::DeleteComponent>();

		for (int i = 0; i < dependencyCache->getSize(); ++i) {
			// if dependency is deleted, delete this as well
			auto dependency = *dependencyIt;
			if (!middle::isShapeAlive(gameState, dependency->idRef.index)) {
				middle::Shape& shape = middle::getShape(gameState, dependencyCache->relevantIdVector[i].index);
				middle::queueAction(gameState, std::make_shared<middle::EditorActionDeleteSingle>(shape.id));
			}
		}

		for (int i = 0; i < deleteCache->getSize(); ++i) {
			// if delete comp delete when frame count counts to 0
			auto deleteComp = *deleteIt;
			middle::Shape& shape = middle::getShape(gameState, deleteCache->relevantIdVector[i].index);
			if (deleteComp->framesUntilDelete <= 0) {
				middle::queueAction(gameState, std::make_shared<middle::EditorActionDeleteSingle>(shape.id));
			}
			--deleteComp->framesUntilDelete;
		}
	}
};

static middle::SystemRegistrar<DeletionSystem> reg("DeletionSystem");
