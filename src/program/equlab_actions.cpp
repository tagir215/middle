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

namespace equlab {

	void AddBubble::execute(middle::GameState* gameState) {
		middle::Shape newBubble = bubble::newBubble(gameState, targetPosition);
		auto registerAction = std::make_unique<middle::EditorActionRegisterShape>(newBubble);
		registerAction->execute(gameState);
		resultId = registerAction->newShapeId;
		actions.push_back(std::move(registerAction));
		if (parentId.index != middle::UNASSIGNED) {
			auto reparent = std::make_unique<middle::EditorActionReparent>(parentId.index, resultId.index);
			reparent->execute(gameState);
			actions.push_back(std::move(reparent));
		}
	}
	void AddBubble::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void AddUnit::execute(middle::GameState* gameState) {
		middle::Shape newUnit = bubble::newUnit(gameState, targetPosition);
		auto registerAction = std::make_unique<middle::EditorActionRegisterShape>(newUnit);
		registerAction->execute(gameState);
		resultId = registerAction->newShapeId;
		actions.push_back(std::move(registerAction));
		if (parentId.index != middle::UNASSIGNED) {
			auto reparent = std::make_unique<middle::EditorActionReparent>(parentId.index, resultId.index);
			reparent->execute(gameState);
			actions.push_back(std::move(reparent));
		}
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
				auto unlink = std::make_unique<bubbleActions::UnlinkMultiplicationTerm>(parentId, id);
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



	void ConnectEqualsLink::execute(middle::GameState* gameState) {
		if (!canConnect(gameState, bubbleIdA, bubbleIdB)) {
			cancelled = true;
			return;
		}

		auto& shapeA = middle::getShape(gameState, bubbleIdA.index);
		auto& shapeB = middle::getShape(gameState, bubbleIdB.index);
		if (!middle::getComponent<components::BubbleComponent>(shapeA) || !middle::getComponent<components::BubbleComponent>(shapeB)) {
			cancelled = true;
			return;
		}

		Vector3 targetPos = (middle::getShapePosition(gameState, bubbleIdA.index)
			+ middle::getShapePosition(gameState, bubbleIdB.index)) * 0.5f;

		middle::Shape equalsProto = bubble::newEquals(gameState, targetPos);
		middle::Shape& equalsShape = middle::registerShape(gameState, equalsProto);

		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(equalsShape.id);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		auto reparentA = std::make_unique<middle::EditorActionReparent>(equalsShape.id.index, bubbleIdA.index);
		reparentA->execute(gameState);
		actions.push_back(std::move(reparentA));
		auto reparentB = std::make_unique<middle::EditorActionReparent>(equalsShape.id.index, bubbleIdB.index);
		reparentB->execute(gameState);
		actions.push_back(std::move(reparentB));
	}
	void ConnectEqualsLink::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
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

		// check if b is in a multiplication
		// if they are remove first from the operation and connect each child seperatedly
		// we don't have to check for a because a can work as a reciever even if its in multiplciation
		bool parentBIsMul = false;
		if (parentIdB.index != middle::UNASSIGNED) {
			auto& parentShapeB = middle::getShape(gameState, parentIdB.index);
			parentBIsMul = middle::getComponent<components::BubbleMultiplyComponent>(parentShapeB);
		}
		if (parentBIsMul) {
			auto unlink = std::make_unique<bubbleActions::UnlinkMultiplicationTerm>(parentIdB, bubbleIdB);
			unlink->execute(gameState);
			actions.push_back(std::move(unlink));
		}

		// connect idB or if they were in operation then all the op children
		auto connectAction = std::make_unique<bubbleActions::LinkMultiplicationTerm>(bubbleIdA, bubbleIdB);
		connectAction->execute(gameState);
		resultId = connectAction->resultShapeId;
		auto& resultShape = middle::getShape(gameState, resultId.index);
		actions.push_back(std::move(connectAction));
	}
	void ConnectMultiplicationLink::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void ConnectPowerLink::execute(middle::GameState* gameState) {
		if (!canConnect(gameState, bubbleIdA, bubbleIdB)) {
			cancelled = true;
			return;
		}
		Vector3 targetPos = (middle::getShapePosition(gameState, bubbleIdA.index)
			+ middle::getShapePosition(gameState, bubbleIdB.index)) * 0.5f;

		middle::Shape powerProto = bubble::newPower(gameState, targetPos);
		middle::Shape& powerShape = middle::registerShape(gameState, powerProto);

		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(powerShape.id);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		auto reparentA = std::make_unique<middle::EditorActionReparent>(powerShape.id.index, bubbleIdA.index);
		reparentA->execute(gameState);
		actions.push_back(std::move(reparentA));
		auto reparentB = std::make_unique<middle::EditorActionReparent>(powerShape.id.index, bubbleIdB.index);
		reparentB->execute(gameState);
		actions.push_back(std::move(reparentB));

		resultId = powerShape.id;
	}
	void ConnectPowerLink::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	float rf() {
		return (std::rand() % 100 + 1) * 0.01f;
	}
	Vector3 randOffset() {
		return Vector3{rf(), rf(), rf()};
	}

	middle::Id bubequToBubble(middle::GameState* gameState, const Vector3& targetPos, const std::string& path)
	{
		auto bubequ = bubequ::parseBubequ(path);
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
		bool doCloseBrackets = false;

		auto bubComp = middle::getComponent<components::BubbleComponent>(shape);
		if (bubComp) {
			result += "(";
			doCloseBrackets = true;
		}

		auto op = middle::getComponent<components::BubbleMultiplyComponent>(shape);
		if (op && op->operationType == components::OperationType::MULTIPLICATION) {
			result += "*";
		}
		else if (op && op->operationType == components::OperationType::POWER) {
			result += "^";
		}
		else if (middle::getComponent<components::BubbleEqualsComponent>(shape)) {
			result += "(=";
			doCloseBrackets = true;
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
		for (middle::Id& childId : children) {
			result += bubbleToBubequ(gameState, childId);
		}

		if (doCloseBrackets) {
			result += ")";
		}
		return result;
	}

}