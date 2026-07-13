#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "equlab_actions.h"
#include "MouseIntersectable.h"
#include "BubbleComponent.h"
#include "SelectedComponent.h"
#include "component_utils.h"
#include "imgui.h"
#include "BubbleUnit.h"

class EqulabSystem : public middle::MiddleGameplaySystem {
public:

	enum class SelectType {
		ADD_EQUALS,
		ADD_MULTIPLICATION,
		ADD_POWER
	};

	SelectType currentSelectType = SelectType::ADD_EQUALS;

	components::CompCache* intersectableBubbleCache;
	components::CompCache* intersectableUnitCache;
	components::CompCache* selectedCache;

	void init(middle::GameState* gameState) override {
		intersectableBubbleCache = middle::newCompCache(gameState, systemName);
		intersectableBubbleCache->addType<components::MouseIntersectable>();
		intersectableBubbleCache->addType<components::BubbleComponent>();

		intersectableUnitCache = middle::newCompCache(gameState, systemName);
		intersectableUnitCache->addType<components::MouseIntersectable>();
		intersectableUnitCache->addType<components::BubbleUnit>();

		selectedCache = middle::newCompCache(gameState, systemName);
		selectedCache->addType<components::SelectedComponent>();
		selectedCache->addType<components::BubbleComponent>();

	}

	void update(middle::GameState* gameState) override {


		// MOUSE CLICK ACTIONS
		if (gameState->input.mouseClicked) {
			
			middle::Id intersectedBubble;
			auto intersectableBubbleIt = intersectableBubbleCache->begin<components::MouseIntersectable>();
			for (int i = 0; i < intersectableBubbleCache->getSize(); ++i) {
				auto intersectable = *intersectableBubbleIt;
				if (intersectable->intersectingTop) {
					intersectedBubble = intersectableBubbleCache->relevantIdVector[i];
					break;
				}
			}

			middle::Id intersectedUnit;
			auto intersectableUnitIt = intersectableUnitCache->begin<components::MouseIntersectable>();
			for (int i = 0; i < intersectableUnitCache->getSize(); ++i) {
				auto intersectable = *intersectableUnitIt;
				if (intersectable->intersectingTop) {
					intersectedUnit = intersectableUnitCache->relevantIdVector[i];
					break;
				}
			}

			middle::Id targetId;
			if (intersectedBubble.index != middle::UNASSIGNED) {
				targetId = intersectedBubble;
			}
			if (intersectedUnit.index != middle::UNASSIGNED) {
				targetId = intersectedUnit;
			}


			if (gameState->equlabInput.oneHeld) {
				auto action = std::make_shared<equlab::AddBubble>(intersectedBubble, gameState->input.mouseXZ_PlanePos);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
			}
			else if (gameState->equlabInput.twoHeld && intersectedBubble.index != middle::UNASSIGNED) {
				auto action = std::make_shared<equlab::AddUnit>(intersectedBubble, gameState->input.mouseXZ_PlanePos);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
			}
			else if (gameState->equlabInput.threeHeld) {
				auto action = std::make_shared<equlab::Negate>(targetId);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
			}
			else if (gameState->equlabInput.fourHeld && intersectedBubble.index != middle::UNASSIGNED) {
				auto action = std::make_shared<equlab::Invert>(intersectedBubble);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
			}
			else if (gameState->equlabInput.sixHeld) {
				auto action = std::make_shared<equlab::Delete>(targetId);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
			}


			else if (gameState->equlabInput.zeroHeld) {
				middle::attachComponent<components::SelectedComponent>(gameState, intersectedBubble);
				currentSelectType = SelectType::ADD_EQUALS;
			}

			else if (gameState->equlabInput.nineHeld) {
				middle::attachComponent<components::SelectedComponent>(gameState, intersectedBubble);
				currentSelectType = SelectType::ADD_MULTIPLICATION;
			}

			else if (gameState->equlabInput.eightHeld) {
				middle::attachComponent<components::SelectedComponent>(gameState, intersectedBubble);
				currentSelectType = SelectType::ADD_POWER;
			}
		}
		// MOUSE RELEASE ACTIONS	
		if (gameState->input.mouseReleased) {

			middle::Id intersectedShape;
			auto intersectableIt = intersectableBubbleCache->begin<components::MouseIntersectable>();
			for (int i = 0; i < intersectableBubbleCache->getSize(); ++i) {
				auto intersectable = *intersectableIt;
				if (intersectable->intersectingTop) {
					intersectedShape = intersectableBubbleCache->relevantIdVector[i];
					break;
				}
			}

			// there should bne 1
			if (selectedCache->getSize() != 1) {
				return;
			}

			if (currentSelectType == SelectType::ADD_EQUALS) {
				middle::Id idA = selectedCache->relevantIdVector[0];
				middle::Id idB = intersectedShape;
				auto connect = std::make_shared<equlab::ConnectEqualsLink>(idA, idB);
				middle::queueAction(gameState, connect);
				gameState->bubbleAlgebraState.bubbleActions.push_back(connect);
			}

			if (currentSelectType == SelectType::ADD_MULTIPLICATION) {
				middle::Id idA = selectedCache->relevantIdVector[0];
				middle::Id idB = intersectedShape;
				auto connect = std::make_shared<equlab::ConnectMultiplicationLink>(idB, idA);
				middle::queueAction(gameState, connect);
				gameState->bubbleAlgebraState.bubbleActions.push_back(connect);
			}

			if (currentSelectType == SelectType::ADD_POWER) {
				middle::Id idA = selectedCache->relevantIdVector[0];
				middle::Id idB = intersectedShape;
				auto connect = std::make_shared<equlab::ConnectPowerLink>(idA, idB);
				middle::queueAction(gameState, connect);
				gameState->bubbleAlgebraState.bubbleActions.push_back(connect);
			}

			middle::queueComponentDeletion<components::SelectedComponent>(gameState, selectedCache->relevantIdVector[0]);
		}
	}
};

static middle::SystemRegistrar<EqulabSystem> reg("EqulabSystem");
