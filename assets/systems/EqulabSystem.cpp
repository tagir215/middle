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

class EqulabSystem : public middle::MiddleGameplaySystem {
public:

	enum class SelectType {
		ADD_EQUALS,
		ADD_MULTIPLICATION,
		ADD_POWER
	};

	SelectType currentSelectType = SelectType::ADD_EQUALS;

	components::CompCache* intersectableCache;
	components::CompCache* selectedCache;

	void init(middle::GameState* gameState) override {
		intersectableCache = middle::newCompCache(gameState, systemName);
		intersectableCache->addType<components::MouseIntersectable>();
		intersectableCache->addType<components::BubbleComponent>();

		selectedCache = middle::newCompCache(gameState, systemName);
		selectedCache->addType<components::SelectedComponent>();
		selectedCache->addType<components::BubbleComponent>();
	}

	void update(middle::GameState* gameState) override {

		static const int size = 128;
		static char varLabel[size] = ""; // buffer for scene name input
		auto ui = [gameState]() {
			ImGui::Begin("Equlab");
			ImGui::InputText("variable label", varLabel, size);
			ImGui::End();
			};
		gameState->uiSetups.push_back(ui);


		// MOUSE CLICK ACTIONS
		if (gameState->input.mouseClicked) {
			
			middle::Id intersectedShape;
			auto intersectableIt = intersectableCache->begin<components::MouseIntersectable>();
			for (int i = 0; i < intersectableCache->getSize(); ++i) {
				auto intersectable = *intersectableIt;
				if (intersectable->intersectingTop) {
					intersectedShape = intersectableCache->relevantIdVector[i];
					break;
				}
			}

			if (gameState->gameInput.one) {
				auto action = std::make_shared<equlab::AddBubble>(intersectedShape, gameState->input.mouseXZ_PlanePos);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
			}
			else if (gameState->gameInput.two && intersectedShape.index != middle::UNASSIGNED) {
				auto action = std::make_shared<equlab::AddUnit>(intersectedShape, gameState->input.mouseXZ_PlanePos);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
			}
			else if (gameState->gameInput.three) {
				auto action = std::make_shared<equlab::AddVariable>(intersectedShape, std::string(varLabel), gameState->input.mouseXZ_PlanePos);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
			}

			else if (gameState->gameInput.four) {
				middle::attachComponent<components::SelectedComponent>(gameState, intersectedShape);
				currentSelectType = SelectType::ADD_EQUALS;
			}

			else if (gameState->gameInput.five) {
				middle::attachComponent<components::SelectedComponent>(gameState, intersectedShape);
				currentSelectType = SelectType::ADD_MULTIPLICATION;
			}

			else if (gameState->gameInput.six) {
				middle::attachComponent<components::SelectedComponent>(gameState, intersectedShape);
				currentSelectType = SelectType::ADD_POWER;
			}
		}
		// MOUSE RELEASE ACTIONS	
		if (gameState->input.mouseReleased) {

			middle::Id intersectedShape;
			auto intersectableIt = intersectableCache->begin<components::MouseIntersectable>();
			for (int i = 0; i < intersectableCache->getSize(); ++i) {
				auto intersectable = *intersectableIt;
				if (intersectable->intersectingTop) {
					intersectedShape = intersectableCache->relevantIdVector[i];
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
