#include "equlab_actions.h"
#include "editor_actions.h"
#include "bubble_actions.h"
#include "bubble_utils.h"
#include "MouseSelectable.h"
#include "BubbleEqualsComponent.h"
#include "BubbleVariable.h"
#include "component_utils.h"
#include "alg_file_utils.h"
#include <queue>
#include "UnIntersectableWindowComponent.h"
#include "TopDogBubbleTag.h"
#include "BubblePowerComponent.h"
#include "BubbleInequaltyComponent.h"

namespace equlab {

	const float freshnessTime = 0.3f;

	void AddBubble::execute(middle::GameState* gameState) {
		middle::Shape newBubbleProto = bubble::newBubble(gameState, targetPosition);
		auto registerAction = std::make_unique<middle::EditorActionRegisterShape>(newBubbleProto);
		registerAction->execute(gameState);
		resultId = registerAction->newShapeId;
		actions.push_back(std::move(registerAction));
		if (parentId.index != middle::UNASSIGNED) {
			auto reparent = std::make_unique<middle::EditorActionReparent>(parentId.index, resultId.index);
			reparent->execute(gameState);
			actions.push_back(std::move(reparent));
		}
		auto timer = middle::attachComponent<components::UnIntersectableWindowComponent>(gameState, resultId);
		timer->timeLeft = freshnessTime;
	}
	void AddBubble::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void AddUnit::execute(middle::GameState* gameState) {
		if (parentId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}

		middle::Shape newUnitProto = bubble::newUnit(gameState, targetPosition);
		auto registerAction = std::make_unique<middle::EditorActionRegisterShape>(newUnitProto);
		registerAction->execute(gameState);
		resultId = registerAction->newShapeId;
		actions.push_back(std::move(registerAction));
		if (parentId.index != middle::UNASSIGNED) {
			auto reparent = std::make_unique<middle::EditorActionReparent>(parentId.index, resultId.index);
			reparent->execute(gameState);
			actions.push_back(std::move(reparent));
		}
		auto timer = middle::attachComponent<components::UnIntersectableWindowComponent>(gameState, resultId);
		timer->timeLeft = freshnessTime;
	}

	void AddUnit::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void Negate::execute(middle::GameState* gameState) {
		if (id.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}
		middle::Id replacementShapeId = bubbleActions::createNegatedReplacementShape(gameState, id);
		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(replacementShapeId);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		auto replace = std::make_unique<bubbleActions::Replace>(id, replacementShapeId);
		replace->execute(gameState);
		actions.push_back(std::move(replace));
	}
	void Negate::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void Invert::execute(middle::GameState* gameState) {
		if (id.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}
		middle::Id replacementShapeId = bubbleActions::createInverseReplacementShape(gameState, id);
		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(replacementShapeId);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		auto replace = std::make_unique<bubbleActions::Replace>(id, replacementShapeId);
		replace->execute(gameState);
		actions.push_back(std::move(replace));
	}

	void Invert::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void AddLabelCharacterToVariable::execute(middle::GameState* gameState) {
		auto& shape = middle::getShape(gameState, id.index);

		auto comp = middle::getComponent<components::BubbleVariable>(shape);


		// if no comp create new bubble variable
		if (!comp) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, id, children);
			if (children.size() > 0) {
				cancelled = true;
				return;
			}
			else {
				middle::Id targetId = id;
				std::string targetLabel = this->label;
				auto customAction = std::make_unique<CustomActionWithUndo>(
					[targetId, targetLabel](middle::GameState* gameState) {
						auto newComp = middle::attachComponent<components::BubbleVariable>(gameState, targetId);
						newComp->label = targetLabel;
					},
					[targetId](middle::GameState* gameState) {
						middle::queueComponentDeletion<components::BubbleVariable>(gameState, targetId);
					});
				customAction->execute(gameState);
				actions.push_back(std::move(customAction));
			}
		}

		// if is already comp add character to end of current lable name
		else {
			middle::Id targetId = id;
			std::string targetLabel = this->label;
			auto customAction = std::make_unique<CustomActionWithUndo>(
				[targetId, targetLabel](middle::GameState* gameState) {
					auto& targetShape = middle::getShape(gameState, targetId.index);
					auto comp = middle::getComponent<components::BubbleVariable>(targetShape);
					comp->label += targetLabel;
				},
				[targetId, targetLabel](middle::GameState* gameState) {
					auto& targetShape = middle::getShape(gameState, targetId.index);
					auto comp = middle::getComponent<components::BubbleVariable>(targetShape);
					int targetSize = targetLabel.size();
					int labelSize = comp->label.size();
					assert(targetSize < labelSize);
					comp->label.erase(labelSize - targetSize, targetSize);
				});
			customAction->execute(gameState);
			actions.push_back(std::move(customAction));
		}
	}
	void AddLabelCharacterToVariable::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}



	void Delete::execute(middle::GameState* gameState) {
		middle::Id parentId = middle::getParent(gameState, id);

		if (parentId.index != middle::UNASSIGNED) {
			auto& parentShape = middle::getShape(gameState, parentId.index);
			auto opComp = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);
			// if op comp unlink instead of delete
			if (opComp) {
				auto unlink = std::make_unique<bubbleActions::UnlinkMultiplicationTerm>(id);
				unlink->execute(gameState);
				actions.push_back(std::move(unlink));
				return;
			}
		}

		auto del = std::make_unique < middle::EditorActionDeleteSingle>(id);
		del->execute(gameState);
		actions.push_back(std::move(del));
	}

	void Delete::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	bool canConnect(middle::GameState* gameState, middle::Id idA, middle::Id idB) {
		auto& shapeA = middle::getShape(gameState, idA.index);
		auto& shapeB = middle::getShape(gameState, idB.index);
		if (!middle::getComponent<components::BubbleComponent>(shapeA) || !middle::getComponent<components::BubbleComponent>(shapeB)) {
			return false;
		}
		middle::Id parentAId = middle::getParent(gameState, idA);
		middle::Id parentBId = middle::getParent(gameState, idB);
		return parentAId == parentBId;
	}



	void AddEquals::execute(middle::GameState* gameState) {
		middle::Shape bubAProto = bubble::newBubble(gameState, targetPos + Vector3{-1,0,0});
		middle::Shape bubBProto = bubble::newBubble(gameState, targetPos + Vector3{1,0,0});
		middle::Shape equalsProto = bubble::newEquals(gameState, targetPos);
		middle::Shape& bubA = middle::registerShape(gameState, bubAProto);
		middle::Shape& bubB = middle::registerShape(gameState, bubBProto);
		middle::Shape& equals = middle::registerShape(gameState, equalsProto);

		middle::EditorActionReparent(equals.id.index, bubA.id.index).execute(gameState);
		middle::EditorActionReparent(equals.id.index, bubB.id.index).execute(gameState);

		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(equals.id);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));
	}

	void AddEquals::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void AddInequals::execute(middle::GameState* gameState) {
		middle::Shape bubAProto = bubble::newBubble(gameState, targetPos + Vector3{-1,0,0});
		middle::Shape bubBProto = bubble::newBubble(gameState, targetPos + Vector3{1,0,0});
		middle::Shape inequalsProto = bubble::newInequals(gameState, targetPos, equalOr);
		middle::Shape& bubA = middle::registerShape(gameState, bubAProto);
		middle::Shape& bubB = middle::registerShape(gameState, bubBProto);
		middle::Shape& inequalsShape = middle::registerShape(gameState, inequalsProto);

		middle::EditorActionReparent(inequalsShape.id.index, bubA.id.index).execute(gameState);
		middle::EditorActionReparent(inequalsShape.id.index, bubB.id.index).execute(gameState);

		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(inequalsShape.id);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));
	}

	void AddInequals::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}



	void ToggleEditable::execute(middle::GameState* gameState) {
	}

	void ToggleEditable::undo(middle::GameState* gameState) {
	}

	void ConnectMultiplicationLink::execute(middle::GameState* gameState) {

		auto& shapeA = middle::getShape(gameState, bubbleIdA.index);
		auto& shapeB = middle::getShape(gameState, bubbleIdB.index);
		if (!middle::getComponent<components::BubbleComponent>(shapeA) || !middle::getComponent<components::BubbleComponent>(shapeB)) {
			cancelled = true;
			return;
		}
		middle::Id parentIdA = middle::getParent(gameState, bubbleIdA);
		middle::Id parentIdB = middle::getParent(gameState, bubbleIdB);
		if (parentIdA != parentIdB) {
			cancelled = true;
			return;
		}

		// create replacement
		int oldIndexA = middle::getLoopIndex(gameState, bubbleIdA);
		int oldIndexB = middle::getLoopIndex(gameState, bubbleIdB);
		middle::Id mulCopyId = middle::deepCopyShapeGlobalCoordinates(gameState, parentIdA);
		std::vector<middle::Id>children;
		middle::getChildren(gameState, mulCopyId, children);
		middle::Id copyIdA = children[oldIndexA];
		middle::Id copyIdB = children[oldIndexB];
		int childrenSize = children.size();
		middle::EditorActionRemoveFromLoop(copyIdA.index).execute(gameState);
		middle::EditorActionRemoveFromLoop(copyIdB.index).execute(gameState);
		if (childrenSize < 4) {
			middle::queueComponentDeletion<components::BubbleMultiplyComponent>(gameState, mulCopyId);
		}
		auto connectAction = bubbleActions::LinkMultiplicationTerm(copyIdA, copyIdB);
		connectAction.execute(gameState);
		middle::EditorActionReparent(mulCopyId.index, connectAction.resultShapeId.index).execute(gameState);
		resultId = mulCopyId;
		middle::executeAction<middle::EditorActionRegisterId>(gameState, this, mulCopyId);

		// replace
		middle::executeAction<bubbleActions::Replace>(gameState, this, parentIdA, mulCopyId);
	}
	void ConnectMultiplicationLink::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void ConnectPower::execute(middle::GameState* gameState) {
		middle::Id oldParentId = middle::getParent(gameState, baseId);

		Vector3 targetPos = (middle::getGlobalPosition(gameState, baseId.index)
			+ middle::getGlobalPosition(gameState, exponentId.index)) * 0.5f;

		middle::Shape newPowerProto = bubble::newPower(gameState, targetPos);
		middle::Shape& newPower = middle::registerShape(gameState, newPowerProto);
		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(newPower.id);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		auto reparentA = std::make_unique<middle::EditorActionReparent>(newPower.id.index, baseId.index);
		reparentA->execute(gameState);
		actions.push_back(std::move(reparentA));
		auto reparentB = std::make_unique<middle::EditorActionReparent>(newPower.id.index, exponentId.index);
		reparentB->execute(gameState);
		actions.push_back(std::move(reparentB));

		auto reparentC = std::make_unique<middle::EditorActionReparent>(oldParentId.index, newPower.id.index);
		reparentC->execute(gameState);
		actions.push_back(std::move(reparentC));

		resultId = newPower.id;
	}

	void ConnectPower::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	float rf() {
		return (std::rand() % 100 + 1) * 0.01f;
	}
	Vector3 randOffset() {
		return Vector3{ rf(), rf(), rf() };
	}

	middle::Id bubequToBubble(middle::GameState* gameState, const Vector3& targetPos, std::shared_ptr<bubequ::Scope>& bubequ)
	{
		std::queue<bubequ::Scope*>scopeQueue;
		std::queue<middle::Id>parentQueue;
		scopeQueue.push(bubequ.get());
		parentQueue.push(middle::Id());

		middle::Id rootId;

		while (scopeQueue.size() > 0) {
			bubequ::Scope* currentScope = scopeQueue.front();
			middle::Id currentParentId = parentQueue.front();
			scopeQueue.pop();
			parentQueue.pop();
			Vector3 randOff = randOffset();
			Vector3 pos = targetPos + randOff;

			middle::Id newNodeId;

			if (auto unitScope = dynamic_cast<bubequ::Unit*>(currentScope)) {
				if (unitScope->type == bubequ::UnitType::ZERO) {
					middle::Shape bubProto = bubble::newBubble(gameState, pos);
					newNodeId = middle::registerShape(gameState, bubProto).id;
				}
				if (unitScope->type == bubequ::UnitType::CONSTANT) {
					newNodeId = bubble::newBubbleWithIntValue(gameState, unitScope->value, pos);
				}
				else if (unitScope->type == bubequ::UnitType::VARIABLE) {
					int s = std::abs(unitScope->value);
					bool isNegative = unitScope->value < 0;
					if (s == 1) {
						middle::Shape varProto = bubble::newVariable(gameState, unitScope->label, pos, isNegative);
						middle::Shape& varShape = middle::registerShape(gameState, varProto);
						newNodeId = varShape.id;
					}
					else {
						middle::Shape bubbleProto = bubble::newBubble(gameState, pos);
						middle::Shape& bubbleShape = middle::registerShape(gameState, bubbleProto);
						for (int i = 0; i < s; ++i) {
							middle::Shape varProto = bubble::newVariable(gameState, unitScope->label, pos, isNegative);
							middle::Shape& varShape = middle::registerShape(gameState, varProto);
							middle::EditorActionReparent(bubbleShape.id.index, varShape.id.index).execute(gameState);
						}
						newNodeId = bubbleShape.id;
					}

				}
			}
			else if (auto linkScope = dynamic_cast<bubequ::Link*>(currentScope)) {
				if (linkScope->type == bubequ::LinkType::MULTIPLICATION) {
					middle::Shape linkProto = bubble::newMultiplication(gameState, pos);
					middle::Shape& linkShape = middle::registerShape(gameState, linkProto);
					newNodeId = linkShape.id;
				}
				else if (linkScope->type == bubequ::LinkType::GREATER) {
					middle::Shape linkProto = bubble::newInequals(gameState, pos, false);
					middle::Shape& linkShape = middle::registerShape(gameState, linkProto);
					newNodeId = linkShape.id;
				}
				else if (linkScope->type == bubequ::LinkType::GREATER_OR_EQUAL) {
					middle::Shape linkProto = bubble::newInequals(gameState, pos, true);
					middle::Shape& linkShape = middle::registerShape(gameState, linkProto);
					newNodeId = linkShape.id;
				}
				else if (linkScope->type == bubequ::LinkType::EQUALS) {
					middle::Shape linkProto = bubble::newEquals(gameState, pos);
					middle::Shape& linkShape = middle::registerShape(gameState, linkProto);
					newNodeId = linkShape.id;
				}
				else if (linkScope->type == bubequ::LinkType::POWER) {
					middle::Shape linkProto = bubble::newPower(gameState, pos);
					middle::Shape& linkShape = middle::registerShape(gameState, linkProto);
					newNodeId = linkShape.id;
				}
			}
			else {
				auto addBub = equlab::AddBubble(currentParentId, pos);
				addBub.execute(gameState);
				middle::Id bubbleId = addBub.resultId;
				newNodeId = bubbleId;
			}

			if (currentParentId.index != middle::UNASSIGNED) {
				middle::EditorActionReparent(currentParentId.index, newNodeId.index).execute(gameState);
			}
			if (rootId.index == middle::UNASSIGNED) {
				rootId = newNodeId;
			}
			for (auto& scope : currentScope->children) {
				scopeQueue.push(scope.get());
				parentQueue.push(newNodeId);
			}
		}

		return rootId;
	}


	std::string bubbleToBubequ(middle::GameState* gameState, middle::Id id)
	{
		auto& shape = middle::getShape(gameState, id.index);

		std::string result;

		result += "(";

		if (middle::getComponent<components::BubbleMultiplyComponent>(shape)) {
			result += "*";
		}
		else if (middle::getComponent<components::BubblePowerComponent>(shape)) {
			result += "^";
		}


		if (middle::getComponent<components::BubbleInequaltyComponent>(shape)) {
			result += ">";
		}
		if (middle::getComponent<components::BubbleEqualsComponent>(shape)) {
			result += "=";
		}

		if (auto unit = middle::getComponent<components::BubbleUnit>(shape)) {
			result += std::to_string(unit->value);
		}
		else if (auto var = middle::getComponent<components::BubbleVariable>(shape)) {
			if (var->isNegative) {
				result += "-";
			}
			result += var->label;
		}

		
		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);
		// if power bubble make sure to write base first, otherwise just loop children
		if (bubble::isPowerBubble(gameState, shape.id)) {
			middle::Id baseId, exponentId;
			bubble::getPowerBaseAndExponent(gameState, shape.id, baseId, exponentId);
			result += bubbleToBubequ(gameState, baseId);
			result += bubbleToBubequ(gameState, exponentId);
		}
		else {
			for (middle::Id& childId : children) {
				result += bubbleToBubequ(gameState, childId);
			}
		}

		result += ")";
		return result;
	}

}