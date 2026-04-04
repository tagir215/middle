#pragma once
#include "LoopSociety.h"
#include "game_state.h"
#include "AlgebraNode.h"
using namespace middle;

namespace bubble {
	bool pointIntersectBubble(middle::GameState* gameState, middle::Shape& bubble, const Vector3& point);
	// get left right top bottom of a rect containing rect children 
	void loopRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	void loopChildrenOnlyRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	void loopRectBoundingBoxInternal(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	void bubbleRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	bool buttonClicked(middle::GameState* gameState, middle::Shape& shape, int function);
	std::vector<middle::Id>getNodes(middle::GameState* gameState, components::LoopSociety* loop);
	std::vector<middle::Id>getConstraints(middle::GameState* gameState, components::LoopSociety* loop);
	middle::Id findBubbleWithPatern(middle::GameState* gameState, middle::Id containerBubble);
	middle::Id shapeToFraction(middle::GameState* gameState, middle::Id shapeId, const Vector3& targetPos, int dividend);
	struct BubbleValue {
		float scale = 0;
		std::string variableLabel = "";
	};
	middle::Id inverseBubble(middle::GameState* gameState, middle::Id& id);
	middle::Id topLevelBubble(middle::GameState* gameState);
	middle::Shape newBubble(middle::GameState* gameState, const Vector3& targetPos);
	middle::Shape newUnit(middle::GameState* gameState, const Vector3& targetPos);
	middle::Shape newVariable(middle::GameState* gameState, const std::string& label, const Vector3& targetPos);
	middle::Shape newExponent(middle::GameState* gameState, const Vector3& targetPos);
	middle::Id newEquals(middle::GameState* gameState, middle::Id bubbleAId, middle::Id bubbleBId, const Vector3& targetPos);
	bool isIntersecting(middle::GameState* gameState, middle::Shape& shape);
	bool unitEquals(middle::GameState* gameState, middle::Id& idA, middle::Id& idB);
	bool exponentEquals(middle::GameState* gameState, middle::Id& idA, middle::Id& idB);
	BubbleValue unitValue(middle::GameState* gameState, middle::Id& containerId);
	int fractionUnitCount(middle::GameState* gameState, middle::Id& fractionId);
	bool matchingBubbles(middle::GameState* gameState, middle::Id& bubbleA, middle::Id bubbleB);
	bool matchesStructureWithVariables(middle::GameState* gameState, middle::Id bubbleId, middle::Id algebraNodeId);
	middle::Id findMatchingStructureWithVariables(middle::GameState* gameState, middle::Id containerId, middle::Id algebraNodeId, int targetDepth, std::set<int>ignoreSet = {});
	middle::Id findMatchingStructureWithVariablesFromSibling(middle::GameState* gameState, middle::Id siblingId, middle::Id algebraNodeId);
	void findMatchingStructurePairWithVariables(middle::GameState* gameState, middle::Id containerId, middle::Id algebraNodeIdA, middle::Id algebraNodeIdB, int targetDepth, middle::Id& resultIdA, middle::Id& resultIdB);
	middle::Id newFraction(middle::GameState* gameState, const Vector3& targetPos, int dividend);
	middle::Id shapeToFraction(middle::GameState* gameState, middle::Id shpaeId, const Vector3& targetPos, int dividend);
	middle::Id fractionQuotient(middle::GameState* gameState, middle::Id& fractionId);
	bool additiveInverses(middle::GameState* gameState, middle::Id idA, middle::Id idB);
	void negate(middle::GameState* gameState, middle::Id id);
	middle::Id bubbleToStructure(middle::GameState* gameState, middle::Id bubbleId);
	components::AlgebraNodeType getStructureType(middle::GameState* gameState, middle::Id id);
	int findDepth(middle::GameState* gameState, middle::Id id);
	middle::Id findTopDog(middle::GameState* gameState, middle::Id id);
	bool isBubbleWithValueOne(middle::GameState* gameState, middle::Id id);
	bool isBubbleWithValueOneNegative(middle::GameState* gameState, middle::Id id);
}
