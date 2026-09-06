#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "TopDogBubbleTag.h"
#include "EditThisTag.h"
#include "middle_shape_utils.h"
#include "component_utils.h"
#include "BubbleEqualsComponent.h"
#include "BubbleComponent.h"
#include "bubble_utils.h"
#include "TextureComponent.h"
#include "Layer.h"
#include "BottomDogBubbleTag.h"

class TopDogSystem : public middle::MiddleGameplaySystem {
	components::CompCache* topDogCache;
	components::CompCache* bottomDogCache;
	components::CompCache* bubbleCache;
	components::CompCache* problemCache;
	components::CompCache* helperCache;

	void init(middle::GameState* gameState) override {
		systemUpdateType = middle::SystemUpdateType::PREFRAME;

		topDogCache = middle::newCompCache(gameState, systemName);
		topDogCache->addType<components::TopDogBubbleTag>();

		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();

		bottomDogCache = middle::newCompCache(gameState, systemName);
		bottomDogCache->addType<components::BottomDogBubbleTag>();
	}

	bool isTopDog(middle::GameState* gameState, middle::Id id) {
		middle::Id parentId = middle::getParent(gameState, id);
		if (parentId.index == middle::UNASSIGNED) {
			return true;
		}
		return false;
	}

	bool isBottomDog(middle::GameState* gameState, middle::Id id) {
		auto layer = middle::getComp<components::Layer>(gameState, id);
		return layer->layer == gameState->bubbleAlgebraState.loadDepth - 1;
	}

	void updateTopDogs(middle::GameState* gameState) {

		// delete top dog components from non top dogs
		for (int i = 0; i < topDogCache->getSize(); ++i) {
			middle::Id id = topDogCache->relevantIdVector[i];
			if (!isTopDog(gameState, id)){
				middle::queueComponentDeletion<components::TopDogBubbleTag>(gameState, id);
			}
		}
		for (middle::Id id : bottomDogCache->relevantIdVector) {
			if (!isBottomDog(gameState, id)) {
				middle::queueComponentDeletion<components::BottomDogBubbleTag>(gameState, id);
			}
		}

		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			middle::Id id = bubbleCache->relevantIdVector[i];
			if (isTopDog(gameState, id)) {
				middle::attachComponent<components::TopDogBubbleTag>(gameState, id);
			}
			if (isBottomDog(gameState, id)) {
				middle::attachComponent<components::BottomDogBubbleTag>(gameState, id);
			}
		}
	}

	template<typename T>
	void transferToTop(middle::GameState* gameState, middle::Id id) {
		middle::queueComponentDeletion<T>(gameState, id);
		int highestContainer = middle::findHighestLevelContainer(gameState, id.index);
		middle::attachComponent<T>(gameState, gameState->ids[highestContainer]);
	}


	void update(middle::GameState* gameState) override {

		updateTopDogs(gameState);
	}
};

static middle::SystemRegistrar<TopDogSystem> reg("TopDogSystem");
