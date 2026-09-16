#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "BubbleComponent.h"
#include "BubbleUnit.h"
#include "component_utils.h"
#include "middle_shape_utils.h"
#include "Layer.h"
#include "bubble_utils.h"
#include "IdRef.h"
#include "SnapRef.h"
#include "TopDogBubbleTag.h"
#include "NonPhysicalBubbleTag.h"

class BubbleLayerSystem : public middle::MiddleGameplaySystem {
public:
	BubbleLayerSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
	}
	components::CompCache* topDogCache;

	void init(middle::GameState* gameState) override {
		topDogCache = middle::newCompCache(gameState, systemName);
		topDogCache->addType<components::BubbleComponent>();
		topDogCache->addType<components::TopDogBubbleTag>();
		topDogCache->addType<components::NonPhysicalBubbleTag>(components::NOTINTERESTED);
	}


	void update(middle::GameState* gameState) override {
		for (middle::Id& id : topDogCache->relevantIdVector) {
			std::stack<middle::Id>idStack;
			std::stack<int>depthStack;
			idStack.push(id);
			depthStack.push(0);

			while (idStack.size() > 0) {
				middle::Id currentId = idStack.top();
				int depth = depthStack.top();
				idStack.pop();
				depthStack.pop();
				auto layer = middle::getComp<components::Layer>(gameState, currentId);
				layer->layer = depth;

				std::vector<middle::Id>children;
				middle::getChildren(gameState, currentId, children);
				for (middle::Id childId : children) {
					idStack.push(childId);
					depthStack.push(depth + 1);
				}
			}
		}

	}
};

static middle::SystemRegistrar<BubbleLayerSystem> reg("BubbleLayerSystem");
