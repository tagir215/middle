#pragma once
#include "game_state.h"
#include "alg_file_utils.h"
#include "middle_shape_utils.h"
#include "bubble_utils.h"
#include "editor_actions.h"
#include "bubble_actions.h"
#include "equlab_actions.h"
#include "component_utils.h"
#include "LocalScale.h"
#include "TopDogBubbleTag.h"
#include "BubblePowerComponent.h"
#include "BubbleInequaltyComponent.h"
#include "BubbleFunctionComponent.h"
#include "BubbleSummationComponent.h"
#include "BubbleEqualsComponent.h"
#include "BubbleVariable.h"
#include "sha256.h"
#include "bubequ.h"

namespace bubequ{

	inline middle::Id bubequToBubble(middle::GameState* gameState, const Vector3& targetPos, std::shared_ptr<bubequ::Scope>& bubequ)
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
			Vector3 pos = targetPos;

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
				else if (linkScope->type == bubequ::LinkType::FUNCTION) {
					middle::Shape linkProto = bubble::newFunction(gameState, linkScope->label, pos);
					middle::Shape& linkShape = middle::registerShape(gameState, linkProto);
					newNodeId = linkShape.id;
				}
				else if (linkScope->type == bubequ::LinkType::SUMMATION) {
					middle::Shape linkProto = bubble::newSummation(gameState, pos);
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

		auto localScaleRoot = middle::getComp<components::LocalScale>(gameState, rootId);
		localScaleRoot->scale.x = gameState->bubbleAlgebraState.worldScale;
		localScaleRoot->scale.y = gameState->bubbleAlgebraState.worldScale;
		localScaleRoot->scale.z = gameState->bubbleAlgebraState.worldScale;

		return rootId;
	}

	inline std::shared_ptr<Scope> bubbleToBubequ(middle::GameState* gameState, const middle::Id id) {
		auto& shape = middle::getShape(gameState, id.index);

		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);

		// early return (DOESN'T MAKE HASHES FOR THESE)
		if (auto unitBubble = middle::getComponent<components::BubbleUnit>(shape)) {
			auto unit = std::make_shared<Unit>();
			unit->type = UnitType::CONSTANT;
			unit->value = unitBubble->value;
			return unit;
		}
		else if (auto var = middle::getComponent<components::BubbleVariable>(shape)) {
			auto unit = std::make_shared<Unit>();
			unit->type = UnitType::VARIABLE;
			unit->label = var->label;
			if (var->isNegative) {
				unit->value = -1;
			}
			else {
				unit->value = 1;
			}
			return unit;
		}
		else if (children.size() == 0) {
			auto unit = std::make_shared<Unit>();
			unit->type = UnitType::ZERO;
			unit->value = 0;
			return unit;
		}

		auto result = std::make_shared<Scope>();

		if (middle::getComponent<components::BubbleMultiplyComponent>(shape)) {
			auto link = std::make_shared<Link>();
			link->type = LinkType::MULTIPLICATION;
			result = link;
		}
		else if (middle::getComponent<components::BubblePowerComponent>(shape)) {
			auto link = std::make_shared<Link>();
			link->type = LinkType::POWER;
			result = link;
		}
		else if (middle::getComponent<components::BubbleSummationComponent>(shape)) {
			auto link = std::make_shared<Link>();
			link->type = LinkType::SUMMATION;
			result = link;
		}
		if (middle::getComponent<components::BubbleInequaltyComponent>(shape)) {
			auto link = std::make_shared<Link>();
			link->type = LinkType::GREATER;
			result = link;
		}
		if (middle::getComponent<components::BubbleEqualsComponent>(shape)) {
			auto link = std::make_shared<Link>();
			link->type = LinkType::EQUALS;
			result = link;
		}
		if (auto func = middle::getComponent<components::BubbleFunctionComponent>(shape)) {
			auto link = std::make_shared<Link>();
			link->type = LinkType::FUNCTION;
			link->label = func->label;
			result = link;
		}

		for (middle::Id& childId : children) {
			std::shared_ptr<Scope> childBubequ = bubbleToBubequ(gameState, childId);
			result->children.push_back(childBubequ);
		}

		return result;
	}

	inline std::string bubequToHashes(middle::GameState* gameState, const std::shared_ptr<Scope>& scope, std::unordered_map<std::string, std::string>& resultMap) {

		std::string bubbleString;

		bubbleString += "(";

		if (auto unit = dynamic_cast<Unit*>(scope.get())) {
			// early return (DOESN'T MAKE HASHES FOR THESE)
			if (unit->type == UnitType::CONSTANT) {
				bubbleString += std::to_string(unit->value);
				return bubbleString + ")";
			}
			else if (unit->type == UnitType::VARIABLE) {
				if (unit->value < 0) {
					bubbleString += "-";
				}
				bubbleString += unit->label;
				return bubbleString + ")";
			}
			else if (unit->type == UnitType::ZERO) {
				bubbleString += ")";
				return bubbleString;
			}
			else{
				assert(false && "not known type");
			}
		}

		if (auto link = dynamic_cast<Link*>(scope.get())) {
			if (link->type == LinkType::MULTIPLICATION) {
				bubbleString += "*";
			}
			else if (link->type == LinkType::POWER) {
				bubbleString += "^";
			}
			else if (link->type == LinkType::SUMMATION) {
				bubbleString += "$";
			}
			if (link->type == LinkType::GREATER) {
				bubbleString += ">";
			}
			if (link->type == LinkType::EQUALS) {
				bubbleString += "=";
			}
			if (link->type == LinkType::FUNCTION) {
				bubbleString += link->label;
			}
		}

		for (auto child : scope->children) {
			std::string childString = bubequToHashes(gameState, child, resultMap);
			if (childString.size() > 0 && childString[0] == '(') {
				bubbleString += childString;
			}
			// assume to be hash so contain in []
			else {
				bubbleString += "[" + childString + "]";
			}
		}

		bubbleString += ")";

		std::string hash = sha256(bubbleString);
		resultMap[hash] = bubbleString;
		return hash;
	}

	inline void replaceBranch(std::shared_ptr<Scope>& root, const std::shared_ptr<Scope>& newBranch, const BubTraversePath& path) {
		if (path.size() == 0) {
			root = newBranch;
			return;
		}
		auto& currentScope = root;
		for (int i = 0; i < path.size(); ++i) {
			int childIndex = path[i];
			currentScope = currentScope->children[childIndex];
		}
		currentScope = newBranch;
	}
}
