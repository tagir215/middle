#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "DeleteComponent.h"
#include "MouseIntersectable.h"
#include "IdRef.h"
#include "BubbleMultiplyComponent.h"
#include "bubble_actions.h"
#include "InventoryItem.h"
#include "component_utils.h"
#include "PlacementComponent.h"
#include "BubbleVariable.h"
#include "TopDogBubbleTag.h"
#include "bubble_utils.h"
#include "ExponentComponent.h"
#include "UiComponent.h"
#include "BubbleAlgebraLevelConfigs.h"
#include "RuntimeHiddenTag.h"

class BubbleModificationSystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* deletionCache;
	components::CompCache* intersectableCache;
	components::CompCache* placementCache;
	components::CompCache* levelConfigsCache;
	components::CompCache* uiCompCache;

	void init(middle::GameState* gameState) {
		deletionCache = middle::newCompCache(gameState);
		deletionCache->addType<components::DeleteComponent>();
		deletionCache->addType<components::IdRef>();

		intersectableCache = middle::newCompCache(gameState);
		intersectableCache->addType<components::MouseIntersectable>();

		placementCache = middle::newCompCache(gameState);
		placementCache->addType<components::BubbleComponent>();
		placementCache->addType<components::PlacementComponent>();
		placementCache->addType<components::IdRef>();

		levelConfigsCache = middle::newCompCache(gameState);
		levelConfigsCache->addType<components::BubbleAlgebraLevelConfigs>();

		uiCompCache = middle::newCompCache(gameState);
		uiCompCache->addType<components::UiComponent>();
	}

	bool isMultiplicationConnection(middle::GameState* gameState, middle::Shape& parentShape) {
		auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);
		if (mulComp) {
			return true;
		}
		return false;
	}

	void insert(middle::GameState* gameState, middle::Shape& variableShape, middle::Shape& shapeToInsert) {
		auto copy = std::make_shared <middle::EditorActionCopySingle>(shapeToInsert.id);
		auto replace = std::make_shared<bubbleActions::Replace>(variableShape.id, middle::Id());
		auto insert = std::make_shared<middle::CustomActionWithUndo>(
			[copy, replace](middle::GameState* gameState) {
				// copy shape to insert
				copy->execute(gameState);
				middle::Id copyId = copy->resultId;
				// negate if variable where inserting to is negative
				middle::Id variableId = replace->shapeToReplaceId;
				auto& varShape = middle::getShape(gameState, variableId.index);
				auto varUnit = middle::getComponent<components::BubbleUnit>(varShape);
				if (varUnit->value == -1) {
					bubble::negate(gameState, copyId);
				}
				// replace, update variable first
				replace->replacingShapeId = copyId;
				replace->execute(gameState);
			},
			[copy, replace](middle::GameState* gameState) {
				replace->undo(gameState);
				copy->undo(gameState);
			});
		middle::queueAction(gameState, insert);
		gameState->bubbleAlgebraState.bubbleActions.push_back(insert);
	}

	void tryPower(middle::GameState* gameState, middle::Shape& shape) {
		auto rootComp = middle::getComponent<components::ExponentComponent>(shape);
		if(!rootComp){
			return;
		}

		auto power = std::make_shared<bubbleActions::ExecutePower>(shape.id);
		middle::queueAction(gameState, power);
		gameState->bubbleAlgebraState.bubbleActions.push_back(power);
	}

	void tryCombine(middle::GameState* gameState, middle::Shape& refParent, middle::Shape& refShape, middle::Shape& intersectedShape) {

		// is multiplication connection
		if (isMultiplicationConnection(gameState, refParent)) {
			auto multiply = std::make_shared<bubbleActions::ExecuteMultiplication>(refShape.id, intersectedShape.id);
			middle::queueAction(gameState, multiply);
			gameState->bubbleAlgebraState.bubbleActions.push_back(multiply);
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

	middle::Id copyOfInventoryItem(middle::GameState* gameState, middle::Id& inventoryItemId) {
		middle::Id& copyId = middle::deepCopyShape(gameState, inventoryItemId.index);
		middle::queueComponentDeletion<components::InventoryItem>(gameState, copyId);
		middle::queueComponentDeletion<components::IdRef>(gameState, copyId);
		middle::queueComponentDeletion<components::UiComponent>(gameState, copyId);
		std::vector<middle::Id>children;
		middle::getAllChildren(gameState, copyId, children);
		for (middle::Id& child : children) {
			middle::queueComponentDeletion<components::UiComponent>(gameState, child);
		}
		return copyId;
	}


	void inventoryAction(middle::GameState* gameState, int actionType, middle::Id& refId, middle::Shape& intersectedShape) {
		std::shared_ptr<middle::EditorActionContainer>action;

		if (actionType == bubbleInventoryItemType::NEW_ADDITION_TERM) {
			middle::Id copyId = copyOfInventoryItem(gameState, refId);
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
		else if (actionType == bubbleInventoryItemType::NEW_MULTIPLICATION_TERM) {
			middle::Id copyId = copyOfInventoryItem(gameState, refId);
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
		// pop as long as not multiplication
		else if (actionType == bubbleInventoryItemType::POP) {
			middle::Id& parentId = middle::getParent(gameState, intersectedShape.id);
			if (parentId.index == middle::UNASSIGNED) {
				return;
			}
			auto& parentShape = middle::getShape(gameState, parentId.index);
			if (!isMultiplicationConnection(gameState, parentShape)) {
				action = std::make_shared<bubbleActions::Pop>(intersectedShape.id);
			}
		}
		else if (actionType == bubbleInventoryItemType::TIMES_ONE) {
			action = std::make_shared<bubbleActions::MulOne>(intersectedShape.id);
		}
		else if (actionType == bubbleInventoryItemType::COMPRESS) {
			action = std::make_shared<bubbleActions::Compress>(intersectedShape.id);
		}
		else if (actionType == bubbleInventoryItemType::BREAK_2) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 2);
		}
		else if (actionType == bubbleInventoryItemType::BREAK_3) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 3);
		}
		else if (actionType == bubbleInventoryItemType::BREAK_4) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 4);
		}
		else if (actionType == bubbleInventoryItemType::BREAK_5) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 5);
		}
		else if (actionType == bubbleInventoryItemType::BREAK_6) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 6);
		}
		else if (actionType == bubbleInventoryItemType::BREAK_7) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 7);
		}
		else if (actionType == bubbleInventoryItemType::BREAK_8) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 8);
		}
		else if (actionType == bubbleInventoryItemType::BREAK_9) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 9);
		}
		else if (actionType == bubbleInventoryItemType::BREAK_10) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 10);
		}
		else if (actionType == bubbleInventoryItemType::BUBBLIFY) {
			action = std::make_shared<bubbleActions::Bubblify>(intersectedShape.id);
		}
		else if (actionType == bubbleInventoryItemType::SIMPLIFY) {
			action = std::make_shared<bubbleActions::Simplify>(intersectedShape.id);
		}
		if (action) {
			middle::queueAction(gameState, action);
			gameState->bubbleAlgebraState.bubbleActions.push_back(action);
		}
	}

	// TODO moves these
	void updateUi(middle::GameState* gameState, int movesLeft) {
			auto uiCompIt = uiCompCache->begin<components::UiComponent>();
			for (int i = 0; i < uiCompCache->getSize(); ++i) {
				auto uiComp = *uiCompIt;
				auto& shape = middle::getShape(gameState, uiCompCache->relevantIdVector[i].index);
				if (uiComp->type == UiElementTypes::OUT_OF_STEPS) {
					bool isHidden = middle::getComponent<components::RuntimeHiddenTag>(shape) != nullptr;
					if (movesLeft <= 0 && isHidden) {
						middle::queueComponentDeletion<components::RuntimeHiddenTag>(gameState, shape.id);
					}

					if (movesLeft > 0 && !isHidden){
						middle::queueComponentAttachment<components::RuntimeHiddenTag>(gameState, shape.id);
					}
				}
				if (uiComp->type == UiElementTypes::STEPS_LEFT_TEXT) {
					auto text = middle::getComponent<components::Text>(shape);
					std::string updatedString = std::to_string(movesLeft);
					text->text = updatedString;
				}
			}
	}

	void update(middle::GameState* gameState) override {


		if (levelConfigsCache->getSize() > 0) {
			auto configsIt = levelConfigsCache->begin<components::BubbleAlgebraLevelConfigs>();
			auto configs = *configsIt;

			if (gameState->bubbleAlgebraState.bubbleActions.size() > 0) {
				if (gameState->bubbleAlgebraState.bubbleActions.back()->cancelled) {
					++configs->allowedMoves;
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
		bool hotKeyPressed = inp.pop || inp.comp || inp.mulOne || inp.sim || inp.bub
			|| inp.two
			|| inp.three
			|| inp.four
			|| inp.five
			|| inp.six
			|| inp.seven
			|| inp.eight
			|| inp.nine;

		if (hotKeyPressed) {
			auto intersectingIt = intersectableCache->begin<components::MouseIntersectable>();
			for (int i = 0; i < intersectableCache->getSize(); ++i) {
				auto intersectable = *intersectingIt;
				if (intersectable->intersectingTop) {
					middle::Shape& intersectingShape = middle::getShape(gameState, intersectableCache->relevantIdVector[i].index);
					if (gameState->gameInput.pop) {
						inventoryAction(gameState, bubbleInventoryItemType::POP, middle::Id(), intersectingShape);
					}
					if (gameState->gameInput.comp) {
						inventoryAction(gameState, bubbleInventoryItemType::COMPRESS, middle::Id(), intersectingShape);
					}
					if (gameState->gameInput.mulOne) {
						inventoryAction(gameState, bubbleInventoryItemType::TIMES_ONE, middle::Id(), intersectingShape);
					}
					if (gameState->gameInput.sim) {
						inventoryAction(gameState, bubbleInventoryItemType::SIMPLIFY, middle::Id(), intersectingShape);
					}
					if (gameState->gameInput.bub) {
						inventoryAction(gameState, bubbleInventoryItemType::BUBBLIFY, middle::Id(), intersectingShape);
					}

					if (gameState->gameInput.two) {
						inventoryAction(gameState, bubbleInventoryItemType::BREAK_2, middle::Id(), intersectingShape);
					}
					if (gameState->gameInput.three) {
						inventoryAction(gameState, bubbleInventoryItemType::BREAK_3, middle::Id(), intersectingShape);
					}
					if (gameState->gameInput.four) {
						inventoryAction(gameState, bubbleInventoryItemType::BREAK_4, middle::Id(), intersectingShape);
					}
					if (gameState->gameInput.five) {
						inventoryAction(gameState, bubbleInventoryItemType::BREAK_5, middle::Id(), intersectingShape);
					}
					if (gameState->gameInput.six) {
						inventoryAction(gameState, bubbleInventoryItemType::BREAK_6, middle::Id(), intersectingShape);
					}
					if (gameState->gameInput.seven) {
						inventoryAction(gameState, bubbleInventoryItemType::BREAK_7, middle::Id(), intersectingShape);
					}
					if (gameState->gameInput.eight) {
						inventoryAction(gameState, bubbleInventoryItemType::BREAK_8, middle::Id(), intersectingShape);
					}
					if (gameState->gameInput.nine) {
						inventoryAction(gameState, bubbleInventoryItemType::BREAK_9, middle::Id(), intersectingShape);
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
			if (!middle::isShapeAlive(gameState, ref->idRef.index) && !hotKeyPressed) {
				return;
			}
			auto& refShape = middle::getShape(gameState, ref->idRef.index);
			middle::Id refParentId = middle::getParent(gameState, refShape.id);
			assert(refShape.id.index != middle::UNASSIGNED);


			auto intersectableIt = intersectableCache->begin<components::MouseIntersectable>();
			for (int i = 0; i < intersectableCache->getSize(); ++i) {
				auto intersectable = *intersectableIt;

				auto& intersectableShape = middle::getShape(gameState, intersectableCache->relevantIdVector[i].index);
				middle::Id parentId = middle::getParent(gameState, intersectableShape.id);

				// check if there's parent, the parent is not a fraction
				if (parentId.index != middle::UNASSIGNED) {
					auto parentShape = middle::getShape(gameState, parentId.index);
					auto fraction = middle::getComponent<components::FractionalComponent>(parentShape);
					if (fraction) {
						continue;
					}
				}

				if (!bubble::isIntersecting(gameState, intersectableShape)) {
					continue;
				}

				// if referencing itself, it can only be power
				if (refShape.id == intersectableShape.id) {
					tryPower(gameState, refShape);
					continue;
				}

				// skip shapefordeletion (the copy being dragged) and ref shape (shape its copy is pointing to)
				if (shapeForDeletion.id == intersectableShape.id || intersectableShape.id == refShape.id) {
					continue;
				}

				if (parentId.index != middle::UNASSIGNED && parentId == refParentId) {
					auto& refParent = middle::getShape(gameState, refParentId.index);
					tryCombine(gameState, refParent, refShape, intersectableShape);
					continue;
				}

				auto variableComp = middle::getComponent<components::BubbleVariable>(intersectableShape);
				auto topDogComp = middle::getComponent<components::TopDogBubbleTag>(refShape);
				if (variableComp && topDogComp) {
					insert(gameState, intersectableShape, refShape);
				}

				auto inventoryItem = middle::getComponent<components::InventoryItem>(refShape);
				if (inventoryItem) {
					inventoryAction(gameState, inventoryItem->itemType, ref->idRef, intersectableShape);
					break;
				}
			}
		}


		int actionCountPostFrame = gameState->bubbleAlgebraState.bubbleActions.size();

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
	};

	static middle::SystemRegistrar<BubbleModificationSystem> reg("BubbleModificationSystem");
