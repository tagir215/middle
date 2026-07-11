#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "TopDogBubbleTag.h"
#include "BubbleAlgebraProblem.h"
#include "HelperBubbleEquation.h"
#include "EditThisTag.h"
#include "middle_shape_utils.h"
#include "component_utils.h"
#include "BubbleEqualsComponent.h"
#include "BubbleComponent.h"
#include "bubble_utils.h"
#include "TextureComponent.h"

class TopDogSystem : public middle::MiddleGameplaySystem {
	components::CompCache* topDogCache;
	components::CompCache* bubbleCache;
	components::CompCache* problemCache;
	components::CompCache* helperCache;

	void init(middle::GameState* gameState) override {
		topDogCache = middle::newCompCache(gameState);
		topDogCache->addType<components::TopDogBubbleTag>();

		bubbleCache = middle::newCompCache(gameState);
		bubbleCache->addType<components::BubbleComponent>();

		problemCache = middle::newCompCache(gameState);
		problemCache->addType<components::BubbleAlgebraProblem>();

		helperCache = middle::newCompCache(gameState);
		helperCache->addType<components::HelperBubbleEquation>();
	}

	bool isTopDog(middle::GameState* gameState, middle::Id id) {
		middle::Id parentId = middle::getParent(gameState, id);
		if (parentId.index == middle::UNASSIGNED) {
			return true;
		}
		middle::Shape& parentShape = middle::getShape(gameState, parentId.index);
		bool parentIsEquals = middle::getComponent<components::BubbleEqualsComponent>(parentShape) != nullptr;
		return parentIsEquals;
	}

	void updateTopDogs(middle::GameState* gameState) {

		// delete top dog components from non top dogs
		for (int i = 0; i < topDogCache->getSize(); ++i) {
			middle::Id id = topDogCache->relevantIdVector[i];
			if (!isTopDog(gameState, id)){
				middle::queueComponentDeletion<components::TopDogBubbleTag>(gameState, id);
			}
		}

		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			middle::Id id = bubbleCache->relevantIdVector[i];
			if (isTopDog(gameState, id)) {
				middle::attachComponent<components::TopDogBubbleTag>(gameState, id);
			}
		}
	}

	template<typename T>
	void transferToTop(middle::GameState* gameState, middle::Id id) {
		middle::queueComponentDeletion<T>(gameState, id);
		int highestContainer = middle::findHighestLevelContainer(gameState, id.index);
		middle::attachComponent<T>(gameState, gameState->ids[highestContainer]);
	}

	void transferToTopProblem(middle::GameState* gameState, middle::Id id) {
		auto& shape = middle::getShape(gameState, id.index);
		auto algProb = middle::getComponent<components::BubbleAlgebraProblem>(shape);
		bool isEditable = algProb->editable;
		middle::queueComponentDeletion<components::BubbleAlgebraProblem>(gameState, id);
		int highestContainer = middle::findHighestLevelContainer(gameState, id.index);
		auto newComp = middle::attachComponent<components::BubbleAlgebraProblem>(gameState, gameState->ids[highestContainer]);
		newComp->editable = isEditable;
	}


	void update(middle::GameState* gameState) override {

		updateTopDogs(gameState);

		for (middle::Id& id : problemCache->relevantIdVector) {
			if (middle::getParent(gameState, id).index != middle::UNASSIGNED) {
				transferToTopProblem(gameState, id);
			}
		}

		for (middle::Id& id : helperCache->relevantIdVector) {
			if (middle::getParent(gameState, id).index != middle::UNASSIGNED) {
				transferToTop<components::HelperBubbleEquation>(gameState, id);
			}
		}
	}
};

static middle::SystemRegistrar<TopDogSystem> reg("TopDogSystem");
