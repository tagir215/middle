#pragma once
#include "LoopSociety.h"
#include "game_state.h"
#include "AlgebraNode.h"
using namespace middle;

namespace bubble {
	extern float bubbleAxis;
	extern float bubbleFontSize;

	bool pointIntersectBubble(middle::GameState* gameState, middle::Shape& bubble, const Vector3& point);
	// get left right top bottom of a rect containing rect children 
	void loopRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	void loopChildrenOnlyRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	void loopRectBoundingBoxInternal(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	void bubbleRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	bool buttonClicked(middle::GameState* gameState, middle::Shape& shape, int function);
	std::vector<middle::Id>getNodes(middle::GameState* gameState, middle::Id id);
	std::vector<middle::Id>getConstraints(middle::GameState* gameState, middle::Id id);
	middle::Id findBubbleWithPattern(middle::GameState* gameState, middle::Id containerBubble);
	struct UnitValue {
		float scale = 0;
		float power = 1;
		std::string variableLabel = "";
	};
	struct BubbleValue {
		float scale = 0;
	};
	middle::Id topLevelBubble(middle::GameState* gameState);
	middle::Shape newBubble(middle::GameState* gameState, const Vector3& targetPos);
	middle::Shape newUnit(middle::GameState* gameState, const Vector3& targetPos, bool isNegative = false);
	middle::Shape newVariable(middle::GameState* gameState, const std::string& label, const Vector3& targetPos, bool isNegative = false);
	middle::Shape newEquals(middle::GameState* gameState, const Vector3& targetPos);
	middle::Shape newInequals(middle::GameState* gameState, const Vector3& targetPos, bool equalOr);
	middle::Shape newMultiplication(middle::GameState* gameState, const Vector3& targetPos);
	middle::Shape newPower(middle::GameState* gameState, const Vector3& targetPos);
	middle::Shape newFunction(middle::GameState* gameState, const std::string& label, const Vector3& targetPos);
	middle::Shape newSummation(middle::GameState* gameState, const Vector3& targetPos);
	middle::Id newSummationWithChildren(middle::GameState* gameState, const Vector3& targetPos);
	middle::Id newPower(middle::GameState* gameState, middle::Id baseId, middle::Id exponentId, const Vector3& targetPos);
	middle::Id newBubbleWithIntValue(middle::GameState* gameState, int value, const Vector3& targetPos);
	bool isIntersecting(middle::GameState* gameState, middle::Shape& shape);
	bool unitEquals(middle::GameState* gameState, middle::Id& idA, middle::Id& idB);
	UnitValue unitValue(middle::GameState* gameState, middle::Id& containerId);
	int fractionUnitCount(middle::GameState* gameState, middle::Id& fractionId);
	bool matchingBubbles(middle::GameState* gameState, middle::Id& bubbleA, middle::Id bubbleB);
	bool matchesStructureWithVariables(middle::GameState* gameState, middle::Id bubbleId, middle::Id algebraNodeId);
	bool matchesStructureWithVariables(middle::GameState* gameState, middle::Id bubbleId, middle::Id algebraNodeId, std::unordered_map<std::string, middle::Id>& varOverrides);
	bool matchesStructureBranch(middle::GameState* gameState, middle::Id bubbleStartPointId, middle::Id bubbleRootId, middle::Id structureStartPointId, middle::Id structureRootId);
	void getVariableStructuresMap(middle::GameState* gameState, middle::Id structureId, std::unordered_map<std::string, std::vector<middle::Id>>& resultMap);
	BubbleValue calculateBubbleValue(middle::GameState* gameState, middle::Id bubbleId, std::unordered_map<std::string, int>& variableValues);
	std::string getVariableLabel(middle::GameState* gameState, middle::Id id);
	void generateRandomVariablesValues(middle::GameState* gameState, middle::Id bubbleId, std::unordered_map<std::string, int>& result, int& valueCounter);
	std::unordered_map<std::string, middle::Id> generateVariableOverrides(middle::GameState* gameState, middle::Id bubbleId, middle::Id algebraRootNodeId);
	middle::Id findMatchingBubbleWithVariables(middle::GameState* gameState, middle::Id containerId, middle::Id algebraNodeId, int targetDepth, std::unordered_map<std::string, middle::Id>& varOverrides, std::set<int>ignoreSet = {});
	middle::Id findMatchingStructureWithVariablesFromSibling(middle::GameState* gameState, middle::Id siblingId, middle::Id algebraNodeId, std::unordered_map<std::string, middle::Id>& varOverrides);
	void findMatchingStructurePairWithVariables(middle::GameState* gameState, middle::Id containerId, middle::Id algebraNodeIdA, middle::Id algebraNodeIdB, int targetDepth, std::unordered_map<std::string, middle::Id>& varOverrides, middle::Id& resultIdA, middle::Id& resultIdB);
	middle::Id findMatchingBubble(middle::GameState* gameState, middle::Id bubbleRootId, middle::Id nodeStartPointId, middle::Id nodeRootId, std::unordered_map<std::string, middle::Id>& varOverrides);
	middle::Id findMatchingBubble(middle::GameState* gameState, middle::Id bubbleRootId, middle::Id nodeStartPointId, middle::Id nodeRootId, std::unordered_map<std::string, middle::Id>& varOverrides, std::set<int>& ignoreSet);
	void findMatchingPairBubbles(middle::GameState* gameState, middle::Id bubbleRootId, middle::Id nodeStartPointAId, middle::Id nodeStartPointBId, middle::Id nodeRootId, std::unordered_map<std::string, middle::Id>& varOverrides, middle::Id& resultIdA, middle::Id& resultIdB);
	middle::Id findMatchingFromSibling(middle::GameState* gameState, middle::Id nodeId, middle::Id siblingId, std::unordered_map<std::string, middle::Id>& varOverrides);
	middle::Id containerize(middle::GameState* gameState, middle::Id id);
	bool isAddition(middle::GameState* gameState, middle::Id id);
	bool isPowerBubble(middle::GameState* gameState, middle::Id id);
	bool isSummation(middle::GameState* gameState, middle::Id id);
	bool isMultiplication(middle::GameState* gameState, middle::Id id);
	bool isUnit(middle::GameState* gameState, middle::Id id);
	void negate(middle::GameState* gameState, middle::Id id);
	void invert(middle::GameState* gameState, middle::Id id);
	middle::Id bubbleToStructure(middle::GameState* gameState, middle::Id bubbleId);
	void bubbleToStructureBranch(middle::GameState* gameState, middle::Id startPointBubbleId, middle::Id bubbleRootId, middle::Id& startPointNodeId, middle::Id& rootNodeId);
	components::AlgebraNodeType getStructureType(middle::GameState* gameState, middle::Id id);
	int findDepth(middle::GameState* gameState, middle::Id id);
	int findBubbleDepth(middle::GameState* gameState, middle::Id id);
	bool isBubbleWithValueOne(middle::GameState* gameState, middle::Id id);
	bool isBubbleWithValueOneNegative(middle::GameState* gameState, middle::Id id);
	bool isBubbleZero(middle::GameState* gameState, middle::Id id);
	bool isEqualsOrInequals(middle::GameState* gameState, middle::Id id);
	bool isEqualsBubble(middle::GameState* gameState, middle::Id id);
	bool isInequalBubble(middle::GameState* gameState, middle::Id id);
	bool isFunctionBubble(middle::GameState* gameState, middle::Id id);
	void getPowerBaseAndExponent(middle::GameState* gameState, middle::Id powerBubble, middle::Id& resultBaseId, middle::Id& resultExponentId);
	void getSummationIndexLimitSummand(middle::GameState* gameState, middle::Id summationBubble, middle::Id& resultIndex, middle::Id& resultUpperLimit, middle::Id& resultSummand);
	void getInequaltyLesserAndGreater(middle::GameState* gameState, middle::Id inequalBubble, middle::Id& resultLesserId, middle::Id& resultGreaterId);
	middle::Id getOtherFromContainerOf2(middle::GameState* gameState, middle::Id id);
	void matchBubbleTransforms(middle::GameState* gameState, middle::Id matchingModelId, middle::Id toMatchId);
	// slow but immediate layout update
	void recursiveBubbleLayoutUpdate(middle::GameState* gameState, middle::Id id);

	template<typename T>
	middle::Id findIdWithCompFromShapeOrItsParents(middle::GameState* gameState, middle::Id id) {
		std::stack < middle::Id> parents;
		parents.push(id);
		while (parents.size() > 0) {
			middle::Id currentId = parents.top();
			parents.pop();
			if (currentId.index == middle::UNASSIGNED) {
				return middle::Id();
			}
			auto& parentShape = middle::getShape(gameState, currentId.index);
			if (middle::getComponent<T>(parentShape)) {
				return parentShape.id;
			}
			middle::Id parentId = middle::getParent(gameState, currentId);
			parents.push(parentId);
		}
		return middle::Id();
	}
}
