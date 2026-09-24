#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "DeleteComponent.h"
#include "IdRef.h"
#include "BubbleMultiplyComponent.h"
#include "bubble_actions.h"
#include "InventoryItem.h"
#include "component_utils.h"
#include "PlacementComponent.h"
#include "BubbleVariable.h"
#include "TopDogBubbleTag.h"
#include "bubble_utils.h"
#include "UiComponent.h"
#include "BubbleAlgebraLevelConfigs.h"
#include "RuntimeHiddenTag.h"
#include "Layer.h"
#include "ProcedureContainer.h"
#include "BubbleEqualsVariable.h"
#include "bubble_constants.h"
#include "SnapRef.h"
#include "TimerComponent.h"
#include "InsertableBubble.h"
#include "imgui.h"
#include "BubblePowerComponent.h"
#include "IntersectingTag.h"
#include "QueuedForSaveTag.h"
#include "BubbleManipulatable.h"
#include "Inventory.h"
#include "bubble_animations.h"

class BubbleModificationSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* deletionCache;
	components::CompCache* intersectingCache;
	components::CompCache* levelConfigsCache;
	components::CompCache* uiCompCache;
	components::CompCache* procContainerCache;
	components::CompCache* inventoryCache;

	void init(middle::GameState* gameState) {
		deletionCache = middle::newCompCache(gameState, systemName);
		deletionCache->addType<components::DeleteComponent>();
		deletionCache->addType<components::IdRef>();

		intersectingCache = middle::newCompCache(gameState, systemName);
		intersectingCache->addType<components::IntersectingTag>();
		intersectingCache->addType<components::BubbleManipulatable>();
		intersectingCache->addType<components::DeleteComponent>(components::NOTINTERESTED);

		levelConfigsCache = middle::newCompCache(gameState, systemName);
		levelConfigsCache->addType<components::BubbleAlgebraLevelConfigs>();

		uiCompCache = middle::newCompCache(gameState, systemName);
		uiCompCache->addType<components::UiComponent>();

		procContainerCache = middle::newCompCache(gameState, systemName);
		procContainerCache->addType<components::ProcedureContainer>();

		inventoryCache = middle::newCompCache(gameState, systemName);
		inventoryCache->addType<components::Inventory>();
	}

	bool isMultiplicationConnection(middle::GameState* gameState, middle::Shape& parentShape) {
		auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);
		if (mulComp) {
			return true;
		}
		return false;
	}

	void substitute(middle::GameState* gameState, middle::Shape& intersectedShape, middle::Shape& deletionRefShape, middle::Shape& shapeForDeletion) {

		middle::Id parentId = middle::getParent(gameState, deletionRefShape.id);
		if (!bubble::isEqualsBubble(gameState, parentId)) {
			return;
		}

		middle::Id otherFunc = bubble::getOtherFromContainerOf2(gameState, deletionRefShape.id);
		std::shared_ptr<bubbleActions::BubbleAction>action;
		if (otherFunc.index != middle::UNASSIGNED) {
			action = std::make_shared<bubbleActions::SubstituteFunction>(intersectedShape.id, deletionRefShape.id);
		}
		else {
			action = std::make_shared<bubbleActions::Substitute>(intersectedShape.id, deletionRefShape.id);
		}

		bubble::queueBubbleAction(gameState, intersectedShape.id, action);
	}

	void tryCombine(middle::GameState* gameState, middle::Shape& refParent, middle::Shape& refShape, middle::Shape& intersectedShape) {

		// is multiplication connection
		if (middle::getComponent<components::BubbleMultiplyComponent>(refParent)) {
			auto multiply = std::make_shared<bubbleActions::ExecuteMultiplication>(refShape.id, intersectedShape.id);
			bubble::queueBubbleAction(gameState, intersectedShape.id, multiply);
			return;
		}
		else if (middle::getComponent<components::BubblePowerComponent>(refParent)) {
			auto doPower = std::make_shared<bubbleActions::ExecutePower>(refShape.id, intersectedShape.id);
			bubble::queueBubbleAction(gameState, intersectedShape.id, doPower);
			return;
		}
		else if (bubble::isSummation(gameState, refParent.id)) {
			auto summation = std::make_shared<bubbleActions::ExpandSummation>(refParent.id);
			bubble::queueBubbleAction(gameState, intersectedShape.id, summation);
			return;
		}
		// else is addition connection
		else {
			auto add = std::make_shared<bubbleActions::ExecuteAddition>(refShape.id, intersectedShape.id);
			auto addAnimation = std::make_shared<bubbleAnimations::AdditionAnimation>(add);
			bubble::queueBubbleAction(gameState, intersectedShape.id, addAnimation);
			return;
		}
	}

	middle::Id copyOfInsertItem(middle::GameState* gameState) {
		if (inventoryCache->relevantIdVector.size() == 1) {
			middle::Id invId = inventoryCache->relevantIdVector[0];
			auto inv = middle::getComp<components::Inventory>(gameState, invId);
			auto loop = middle::getComp<components::LoopSociety>(gameState, invId);
			middle::Id childId = loop->loopMemberIds[inv->activeIndex];
			middle::Id resultId;
			if (inv->invert) {
				 resultId = bubbleActions::createInverseReplacementShape(gameState, childId);
			}
			else {
				resultId = middle::deepCopyShape(gameState, childId.index);
			}
			if (inv->negate) {
				middle::Id tempId = resultId;
				resultId = bubbleActions::createNegatedReplacementShape(gameState, resultId);
				middle::deepCopyShape(gameState, tempId.index);
			}
			return resultId;
		}
		return middle::Id();
	}

	void insertOperation(middle::GameState* gameState, int actionType, middle::Id intersectingId) {
		std::shared_ptr<bubbleActions::BubbleAction>action;

		if (actionType == bubbleInventoryItemType::NEW_ADDITION_TERM) {
			middle::Id copyId = copyOfInsertItem(gameState);
			action = std::make_shared<bubbleActions::NewAdditionTerm>(intersectingId, copyId, gameState->input.mouseXZ_PlanePos);
		}
		else if (actionType == bubbleInventoryItemType::NEW_MULTIPLICATION_TERM) {
			middle::Id copyId = copyOfInsertItem(gameState);
			action = std::make_shared<bubbleActions::NewMultiplicationTerm>(intersectingId, copyId, gameState->input.mouseXZ_PlanePos);
		}
		else if (actionType == bubbleInventoryItemType::NEW_POWER_TERM) {
			middle::Id copyId = copyOfInsertItem(gameState);
			action = std::make_shared<bubbleActions::NewPowerTerm>(intersectingId, copyId, gameState->input.mouseXZ_PlanePos);
		}
		else if (actionType == bubbleInventoryItemType::INSERT_X_OVER_X) {
			middle::Id copyId = copyOfInsertItem(gameState);
			action = std::make_shared<bubbleActions::InsertAsXOverX>(intersectingId, copyId, gameState->input.mouseXZ_PlanePos);
		}
		else if (actionType == bubbleInventoryItemType::INSERT_X_MINUS_X) {
			middle::Id copyId = copyOfInsertItem(gameState);
			action = std::make_shared<bubbleActions::InsertAsXMinusX>(intersectingId, copyId, gameState->input.mouseXZ_PlanePos);
		}

		if (action) {
			bubble::queueBubbleAction(gameState, intersectingId, action);
		}
		else {
			queueSound(gameState, bubbleSounds::ERROR_SOUND);
		}
	}

	void microOperation(middle::GameState* gameState, int actionType, middle::Id& refId, middle::Shape& intersectedShape) {
		std::shared_ptr<bubbleActions::BubbleAction>action;

		// pop as long as not multiplication
		if (actionType == bubbleInventoryItemType::POP) {
			middle::Id& parentId = middle::getParent(gameState, intersectedShape.id);
			if (parentId.index == middle::UNASSIGNED) {
				queueSound(gameState, bubbleSounds::ERROR_SOUND);
				return;
			}
			auto& parentShape = middle::getShape(gameState, parentId.index);
			if (!isMultiplicationConnection(gameState, parentShape)) {
				action = std::make_shared<bubbleActions::Pop>(intersectedShape.id);
			}
		}

		else if (actionType == bubbleInventoryItemType::PROCEDURE) {
			assert(false);
		}
		else if (actionType == bubbleInventoryItemType::MUL_ONE) {
			action = std::make_shared<bubbleActions::MulOne>(intersectedShape.id);
		}
		else if (actionType == bubbleInventoryItemType::MUL_NEGATIVE_ONE) {
			action = std::make_shared<bubbleActions::MulNegativeOne>(intersectedShape.id);
		}
		else if (actionType == bubbleInventoryItemType::COMPRESS_MULTIPLICATION) {
			action = std::make_shared<bubbleActions::CompressCommonFactor>(intersectedShape.id);
		}
		else if (actionType == bubbleInventoryItemType::COMPRESS_EXPONENT) {
			action = std::make_shared<bubbleActions::CompressPowers>(intersectedShape.id);
		}
		else if (actionType == bubbleInventoryItemType::BUBBLIFY) {
			action = std::make_shared<bubbleActions::Bubblify>(intersectedShape.id);
		}
		else if (actionType == bubbleInventoryItemType::SIMPLIFY) {
			action = std::make_shared<bubbleActions::Simplify>(intersectedShape.id);
		}
		else if (actionType == bubbleInventoryItemType::CANCEL) {
			action = std::make_shared<bubbleActions::Cancel>(intersectedShape.id);
		}
		if (action) {
			bubble::queueBubbleAction(gameState, intersectedShape.id, action);
		}
		else {
			queueSound(gameState, bubbleSounds::ERROR_SOUND);
		}
	}

	// TODO moves these
	void updateUi(middle::GameState* gameState, int movesLeft) {
		auto stepsLeft = [gameState, movesLeft]() {
			ImGui::Begin("Moves Left");
			std::string updatedString = std::to_string(movesLeft);
			ImGui::Text(updatedString.c_str());
			ImGui::End();
			};
		middle::queueUi(gameState, stepsLeft);
	}


	void copyAsHelper(middle::GameState* gameState, middle::Id id, const midMath::Vector3& targetPos) {
		auto copyAction = std::make_shared<bubbleActions::CopyAsHelper>(id, targetPos);
		bubble::queueBubbleAction(gameState, id, copyAction);
	}

	// TODO CLEAN CODE HERE
	void update(middle::GameState* gameState) override {

		// return if procedure is executing
		if (procContainerCache->getSize() > 0) {
			auto containerIt = procContainerCache->begin<components::ProcedureContainer>();
			auto container = *containerIt;
			if (container->targetActionStackSize > 0) {
				return;
			}
		}

		if (levelConfigsCache->getSize() > 0) {
			auto configsIt = levelConfigsCache->begin<components::BubbleAlgebraLevelConfigs>();
			auto configs = *configsIt;

			if (gameState->bubbleAlgebraState.bubbleActions.size() > 0) {
				if (gameState->bubbleAlgebraState.bubbleActions.back()->cancelled) {
					++configs->allowedMoves;
					queueSound(gameState, bubbleSounds::ERROR_SOUND);
					middle::queueAction(gameState, std::make_shared<middle::CustomAction>([](middle::GameState* gameState) {
						gameState->bubbleAlgebraState.bubbleActions.back()->undo(gameState);
						gameState->bubbleAlgebraState.bubbleActions.pop_back();
						}));
					return;
				}
			}

			updateUi(gameState, configs->allowedMoves);
			if (configs->allowedMoves <= 0) {
				return;
			}

		}

		int actionCountPreFrame = gameState->bubbleAlgebraState.bubbleActions.size();


		auto& inp = gameState->gameInput;
		bool hotKeyPressed = inp.copy || inp.insertTerm || inp.pop || inp.comp || inp.mulOne || inp.proc || inp.can
			|| inp.two
			|| inp.three
			|| inp.four
			|| inp.five
			|| inp.six
			|| inp.seven
			|| inp.eight
			|| inp.nine;

		if (hotKeyPressed) {
			auto intersectingIt = intersectingCache->begin<components::IntersectingTag>();
			for (int i = 0; i < intersectingCache->getSize(); ++i) {
				auto intersecting = *intersectingIt;
				if (intersecting->intersectingTop) {
					middle::Shape& intersectingShape = middle::getShape(gameState, intersectingCache->relevantIdVector[i].index);

					// copy is not undoable bubble action for now
					if (gameState->gameInput.copy) {
						if (inventoryCache->relevantIdVector.size() == 1) {
							middle::queueAction(gameState, std::make_shared<bubbleActions::CopyToInventory>(inventoryCache->relevantIdVector[0], intersectingShape.id));
						}
					}

					// insert
					if (gameState->gameInput.insertTerm) {
						if (inventoryCache->relevantIdVector.size() == 1) {
							middle::Id invId = inventoryCache->relevantIdVector[0];
							auto inv = middle::getComp<components::Inventory>(gameState, invId);
							auto loop = middle::getComp<components::LoopSociety>(gameState, invId);
							if (loop->loopMemberIds.size() == 0 || loop->loopMemberIds[inv->activeIndex].index == middle::UNASSIGNED) {
								continue;
							}
							if (bubble::isEqualsBubble(gameState, intersectingShape.id)) {
								if(inv->insertType == components::INSERT_ADD)
									insertOperation(gameState, bubbleInventoryItemType::NEW_ADDITION_TERM, intersectingShape.id);
								if(inv->insertType == components::INSERT_MULTIPLY)
									insertOperation(gameState, bubbleInventoryItemType::NEW_MULTIPLICATION_TERM, intersectingShape.id);
								if(inv->insertType == components::INSERT_POWER)
									insertOperation(gameState, bubbleInventoryItemType::NEW_POWER_TERM, intersectingShape.id);
							}
							else {
								if (inv->invariantType == components::X_OVER_X)
									insertOperation(gameState, bubbleInventoryItemType::INSERT_X_OVER_X, intersectingShape.id);
								if (inv->invariantType == components::X_MINUS_X)
									insertOperation(gameState, bubbleInventoryItemType::INSERT_X_MINUS_X, intersectingShape.id);
							}
						}
					}


					if (gameState->gameInput.pop) {
						if (!gameState->gameInput.shiftHeld) {
							microOperation(gameState, bubbleInventoryItemType::POP, middle::Id(), intersectingShape);
						}
						else {
							microOperation(gameState, bubbleInventoryItemType::BUBBLIFY, middle::Id(), intersectingShape);
						}
					}
					if (gameState->gameInput.comp) {
						if (!gameState->gameInput.shiftHeld) {
							microOperation(gameState, bubbleInventoryItemType::COMPRESS_MULTIPLICATION, middle::Id(), intersectingShape);
						}
						else {
							microOperation(gameState, bubbleInventoryItemType::COMPRESS_EXPONENT, middle::Id(), intersectingShape);
						}
					}
					if (gameState->gameInput.mulOne) {
						if (!gameState->gameInput.shiftHeld) {
							microOperation(gameState, bubbleInventoryItemType::MUL_ONE, middle::Id(), intersectingShape);
						}
						else {
							microOperation(gameState, bubbleInventoryItemType::MUL_NEGATIVE_ONE, middle::Id(), intersectingShape);
						}
					}
					if (gameState->gameInput.can) {
						if (!gameState->gameInput.shiftHeld) {
							microOperation(gameState, bubbleInventoryItemType::CANCEL, middle::Id(), intersectingShape);
						}
						else {
							microOperation(gameState, bubbleInventoryItemType::SIMPLIFY, middle::Id(), intersectingShape);
						}
					}
					if (gameState->gameInput.proc) {
						microOperation(gameState, bubbleInventoryItemType::PROCEDURE, middle::Id(), intersectingShape);
					}
				}
			}
		}


		for (int i = 0; i < deletionCache->getSize(); ++i) {

			auto deletionIt = deletionCache->begin<components::DeleteComponent>();
			auto idRefIt = deletionCache->begin<components::IdRef>();
			auto deletion = *deletionIt;
			auto ref = *idRefIt;

			middle::Id shapeIdForDeletion;
			if (deletionCache->getSize() > 0) {
				shapeIdForDeletion = deletionCache->relevantIdVector[0];
			}


			if (shapeIdForDeletion.index == middle::UNASSIGNED) {
				return;
			}


			auto& shapeForDeletion = middle::getShape(gameState, shapeIdForDeletion.index);
			if (!isValidId(gameState, ref->idRef) && !hotKeyPressed) {
				return;
			}
			auto& deletionsRefShape = middle::getShape(gameState, ref->idRef.index);
			middle::Id refParentId = middle::getParent(gameState, deletionsRefShape.id);
			assert(deletionsRefShape.id.index != middle::UNASSIGNED);
			auto deletionsRefShapeLayer = middle::getComponent<components::Layer>(deletionsRefShape);
			bool shapeForDeletionIsInventoryItem = middle::getComponent<components::InventoryItem>(shapeForDeletion);


			int intersectCount = 0;

			auto intersectingIt = intersectingCache->begin<components::IntersectingTag>();
			for (int i = 0; i < intersectingCache->getSize(); ++i) {
				auto intersecting = *intersectingIt;

				auto& intersectableShape = middle::getShape(gameState, intersectingCache->relevantIdVector[i].index);
				middle::Id parentId = middle::getParent(gameState, intersectableShape.id);

				++intersectCount;

				// check if there's parent, the parent is not a fraction
				if (parentId.index != middle::UNASSIGNED) {
					auto parentShape = middle::getShape(gameState, parentId.index);
					auto fraction = middle::getComponent<components::FractionalComponent>(parentShape);
					if (fraction) {
						continue;
					}
				}


				// all other actions need to be in the same layer 
				auto intersectingLayer = middle::getComponent<components::Layer>(intersectableShape);
				if (intersectingLayer->layer != deletionsRefShapeLayer->layer && !shapeForDeletionIsInventoryItem) {
					continue;
				}

				// skip shapefordeletion (the copy being dragged) and ref shape (shape its copy is pointing to)
				if (shapeForDeletion.id == intersectableShape.id || intersectableShape.id == deletionsRefShape.id) {
					continue;
				}

				if (parentId.index != middle::UNASSIGNED && parentId == refParentId) {
					auto& refParent = middle::getShape(gameState, refParentId.index);
					tryCombine(gameState, refParent, deletionsRefShape, intersectableShape);
					continue;
				}
			}

			if (intersectCount == 0) {
				copyAsHelper(gameState, ref->idRef, middle::getGlobalPosition(gameState, shapeForDeletion.id));
			}
		}


		int actionCountPostFrame = gameState->bubbleAlgebraState.bubbleActions.size();
	}
};

static middle::SystemRegistrar<BubbleModificationSystem> reg("BubbleModificationSystem");
