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

namespace bubequ{
	inline middle::Id bubequToBubble(middle::GameState* gameState, const Vector3& targetPos, std::shared_ptr<bubequ::Scope>& bubequ);
	inline std::string bubbleToBubequ(middle::GameState* gameState, middle::Id id);

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


	inline std::string bubbleToBubequ(middle::GameState* gameState, const middle::Id id)
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
		else if (middle::getComponent<components::BubbleSummationComponent>(shape)) {
			result += "$";
		}
		if (middle::getComponent<components::BubbleInequaltyComponent>(shape)) {
			result += ">";
		}
		if (middle::getComponent<components::BubbleEqualsComponent>(shape)) {
			result += "=";
		}
		if (auto func = middle::getComponent<components::BubbleFunctionComponent>(shape)) {
			result += func->label;
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

		result += ")";
		return result;
	}

	inline std::string bubbleToBubequHashes(middle::GameState* gameState, const middle::Id id, std::unordered_map<std::string, std::string>& resultMap) {

		auto& shape = middle::getShape(gameState, id.index);

		std::string bubbleString;

		bubbleString += "(";

		if (middle::getComponent<components::BubbleMultiplyComponent>(shape)) {
			bubbleString += "*";
		}
		else if (middle::getComponent<components::BubblePowerComponent>(shape)) {
			bubbleString += "^";
		}
		else if (middle::getComponent<components::BubbleSummationComponent>(shape)) {
			bubbleString += "$";
		}
		if (middle::getComponent<components::BubbleInequaltyComponent>(shape)) {
			bubbleString += ">";
		}
		if (middle::getComponent<components::BubbleEqualsComponent>(shape)) {
			bubbleString += "=";
		}
		if (auto func = middle::getComponent<components::BubbleFunctionComponent>(shape)) {
			bubbleString += func->label;
		}

		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);

		// early return (DOESN'T MAKE HASHES FOR THESE)
		if (auto unit = middle::getComponent<components::BubbleUnit>(shape)) {
			bubbleString += std::to_string(unit->value);
			return bubbleString + ")";
		}
		else if (auto var = middle::getComponent<components::BubbleVariable>(shape)) {
			if (var->isNegative) {
				bubbleString += "-";
			}
			bubbleString += var->label;
			return bubbleString + ")";
		}
		else if (children.size() == 0) {
			bubbleString += ")";
			return bubbleString;
		}
		

		for (middle::Id& childId : children) {
			bubbleString += bubbleToBubequHashes(gameState, childId, resultMap);
		}

		bubbleString += ")";

		std::string hash = sha256(bubbleString);
		resultMap[hash] = bubbleString;
		return hash;
	}
}
