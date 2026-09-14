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
#include "InViewTag.h"
#include "TopDogInViewTag.h"

class TopDogSystem : public middle::MiddleGameplaySystem {
	components::CompCache* topDogCache;
	components::CompCache* topDogInViewCache;
	components::CompCache* bubbleCache;
	components::CompCache* bubblesInViewCache;

	void init(middle::GameState* gameState) override {
		systemUpdateType = middle::SystemUpdateType::INITFRAME;

		topDogCache = middle::newCompCache(gameState, systemName);
		topDogCache->addType<components::TopDogBubbleTag>();

		topDogInViewCache = middle::newCompCache(gameState, systemName);
		topDogInViewCache->addType<components::TopDogInViewTag>();

		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();

		bubblesInViewCache = middle::newCompCache(gameState, systemName);
		bubblesInViewCache->addType<components::BubbleComponent>();
		bubblesInViewCache->addType<components::InViewTag>();
	}

	bool isTopDog(middle::GameState* gameState, middle::Id id) {
		middle::Id parentId = middle::getParent(gameState, id);
		if (parentId.index == middle::UNASSIGNED) {
			return true;
		}
		return false;
	}

	bool isTopDogInView(middle::GameState* gameState, middle::Id id) {
		middle::Id parentId = middle::getParent(gameState, id);
		if (!middle::getComp<components::InViewTag>(gameState, id)) {
			return false;
		}
		if (parentId.index == middle::UNASSIGNED) {
			return middle::getComp<components::InViewTag>(gameState, id) != nullptr;
		}
		else {
			return middle::getComp<components::InViewTag>(gameState, parentId) == nullptr;
		}
	}

	void updateTopDogs(middle::GameState* gameState) {

		// delete top dog components from non top dogs
		for (int i = 0; i < topDogCache->getSize(); ++i) {
			middle::Id id = topDogCache->relevantIdVector[i];
			if (!isTopDog(gameState, id)){
				middle::queueComponentDeletion<components::TopDogBubbleTag>(gameState, id);
			}
		}
		for (middle::Id id : topDogInViewCache->relevantIdVector) {
			if (!isTopDogInView(gameState, id)) {
				middle::queueComponentDeletion<components::TopDogInViewTag>(gameState, id);
			}
		}

		for (middle::Id id : bubbleCache->relevantIdVector) {
			if (isTopDog(gameState, id)) {
				middle::attachComponent<components::TopDogBubbleTag>(gameState, id);
			}
		}
		for (middle::Id id : bubblesInViewCache->relevantIdVector) {
			if (isTopDogInView(gameState, id)) {
				middle::attachComponent<components::TopDogInViewTag>(gameState, id);
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
