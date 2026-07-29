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

class BubbleLayerSystem : public middle::MiddleGameplaySystem {
public:
	BubbleLayerSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
	}
	components::CompCache* bubbleLayerCache;

	void init(middle::GameState* gameState) override {

		bubbleLayerCache = middle::newCompCache(gameState, systemName);
		bubbleLayerCache->addType<components::BubbleComponent>();
		bubbleLayerCache->addType<components::Layer>();
		bubbleLayerCache->addType<components::IdRef>(components::NOTINTERESTED);
		bubbleLayerCache->addType<components::SnapRef>(components::NOTINTERESTED);
	}
	void update(middle::GameState* gameState) override {
		auto bubbleLayerIt = bubbleLayerCache->begin<components::Layer>();
		for (middle::Id& id : bubbleLayerCache->relevantIdVector) {
			auto layer = *bubbleLayerIt;
			layer->layer = bubble::findDepth(gameState, id);
		}

		if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED) {

			middle::Id grabbedId = gameState->bubbleAlgebraState.grabbedId;
			auto& grabbedShape = middle::getShape(gameState, grabbedId.index);
			auto idRef = middle::getComponent<components::IdRef>(grabbedShape);

			if (idRef && middle::isValidId(gameState, idRef->idRef)) {

				auto& refShape = middle::getShape(gameState, idRef->idRef.index);
				int depth = bubble::findDepth(gameState, refShape.id);

				std::vector<middle::Id> children;
				middle::getAllChildren(gameState, grabbedId, children);
				int layerOffset = depth;

				for (middle::Id& childId : children) {
					auto& childShape = middle::getShape(gameState, childId.index);
					auto layer = middle::getComponent<components::Layer>(childShape);
					if (layer) {
						layer->layer += layerOffset;
					}
				}
			}

		}
	}
};

static middle::SystemRegistrar<BubbleLayerSystem> reg("BubbleLayerSystem");
