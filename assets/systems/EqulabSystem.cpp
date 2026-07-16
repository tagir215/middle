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
#include "alg_file_utils.h"
#include "LoopSociety.h"
#include "BubbleEqualsComponent.h"

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
	components::CompCache* loopCache;

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

		loopCache = middle::newCompCache(gameState, systemName);
		loopCache->addType<components::LoopSociety>();
	}

	std::string keyToString(middle::GameState* gameState) {
		auto& in = gameState->equlabInput;
		if (in.aClicked) return "a";
		if (in.bClicked) return "b";
		if (in.cClicked) return "c";
		if (in.dClicked) return "d";
		if (in.eClicked) return "e";
		if (in.fClicked) return "f";
		if (in.gClicked) return "g";
		if (in.hClicked) return "h";
		if (in.iClicked) return "i";
		if (in.jClicked) return "j";
		if (in.kClicked) return "k";
		if (in.lClicked) return "l";
		if (in.mClicked) return "m";
		if (in.nClicked) return "n";
		if (in.oClicked) return "o";
		if (in.pClicked) return "p";
		if (in.qClicked) return "q";
		if (in.rClicked) return "r";
		if (in.sClicked) return "s";
		if (in.tClicked) return "t";
		if (in.uClicked) return "u";
		if (in.vClicked) return "v";
		if (in.wClicked) return "w";
		if (in.xClicked) return "x";
		if (in.yClicked) return "y";
		if (in.zClicked) return "z";
		return "";
	}

	void update(middle::GameState* gameState) override {

		auto testui = [gameState, this]() {
			ImGui::Begin("test parsing bubequ");
			if (ImGui::Button("TEST LOAD")) {
				auto scope = bubequ::parseBubequ("../assets/equations/test.bubequ");
				middle::Id interesting = equlab::bubequToBubble(gameState,
					gameState->input.mouseXZ_PlanePos, "../assets/equations/test.bubequ");
				int a = 0;
			}
			if (ImGui::Button("TEST SAVE")) {
				middle::Id targetId;

				auto bubIt = loopCache->begin<components::LoopSociety>();
				for (int i = 0; i < loopCache->getSize(); ++i) {
					auto loop = *bubIt;
					// TODO 
					if (loop->parentLoopId.index == middle::UNASSIGNED){
						middle::Id relId = loopCache->relevantIdVector[i];
						auto relShape = middle::getShape(gameState, relId.index);
						if (middle::getComponent<components::BubbleComponent>(relShape)
							|| middle::getComponent<components::BubbleEqualsComponent>(relShape)) {
							targetId = relId;
							break;
						}
					}
				}
				if (targetId.index != middle::UNASSIGNED) {
					std::string equstring = equlab::bubbleToBubequ(gameState, targetId);
					bubequ::saveBubequ(equstring);
				}
			}
			ImGui::End();
			};
		gameState->uiSetups.push_back(testui);

		std::string keyString = keyToString(gameState);
		if (keyString != "") {
			middle::Id intersectedBubble;
			auto intersectableBubbleIt = intersectableBubbleCache->begin<components::MouseIntersectable>();
			for (int i = 0; i < intersectableBubbleCache->getSize(); ++i) {
				auto intersectable = *intersectableBubbleIt;
				if (intersectable->intersectingTop) {
					intersectedBubble = intersectableBubbleCache->relevantIdVector[i];
					break;
				}
			}
			if (intersectedBubble.index != middle::UNASSIGNED) {
				auto action = std::make_shared<equlab::AddLabelCharacterToVariable>(intersectedBubble, keyString);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
			}
		}

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
		if (gameState->input.mouseReleased && selectedCache->getSize() == 1) {

			middle::Id intersectedShape;
			auto intersectableIt = intersectableBubbleCache->begin<components::MouseIntersectable>();
			for (int i = 0; i < intersectableBubbleCache->getSize(); ++i) {
				auto intersectable = *intersectableIt;
				if (intersectable->intersectingTop) {
					intersectedShape = intersectableBubbleCache->relevantIdVector[i];
					break;
				}
			}

			middle::Id selectedId = selectedCache->relevantIdVector[0];

			if (intersectedShape.index == middle::UNASSIGNED) {
				return;
			}
			if (intersectedShape == selectedId) {
				return;
			}

			if (currentSelectType == SelectType::ADD_EQUALS) {
				middle::Id idA = selectedId;
				middle::Id idB = intersectedShape;
				auto connect = std::make_shared<equlab::ConnectEqualsLink>(idA, idB);
				middle::queueAction(gameState, connect);
				gameState->bubbleAlgebraState.bubbleActions.push_back(connect);
			}

			if (currentSelectType == SelectType::ADD_MULTIPLICATION) {
				middle::Id idA = selectedId;
				middle::Id idB = intersectedShape;
				auto connect = std::make_shared<equlab::ConnectMultiplicationLink>(idB, idA);
				middle::queueAction(gameState, connect);
				gameState->bubbleAlgebraState.bubbleActions.push_back(connect);
			}

			if (currentSelectType == SelectType::ADD_POWER) {
				middle::Id idA = selectedId;
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
