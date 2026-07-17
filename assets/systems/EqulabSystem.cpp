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
#include "BubbleVariable.h"
#include "TimerComponent.h"
#include "TopDogBubbleTag.h"
#include "bubble_paths.h"
#include "ActiveBubbleTag.h"
#include "component_utils.h"

class EqulabSystem : public middle::MiddleGameplaySystem {
public:

	enum class SelectType {
		NONE,
		ADD_MULTIPLICATION,
		ADD_POWER
	};

	SelectType currentSelectType = SelectType::NONE;

	components::CompCache* intersectableBubbleCache;
	components::CompCache* intersectableUnitCache;
	components::CompCache* selectedCache;
	components::CompCache* topDogCache;
	components::CompCache* timerCache;
	components::CompCache* activeBubbleCache;

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

		topDogCache = middle::newCompCache(gameState, systemName);
		topDogCache->addType<components::TopDogBubbleTag>();

		timerCache = middle::newCompCache(gameState, systemName);
		timerCache->addType<components::TimerComponent>();
		timerCache->addType<components::BubbleComponent>();

		activeBubbleCache = middle::newCompCache(gameState, systemName);
		activeBubbleCache->addType<components::ActiveBubbleTag>();
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

		// ACTIVITY UPDATE
		float minDistance = std::numeric_limits<float>::max();
		middle::Id closestId;
		for (middle::Id& id : topDogCache->relevantIdVector) {
			middle::Id parentId = middle::getParent(gameState, id);
			middle::Id targetId;
			Vector3 center;
			if (parentId.index != middle::UNASSIGNED) {
				std::vector<middle::Id>children;
				middle::getChildren(gameState, parentId, children);
				assert(children.size() == 2);
				middle::Id idA = children[0];
				middle::Id idB = children[1];
				center = (middle::getShapePosition(gameState, idA.index) +
					middle::getShapePosition(gameState, idB.index)) * 0.5f;
				targetId = parentId;
			}
			else {
				center = middle::getShapePosition(gameState, id.index);
				targetId = id;
			}

			float distSqr = Vector3DistanceSqr(center, gameState->activeCamera.position);
			if (distSqr < minDistance) {
				minDistance = distSqr;
				closestId = targetId;
			}
		}
		// UPDATE ACTIVE TO CLOSEST TO CAMERA
		if (closestId.index != middle::UNASSIGNED) {

			if (activeBubbleCache->relevantIdVector.size() > 0) {
				bool needUpdate = false;
				for (middle::Id& activeId : activeBubbleCache->relevantIdVector) {
					if (activeId != closestId) {
						middle::queueComponentDeletion<components::ActiveBubbleTag>(gameState, activeId);
						needUpdate = true;
					}
				}
				if (needUpdate) {
					middle::queueComponentAttachment<components::ActiveBubbleTag>(gameState, closestId);
				}
			}
			else if (activeBubbleCache->relevantIdVector.size() == 0) {
				middle::queueComponentAttachment<components::ActiveBubbleTag>(gameState, closestId);
			}
		}




		// UI 
		auto equlabUi = [gameState, this]() {
			ImGui::Begin("Bubequ file");
			static char equationName[128] = "";
			ImGui::InputText("Equation name", equationName, IM_ARRAYSIZE(equationName));
			if (ImGui::IsItemFocused()) {
				gameState->inputBlockers.insert(middle::InputBlockers::KEYBOARD_BLOCK);
				gameState->inputBlockers.insert(middle::InputBlockers::MOUSE_BLOCK);
			}

			if (ImGui::Button("Save bubequ")) {
				for (middle::Id& activeId : activeBubbleCache->relevantIdVector) {
					std::string equstring = equlab::bubbleToBubequ(gameState, activeId);
					bubequ::saveBubequ(equationName, equstring);
				}
			}

			ImGui::Separator();
			if (ImGui::Button("Delete bubequ")) {
				for (middle::Id& activeId : activeBubbleCache->relevantIdVector) {
					auto deleteAction = std::make_shared<middle::EditorActionDeleteSingle>(activeId);
					middle::queueAction(gameState, deleteAction);
					gameState->bubbleAlgebraState.bubbleActions.push_back(deleteAction);
				}
			}
			ImGui::End();



			ImGui::Begin("bubequ list");
			std::vector<std::string>filenames = bubequ::getFilenames(bubblePaths::EQUATION_FOLDER);
			for (auto& name : filenames) {
				if (ImGui::Button(name.c_str())) {
					const std::string path = bubblePaths::EQUATION_FOLDER + "/" + name;
					Vector3 camXZPos = gameState->activeCamera.position;
					camXZPos.y = 0;
					auto bubequ = bubequ::loadBubequ(path);
					middle::Id id = equlab::bubequToBubble(gameState, camXZPos, bubequ);
					auto registerAction = std::make_shared<middle::EditorActionRegisterId>(id);
					middle::queueAction(gameState, registerAction);
					gameState->bubbleAlgebraState.bubbleActions.push_back(registerAction);
				}
			}
			ImGui::End();
			};
		gameState->uiSetups.push_back(equlabUi);







		// ACTIONS
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

			middle::Id targetId;
			if (intersectedBubble.index != middle::UNASSIGNED) {
				targetId = intersectedBubble;
			}

			bool cantAdd = false;
			if (intersectedBubble.index != middle::UNASSIGNED) {
				auto& intersectedShape = middle::getShape(gameState, intersectedBubble.index);
				auto unitComp = middle::getComponent<components::BubbleUnit>(intersectedShape);
				auto varComp = middle::getComponent<components::BubbleVariable>(intersectedShape);
				cantAdd = unitComp || varComp;
			}

			// have only one equation at a time..
			if (topDogCache->getSize() > 0 && intersectedBubble.index == middle::UNASSIGNED) {
				cantAdd = true;
			}

			if (!cantAdd && gameState->equlabInput.oneHeld) {
				auto action = std::make_shared<equlab::AddBubble>(intersectedBubble, gameState->input.mouseXZ_PlanePos);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
			}
			else if (!cantAdd && gameState->equlabInput.twoHeld && intersectedBubble.index != middle::UNASSIGNED) {
				auto action = std::make_shared<equlab::AddUnit>(intersectedBubble, gameState->input.mouseXZ_PlanePos);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
			}
			else if (!cantAdd && gameState->equlabInput.threeHeld) {
				auto action = std::make_shared<equlab::AddEquals>(gameState->input.mouseXZ_PlanePos);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
			}
			else if (gameState->equlabInput.fourHeld) {
				auto action = std::make_shared<equlab::Negate>(targetId);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
			}
			else if (gameState->equlabInput.fiveHeld && intersectedBubble.index != middle::UNASSIGNED) {
				auto action = std::make_shared<equlab::Invert>(intersectedBubble);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
			}

			else if (gameState->equlabInput.sixHeld) {
				auto action = std::make_shared<equlab::Delete>(targetId);
				middle::queueAction(gameState, action);
				gameState->bubbleAlgebraState.bubbleActions.push_back(action);
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
