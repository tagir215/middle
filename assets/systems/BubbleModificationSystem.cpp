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

class BubbleModificationSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* deletionCache;
	components::CompCache* intersectingCache;
	components::CompCache* placementCache;
	components::CompCache* levelConfigsCache;
	components::CompCache* uiCompCache;
	components::CompCache* procContainerCache;

	void init(middle::GameState* gameState) {
		deletionCache = middle::newCompCache(gameState, systemName);
		deletionCache->addType<components::DeleteComponent>();
		deletionCache->addType<components::IdRef>();

		intersectingCache = middle::newCompCache(gameState, systemName);
		intersectingCache->addType<components::IntersectingTag>();
		intersectingCache->addType<components::DeleteComponent>(components::NOTINTERESTED);

		placementCache = middle::newCompCache(gameState, systemName);
		placementCache->addType<components::BubbleComponent>();
		placementCache->addType<components::PlacementComponent>();
		placementCache->addType<components::IdRef>();

		levelConfigsCache = middle::newCompCache(gameState, systemName);
		levelConfigsCache->addType<components::BubbleAlgebraLevelConfigs>();

		uiCompCache = middle::newCompCache(gameState, systemName);
		uiCompCache->addType<components::UiComponent>();

		procContainerCache = middle::newCompCache(gameState, systemName);
		procContainerCache->addType<components::ProcedureContainer>();
	}

	bool isMultiplicationConnection(middle::GameState* gameState, middle::Shape& parentShape) {
		auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);
		if (mulComp) {
			return true;
		}
		return false;
	}

	void insert(middle::GameState* gameState, middle::Shape& intersectedShape, middle::Shape& deletionRefShape, middle::Shape& shapeForDeletion) {
		// check that grabbed bubble equals a variable and that variable where is being inserted has the same label
		auto bubbleEqualsVar = middle::getComponent<components::BubbleEqualsVariable>(shapeForDeletion);
		auto varComp = middle::getComponent<components::BubbleVariable>(intersectedShape);

		// Insert bubble to variable
		if (bubbleEqualsVar && bubbleEqualsVar->wantsToReplaceVariable && varComp && bubbleEqualsVar->variableLabel == varComp->label) {
			auto insertAction = std::make_shared<bubbleActions::Insert>(intersectedShape.id, deletionRefShape.id);
			middle::queueAction(gameState, insertAction);
			gameState->bubbleAlgebraState.bubbleActions.push_back(insertAction);
		}

		// Replace bubble with variable
		else if (bubbleEqualsVar && bubbleEqualsVar->wantsToReplaceBubble && bubble::matchingBubbles(gameState, bubbleEqualsVar->matchingIdRef, intersectedShape.id)) {
			auto insertAction = std::make_shared<bubbleActions::Insert>(intersectedShape.id, deletionRefShape.id);
			middle::queueAction(gameState, insertAction);
			gameState->bubbleAlgebraState.bubbleActions.push_back(insertAction);
		}
	}

	void tryCombine(middle::GameState* gameState, middle::Shape& refParent, middle::Shape& refShape, middle::Shape& intersectedShape) {

		// is multiplication connection
		if (middle::getComponent<components::BubbleMultiplyComponent>(refParent)) {
			auto multiply = std::make_shared<bubbleActions::ExecuteMultiplication>(refShape.id, intersectedShape.id);
			middle::queueAction(gameState, multiply);
			gameState->bubbleAlgebraState.bubbleActions.push_back(multiply);
			return;
		}
		else if (middle::getComponent<components::BubblePowerComponent>(refParent)) {
			auto doPower = std::make_shared<bubbleActions::ExecutePower>(refShape.id, intersectedShape.id);
			middle::queueAction(gameState, doPower);
			gameState->bubbleAlgebraState.bubbleActions.push_back(doPower);
			return;
		}
		// else is addition connection
		else {
			auto add = std::make_shared<bubbleActions::ExecuteAddition>(refShape.id, intersectedShape.id);
			middle::queueAction(gameState, add);
			gameState->bubbleAlgebraState.bubbleActions.push_back(add);
			return;
		}
	}

	middle::Id copyOfInsertItem(middle::GameState* gameState, middle::Id& inventoryItemId) {
		middle::Id& copyId = middle::deepCopyShape(gameState, inventoryItemId.index);
		middle::queueComponentDeletion<components::InsertableBubble>(gameState, copyId);
		return copyId;
	}

	void insertOperation(middle::GameState* gameState, int actionType, middle::Id& refId, middle::Shape& intersectedShape) {
		std::shared_ptr<middle::EditorActionContainer>action;

		if (actionType == BubbleInsertType::ADD_OUTER) {
			middle::Id copyId = copyOfInsertItem(gameState, refId);
			auto registerAction = std::make_shared<middle::EditorActionRegisterId>(copyId);
			auto newTermAction = std::make_shared<bubbleActions::NewAdditionTerm>(intersectedShape.id, copyId, gameState->input.mouseXZ_PlanePos);
			action = std::make_shared < middle::CustomActionWithUndo>(
				[registerAction, newTermAction](middle::GameState* gameState) {
					registerAction->execute(gameState);
					newTermAction->execute(gameState);
				},
				[registerAction, newTermAction](middle::GameState* gameState) {
					newTermAction->undo(gameState);
					registerAction->undo(gameState);
				});
		}
		else if (actionType == BubbleInsertType::MULTIPLY_OUTER) {
			middle::Id copyId = copyOfInsertItem(gameState, refId);
			auto registerAction = std::make_shared<middle::EditorActionRegisterId>(copyId);
			auto newTermAction = std::make_shared<bubbleActions::NewMultiplicationTerm>(intersectedShape.id, copyId, gameState->input.mouseXZ_PlanePos);
			action = std::make_shared < middle::CustomActionWithUndo>(
				[registerAction, newTermAction](middle::GameState* gameState) {
					registerAction->execute(gameState);
					newTermAction->execute(gameState);
				},
				[registerAction, newTermAction](middle::GameState* gameState) {
					newTermAction->undo(gameState);
					registerAction->undo(gameState);
				});
		}
		else if (actionType == BubbleInsertType::POWER_OUTER) {
			middle::Id copyId = copyOfInsertItem(gameState, refId);
			auto registerAction = std::make_shared<middle::EditorActionRegisterId>(copyId);
			auto newTermAction = std::make_shared<bubbleActions::NewPowerTerm>(intersectedShape.id, copyId, gameState->input.mouseXZ_PlanePos);
			action = std::make_shared < middle::CustomActionWithUndo>(
				[registerAction, newTermAction](middle::GameState* gameState) {
					registerAction->execute(gameState);
					newTermAction->execute(gameState);
				},
				[registerAction, newTermAction](middle::GameState* gameState) {
					newTermAction->undo(gameState);
					registerAction->undo(gameState);
				});
		}
		else if (actionType == BubbleInsertType::ADD_X_MINUS_X) {
			middle::Id copyId = copyOfInsertItem(gameState, refId);
			auto registerAction = std::make_shared<middle::EditorActionRegisterId>(copyId);
			auto insertAction = std::make_shared<bubbleActions::InsertAsXMinusX>(intersectedShape.id, copyId, gameState->input.mouseXZ_PlanePos);
			action = std::make_shared < middle::CustomActionWithUndo>(
				[registerAction, insertAction](middle::GameState* gameState) {
					registerAction->execute(gameState);
					insertAction->execute(gameState);
				},
				[registerAction, insertAction](middle::GameState* gameState) {
					insertAction->undo(gameState);
					registerAction->undo(gameState);
				});
		}
		else if (actionType == BubbleInsertType::MULTIPLY_X_OVER_X) {
			middle::Id copyId = copyOfInsertItem(gameState, refId);
			auto registerAction = std::make_shared<middle::EditorActionRegisterId>(copyId);
			auto insertAction = std::make_shared<bubbleActions::InsertAsXOverX>(intersectedShape.id, copyId, gameState->input.mouseXZ_PlanePos);
			action = std::make_shared < middle::CustomActionWithUndo>(
				[registerAction, insertAction](middle::GameState* gameState) {
					registerAction->execute(gameState);
					insertAction->execute(gameState);
				},
				[registerAction, insertAction](middle::GameState* gameState) {
					insertAction->undo(gameState);
					registerAction->undo(gameState);
				});
		}

		if (action) {
			middle::queueAction(gameState, action);
			gameState->bubbleAlgebraState.bubbleActions.push_back(action);
		}
		else {
			queueSound(gameState, bubbleSounds::ERROR_SOUND);
		}
	}

	void microOperation(middle::GameState* gameState, int actionType, middle::Id& refId, middle::Shape& intersectedShape) {
		std::shared_ptr<middle::EditorActionContainer>action;

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
			if (procContainerCache->getSize() == 1) {
				middle::Id procContainerId = procContainerCache->relevantIdVector[0];
				action = std::make_shared<bubbleActions::StartProcedure>(procContainerId, intersectedShape.id);
			}
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
			middle::queueAction(gameState, action);
			gameState->bubbleAlgebraState.bubbleActions.push_back(action);
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
		gameState->uiSetups.push_back(stepsLeft);
	}


	void copyAsHelper(middle::GameState* gameState, middle::Id id, const Vector3& targetPos) {
		auto copyAction = std::make_shared<bubbleActions::CopyAsHelper>(id, targetPos);
		middle::queueAction(gameState, copyAction);
		gameState->bubbleAlgebraState.bubbleActions.push_back(copyAction);
	}

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
		bool hotKeyPressed = inp.pop || inp.comp || inp.mulOne || inp.proc || inp.can
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

			auto intersectableIt = intersectingCache->begin<components::IntersectingTag>();
			for (int i = 0; i < intersectingCache->getSize(); ++i) {
				auto intersectable = *intersectableIt;

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

				if (intersectable->intersectingTop) {
					//auto inventoryItem = middle::getComponent<components::InventoryItem>(deletionsRefShape);
					//if (inventoryItem) {
					//	inventoryAction(gameState, inventoryItem->itemType, ref->idRef, intersectableShape);
					//	break;
					//}
					auto insertable = middle::getComponent<components::InsertableBubble>(deletionsRefShape);
					if (insertable) {
						insertOperation(gameState, gameState->bubbleAlgebraState.currentInsertType, ref->idRef, intersectableShape);
						break;
					}
				}


				// variables can be non same layer, so check before layer filter, but it needs to intersect at top
				if (intersectable->intersectingTop) {
					auto topDogComp = middle::getComponent<components::TopDogBubbleTag>(deletionsRefShape);
					if (topDogComp) {
						insert(gameState, intersectableShape, deletionsRefShape, shapeForDeletion);
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
				copyAsHelper(gameState, ref->idRef, middle::getGlobalPosition(gameState, shapeForDeletion.id.index));
			}
		}


		int actionCountPostFrame = gameState->bubbleAlgebraState.bubbleActions.size();

		if (levelConfigsCache->getSize() > 0) {
			if (actionCountPostFrame > actionCountPreFrame) {
				auto configsIt = levelConfigsCache->begin<components::BubbleAlgebraLevelConfigs>();
				auto configs = *configsIt;
				middle::queueAction(gameState, std::make_unique<middle::CustomAction>(
					[configs](middle::GameState* gameState)
					{
						--configs->allowedMoves;
						if (configs->allowedMoves < 0) {
							configs->allowedMoves = 0;
						}
					}));
			}
		}
	}
};

static middle::SystemRegistrar<BubbleModificationSystem> reg("BubbleModificationSystem");
