#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "BubbleComponent.h"
#include "Circle.h"
#include "component_utils.h"

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
	}

	const float startingRadius = 30;
	void update(middle::GameState* gameState) override {
		// add circles
		for (int i = 0; i < circlessCache->getSize(); ++i) {
			auto& circleId = circlessCache->relevantIdVector[i];
			auto circle = middle::attachComponent<components::Circle>(gameState, circleId);
			circle->radius = startingRadius;
		}

		auto bubbleIt = circfullCache->begin<components::BubbleComponent>();
		auto circleIt = circfullCache->begin<components::Circle>();
		for (int i = 0; i < circfullCache->getSize(); ++i) {
			auto bubble = *bubbleIt;
			auto circle = *circleIt;
		}

	}
};

static middle::SystemRegistrar<NewBubbleOutlineSystem> reg("NewBubbleOutlineSystem");
