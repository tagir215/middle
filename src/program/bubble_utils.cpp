#include "bubble_utils.h"
#include "middle_component_table.h"
#include "BubbleComponent.h"
#include "middle_shape_utils.h"
#include "Rectangle.h"
#include "InputVariable.h"
#include "OutputVariable.h"
#include "Button.h"
#include "MouseIntersectable.h"
#include "Sphere.h"
#include "Constraint.h"
#include "BubbleRef.h"
#include "editor_actions.h"
#include "PhysicsData.h"
#include "BubbleVariable.h"
#include "Circle.h"
#include "TopDogBubbleTag.h"
#include "MouseSelectable.h"
#include "BubbleEqualsComponent.h"
#include "Layer.h"
#include "ExponentComponent.h"
#include "BubbleAlgebraProblem.h"
#include "component_utils.h"
#include "DeleteComponent.h"
#include "HelperBubbleEquation.h"
#include "EditThisTag.h"
#include "LocalPosition.h"
#include "LocalScale.h"
#include "GlobalTransform.h"

namespace bubble {
	float unitRadius = 14;
	float variableRadius = 24;
	float variableTextFontSize = 29;
	float minTopDogRadius = 50;

	bool pointIntersectBubble(middle::GameState* gameState, middle::Shape& bubbleShape, const Vector3& point)
	{
		auto bubbleComponent = middle::getComponent<components::BubbleComponent>(bubbleShape);
		assert(bubbleComponent);
		auto ref = middle::getComponent<components::BubbleRef>(bubbleShape);
		if (!ref || !middle::isValidId(gameState, ref->idRef)) {
			return false;
		}
		Vector3 center = middle::getShapePosition(gameState, bubbleShape.id.index);

		auto& bubbleContainer = middle::getShape(gameState, ref->idRef.index);

		std::vector<middle::Id> outlineConstraints = getConstraints(gameState, bubbleContainer.id);

		for (int i = 0; i < outlineConstraints.size(); ++i) {
			auto& constraintShape = middle::getShape(gameState, outlineConstraints[i].index);
			auto constraint = middle::getComponent<components::Constraint>(constraintShape);

			auto& idA = constraint->idA;
			auto& idB = constraint->idB;
			Vector3 posA = middle::getShapePosition(gameState, idA.index);
			Vector3 posB = middle::getShapePosition(gameState, idB.index);
			Vector3 dir = posB - posA;
			// 2d normal
			Vector3 normal = { -dir.z, 0 , dir.x };
			Vector3 toCentroid = center - posA;
			if (Vector3DotProduct(normal, toCentroid) > 0) {
				normal = Vector3Negate(normal);
			}

			Vector3 toPoint = point - posA;

			if (Vector3DotProduct(toPoint, normal) > 0) {
				return false;
			}
		}
		return true;
	}

	void loopRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ)
	{
		*leftX = 100000;
		*rightX = -100000;
		*bottomZ = *leftX;
		*topZ = *rightX;
		loopRectBoundingBoxInternal(gameState, shapeId, leftX, rightX, bottomZ, topZ);
	}

	void loopChildrenOnlyRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ)
	{
		auto& shape = middle::getShape(gameState, shapeId.index);
		*leftX = 100000;
		*rightX = -100000;
		*bottomZ = *leftX;
		*topZ = *rightX;
		std::vector<middle::Id>children;
		middle::getChildren(gameState, shape.id, children);
		for (const middle::Id& childId : children) {
			loopRectBoundingBoxInternal(gameState, childId, leftX, rightX, bottomZ, topZ);
		}
	}

	bool shouldSkipRectBound(middle::Shape& shape) {
		if (!middle::getComponent<components::Rectangle>(shape)) {
			return true;
		}
		if (middle::getComponent<components::InputVariable>(shape)) {
			return true;
		}
		if (middle::getComponent<components::OutputVariable>(shape)) {
			return true;
		}
		return false;
	}

	void loopRectBoundingBoxInternal(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ)
	{
		auto& shape = middle::getShape(gameState, shapeId.index);
		Vector3 pos = middle::getShapePosition(gameState, shapeId.index);
		auto rect = middle::getComponent<components::Rectangle>(shape);
		if (shouldSkipRectBound(shape)) {
			return;
		}
		Vector3 s = getTotalScale(gameState, shape.id);

		float top = pos.z + rect->height * 0.5f * s.z;
		float bottom = pos.z - rect->height * 0.5f * s.z;
		float left = pos.x - rect->width * 0.5f * s.x;
		float right = pos.x + rect->width * 0.5f * s.x;
		if (top > *topZ) {
			*topZ = top;
		}
		if (bottom < *bottomZ) {
			*bottomZ = bottom;
		}
		if (left < *leftX) {
			*leftX = left;
		}
		if (right > *rightX) {
			*rightX = right;
		}

		std::vector < middle::Id>children;
		middle::getChildren(gameState, shape.id, children);
		for (const middle::Id& childId : children) {
			loopRectBoundingBoxInternal(gameState, childId, leftX, rightX, bottomZ, topZ);
		}
	}

	void bubbleRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ)
	{
		auto& shape = middle::getShape(gameState, shapeId.index);
		std::vector<middle::Id> allThingsInTheBubbleSinceTheBeginningOfTime;
		allThingsInTheBubbleSinceTheBeginningOfTime.push_back(shape.id);
		middle::getAllChildren(gameState, shapeId, allThingsInTheBubbleSinceTheBeginningOfTime);
		*leftX = 100000;
		*rightX = -100000;
		*bottomZ = *leftX;
		*topZ = *rightX;
		for (middle::Id& id : allThingsInTheBubbleSinceTheBeginningOfTime) {
			auto& child = middle::getShape(gameState, id.index);
			auto circle = middle::getComponent<components::Circle>(child);
			if (!circle) {
				continue;
			}
			auto childPos = middle::getComponent<components::Position>(child);
			float left = childPos->posX - circle->radius;
			float right = childPos->posX + circle->radius;
			float top = childPos->posZ + circle->radius;
			float bottom = childPos->posZ - circle->radius;
			if (left < *leftX) {
				*leftX = left;
			}
			if (right > *rightX) {
				*rightX = right;
			}
			if (bottom < *bottomZ) {
				*bottomZ = bottom;
			}
			if (top > *topZ) {
				*topZ = top;
			}
		}
	}

	bool buttonClicked(middle::GameState* gameState, middle::Shape& shape, int function)
	{
		if (!gameState->input.mouseClicked) {
			return false;
		}
		auto button = middle::getComponent<components::Button>(shape);
		if (!button) {
			return false;
		}
		if (button->function != function) {
			return false;
		}
		auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
		assert(intersectable);
		if (intersectable->intersectingTop) {
			return true;
		}
		return false;
	}


	std::vector<middle::Id>getNodes(middle::GameState* gameState, middle::Id id) {
		std::vector<middle::Id>ids;
		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);
		for (middle::Id& id : children) {
			auto& childShape = middle::getShape(gameState, id.index);
			if (middle::getComponent<components::Sphere>(childShape)) {
				ids.push_back(id);
			}
		}
		return ids;
	}

	std::vector<middle::Id>getConstraints(middle::GameState* gameState, middle::Id id) {
		std::vector<middle::Id>ids;
		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);
		for (middle::Id& id : children) {
			auto& childShape = middle::getShape(gameState, id.index);
			if (middle::getComponent<components::Constraint>(childShape)) {
				ids.push_back(id);
			}
		}
		return ids;
	}

	middle::Id findBubbleWithPattern(middle::GameState* gameState, middle::Id containerBubble)
	{
		std::queue<middle::Id>ids;
		ids.push(containerBubble);
		while (ids.size() > 0) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, containerBubble, children);

		}
		return middle::Id();
	}

	middle::Id inverseBubble(middle::GameState* gameState, middle::Id& id)
	{
		middle::Id copy = middle::deepCopyShape(gameState, id.index);
		auto& copyShape = middle::getShape(gameState, copy.index);
		auto bubble = middle::getComponent<components::BubbleComponent>(copyShape);
		return copy;
	}

	middle::Id topLevelBubble(middle::GameState* gameState)
	{
		middle::Id resultId;
		middle::loopInstances(gameState, [gameState, &resultId](int i, middle::Shape& shape) {
			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			auto mul = middle::getComponent<components::BubbleMultiplyComponent>(shape);
			if (!bubble && !mul) {
				return true;
			}
			middle::Id& parentId = middle::getParent(gameState, shape.id);
			if (parentId.index == middle::UNASSIGNED) {
				resultId = shape.id;
				return false;
			}
			return true;
			});
		return resultId;
	}



	bool isIntersecting(middle::GameState* gameState, middle::Shape& shape) {
		auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);

		return intersectable->intersectingTop;
	}

	bool unitEquals(middle::GameState* gameState, middle::Id& idA, middle::Id& idB)
	{
		UnitValue valueA = unitValue(gameState, idA);
		UnitValue valueB = unitValue(gameState, idB);
		const float epsilon = 1e-4f;
		bool equalMagnitude = std::abs(valueA.scale - valueB.scale) < epsilon;
		bool equalLabel = valueA.variableLabel == valueB.variableLabel;
		return equalMagnitude && equalLabel;
	}

	bool isBubbleWithSingleVariable(middle::GameState* gameState, middle::Id bubbleId) {
		std::vector<middle::Id> children;
		middle::getChildren(gameState, bubbleId, children);
		if (children.size() != 1) {
			return false;
		}
		return getStructureType(gameState, children[0]) == components::AlgebraNodeType::VARIABLE;
	}

	bool containerStructureEquals(middle::GameState* gameState, middle::Id idA, middle::Id idB) {
		middle::Shape& shapeA = middle::getShape(gameState, idA.index);
		middle::Shape& shapeB = middle::getShape(gameState, idB.index);
		// make sure same type of container
		auto typeA = getStructureType(gameState, shapeA.id);
		auto typeB = getStructureType(gameState, shapeB.id);
		if (typeA != typeB) {
			return false;
		}

		// make sure equal amount of children
		std::vector<middle::Id>childrenA;
		std::vector<middle::Id>childrenB;
		middle::getChildren(gameState, shapeA.id, childrenA);
		middle::getChildren(gameState, shapeB.id, childrenB);
		int size = childrenA.size();
		if (size != childrenB.size()) {
			return false;
		}

		return true;
	}


	UnitValue unitValue(middle::GameState* gameState, middle::Id& containerId)
	{
		UnitValue result;
		middle::Shape& shape = middle::getShape(gameState, containerId.index);
		auto unit = middle::getComponent<components::BubbleUnit>(shape);
		if (unit) {
			result.scale = unit->value;
		}
		auto node = middle::getComponent<components::AlgebraNode>(shape);
		if (node) {
			result.scale = node->value;
		}
		return result;
	}

	int fractionUnitCount(middle::GameState* gameState, middle::Id& fractionId)
	{
		auto& shape = middle::getShape(gameState, fractionId.index);
		auto loop = middle::getComponent<components::LoopSociety>(shape);
		return loop->loopMemberIds.size();
	}

	bool bubblePropertiesEqual(middle::GameState* gameState, middle::Id& idA, middle::Id idB) {
		auto& shapeA = middle::getShape(gameState, idA.index);
		auto& shapeB = middle::getShape(gameState, idB.index);
		auto bubbleA = middle::getComponent<components::BubbleComponent>(shapeA);
		auto bubbleB = middle::getComponent<components::BubbleComponent>(shapeB);
		auto nodeA = middle::getComponent<components::AlgebraNode>(shapeA);
		auto nodeB = middle::getComponent<components::AlgebraNode>(shapeB);
		auto varA = middle::getComponent<components::BubbleVariable>(shapeA);
		// only idB is allowed to be AlgebraNode type
		assert(!nodeA);

		if (bubbleA && bubbleB) {
			auto varB = middle::getComponent<components::BubbleVariable>(shapeB);
			if (varA && varB) {
				if (varA->isNegative != varB->isNegative || varA->label != varB->label) {
					return false;
				}
			}
			return true;
		}
		if (bubbleA && nodeB) {
			if (varA) {
				if (varA->isNegative != nodeB->isNegative || varA->label != nodeB->variableLabel) {
					return false;
				}
			}
			return true;
		}
		return false;
	}

	bool matchingBubbles(middle::GameState* gameState, middle::Id& idA, middle::Id idB) {
		auto& shapeA = middle::getShape(gameState, idA.index);
		auto& shapeB = middle::getShape(gameState, idB.index);
		auto bubbleA = middle::getComponent<components::BubbleComponent>(shapeA);
		auto bubbleB = middle::getComponent<components::BubbleComponent>(shapeB);
		auto unitA = middle::getComponent<components::BubbleUnit>(shapeA);
		auto unitB = middle::getComponent<components::BubbleUnit>(shapeB);
		auto nodeA = middle::getComponent<components::AlgebraNode>(shapeA);
		auto nodeB = middle::getComponent<components::AlgebraNode>(shapeB);
		// idB is allowed to be AlgebraNode, but not idA
		assert(!nodeA);
		auto typeA = getStructureType(gameState, idA);
		auto typeB = getStructureType(gameState, idB);

		// if one is inverse other is not return false
		if ((typeA == components::AlgebraNodeType::BUBBLE && (typeB == components::AlgebraNodeType::BUBBLE)) ||
			(typeA == components::AlgebraNodeType::VARIABLE && typeB == components::AlgebraNodeType::VARIABLE)) {
			if (!bubblePropertiesEqual(gameState, idA, idB)) {
				return false;
			}
		}

		// check that units equal
		bool bothUnits = typeA == components::AlgebraNodeType::UNIT && typeB == components::AlgebraNodeType::UNIT;
		if (bothUnits) {
			return unitEquals(gameState, idA, idB);
		}

		// if both are non units, so are some kind of containers
		bool neitherUnits = typeA != components::AlgebraNodeType::UNIT && typeB != components::AlgebraNodeType::UNIT;
		if (neitherUnits) {
			if (!containerStructureEquals(gameState, idA, idB)) {
				return false;
			}
			std::vector<middle::Id>childrenA;
			std::vector<middle::Id>childrenB;
			middle::getChildren(gameState, shapeA.id, childrenA);
			middle::getChildren(gameState, shapeB.id, childrenB);
			int size = childrenA.size();
			// store found matches
			std::vector<bool> mem;
			mem.resize(size);
			for (auto& b : mem) {
				b = false;
			}
			// see if matching strcutres of children between the 2 containers
			int matchingCount = 0;
			for (middle::Id& id : childrenA) {
				for (int i = 0; i < size; ++i) {
					if (mem[i]) {
						continue;
					}
					if (matchingBubbles(gameState, id, childrenB[i])) {
						++matchingCount;
						mem[i] = true;
						break;
					}
				}
			}
			return matchingCount == size;
		}

		// one is unit other is structure, or somehting else weird
		return false;
	}

	bool matchesStructureWithVariables(middle::GameState* gameState, middle::Id bubbleId, middle::Id algebraNodeId)
	{
		std::unordered_map<std::string, middle::Id> overrideMap;
		return matchesStructureWithVariables(gameState, bubbleId, algebraNodeId, overrideMap);
	}

	// TODO sucks
	middle::Id abstractNodeToConcreteBubble(middle::GameState* gameState, middle::Id algebraNodeId, std::unordered_map<std::string, middle::Id>& varOverrides) {
		assert(getStructureType(gameState, algebraNodeId) == components::AlgebraNodeType::VARIABLE);
		std::string varName = getVariableLabel(gameState, algebraNodeId);
		if (varOverrides.find(varName) != varOverrides.end()) {
			middle::Id overridingId = varOverrides[varName];
			auto& nodeShape = middle::getShape(gameState, algebraNodeId.index);
			auto node = middle::getComponent<components::AlgebraNode>(nodeShape);
			auto& overridingNodeShape = middle::getShape(gameState, overridingId.index);
			auto overridingNode = middle::getComponent<components::AlgebraNode>(overridingNodeShape);
			if (node->power != 1 && overridingNode->power == 1) {
				middle::Id nodeCopyId = middle::deepCopyShape(gameState, overridingId.index);
				auto& nodeCopyShape = middle::getShape(gameState, nodeCopyId.index);
				auto nodeCopy = middle::getComponent<components::AlgebraNode>(nodeCopyShape);
				nodeCopy->power = node->power;
				nodeCopy->isNegativePower = node->isNegativePower;
				nodeCopy->isInversePower = node->isInversePower;
				middle::attachComponent<components::DeleteComponent>(gameState, nodeCopyId);
				return nodeCopyId;
			}

			return overridingId;
		}
		return middle::Id();
	}


	bool matchesStructureWithVariables(middle::GameState* gameState, middle::Id bubbleId, middle::Id algebraNodeId, std::unordered_map<std::string, middle::Id>& varOverrides) {
		auto algebraNodeType = getStructureType(gameState, algebraNodeId);

		// replace node from algebra structure with overriding value
		if (varOverrides.size() > 0 && algebraNodeType == components::AlgebraNodeType::VARIABLE) {

			middle::Id overridingId = abstractNodeToConcreteBubble(gameState, algebraNodeId, varOverrides);
			if (overridingId.index != middle::UNASSIGNED) {
				algebraNodeId = overridingId;
				algebraNodeType = getStructureType(gameState, overridingId);
			}
		}

		auto bubbleType = getStructureType(gameState, bubbleId);
		bool aUnit = algebraNodeType == components::AlgebraNodeType::UNIT;
		bool bUnit = bubbleType == components::AlgebraNodeType::UNIT;


		if (aUnit && bUnit) {
			return unitEquals(gameState, bubbleId, algebraNodeId);
		}

		if (algebraNodeType == components::AlgebraNodeType::VARIABLE && bubbleType == components::AlgebraNodeType::VARIABLE) {
			return bubblePropertiesEqual(gameState, bubbleId, algebraNodeId);
		}

		auto& bubbleShape = middle::getShape(gameState, bubbleId.index);
		auto& structureRootShape = middle::getShape(gameState, algebraNodeId.index);

		if (!aUnit && !bUnit) {
			// containers need to match if algebra node is a container
			if (!containerStructureEquals(gameState, bubbleId, algebraNodeId)) {
				return false;
			}
			std::vector<middle::Id>childrenA;
			std::vector<middle::Id>childrenB;
			middle::getChildren(gameState, bubbleShape.id, childrenA);
			middle::getChildren(gameState, structureRootShape.id, childrenB);
			int size = childrenA.size();
			// store found matches
			std::vector<bool> mem;
			mem.resize(size);
			for (auto& b : mem) {
				b = false;
			}
			// see if matching strcutres of children between the 2 containers
			int matchingCount = 0;
			for (middle::Id& id : childrenA) {
				for (int i = 0; i < size; ++i) {
					if (mem[i]) {
						continue;
					}

					if (matchesStructureWithVariables(gameState, id, childrenB[i], varOverrides)) {
						++matchingCount;
						mem[i] = true;
						break;
					}
				}
			}
			return matchingCount == size;
		}

		return false;
	}

	bool matchesStructureBranch(middle::GameState* gameState, middle::Id bubbleStartPointId, middle::Id bubbleRootId, middle::Id structureStartPointId, middle::Id structureRootId)
	{
		std::stack<middle::Id> bubbleStack;
		std::stack<middle::Id> nodeStack;
		bubbleStack.push(bubbleStartPointId);
		bubbleStack.push(structureStartPointId);
		while (bubbleStack.size() > 0) {
			middle::Id bubbleStackTopId = bubbleStack.top();
			bubbleStack.pop();
			middle::Id nodeStackTopId = nodeStack.top();
			nodeStack.pop();
			if (!matchingBubbles(gameState, bubbleStackTopId, nodeStackTopId)) {
				return false;
			}
			if (bubbleStackTopId == bubbleRootId && nodeStackTopId == structureRootId) {
				return true;
			}
			middle::Id bubbleParentId = middle::getParent(gameState, bubbleStackTopId);
			middle::Id nodeParentId = middle::getParent(gameState, nodeStackTopId);
			if (bubbleParentId.index == middle::UNASSIGNED || nodeParentId.index == middle::UNASSIGNED) {
				return false;
			}
			bubbleStack.push(bubbleParentId);
			nodeStack.push(nodeParentId);
		}
	}

	void findMembersAtDepth(middle::GameState* gameState, middle::Id containerId, int targetDepth, std::vector<middle::Id>& results) {
		std::queue<middle::Id>idQueue;
		std::queue<int>depthQueue;
		idQueue.push(containerId);
		depthQueue.push(0);
		while (idQueue.size() > 0) {
			middle::Id currentId = idQueue.front();
			idQueue.pop();
			int currentDepth = depthQueue.front();
			depthQueue.pop();
			if (currentDepth == targetDepth) {
				results.push_back(currentId);
			}
			auto& shape = middle::getShape(gameState, currentId.index);
			std::vector<middle::Id>children;
			middle::getChildren(gameState, shape.id, children);
			for (middle::Id& id : children) {
				idQueue.push(id);
				depthQueue.push(currentDepth + 1);
			}
		}
	}


	bool appendBubbleAsStrcuctureIfNotExisting(middle::GameState* gameState, middle::Id idToAppend, std::vector<middle::Id>& list) {
		for (middle::Id& id : list) {
			if (matchingBubbles(gameState, idToAppend, id)) {
				return false;
			}
		}
		list.push_back(bubbleToStructure(gameState, idToAppend));
		return true;
	}

	void getUniqueSetOfMatchingStructures(middle::GameState* gameState, middle::Id containerId, middle::Id structureId, std::vector<middle::Id>& results) {
		std::set<int>ignoreSet;
		std::unordered_map<std::string, middle::Id> overrides;
		int depth = findDepth(gameState, structureId);
		std::vector<middle::Id> idsAtDepth;
		findMembersAtDepth(gameState, containerId, depth, idsAtDepth);
		for (middle::Id id : idsAtDepth) {
			appendBubbleAsStrcuctureIfNotExisting(gameState, id, results);
		}
	}

	struct VariationNode {
		int index;
		std::shared_ptr<VariationNode>parentNode;
		std::vector<std::shared_ptr<VariationNode>>children;
		std::vector<int> indexPool;
	};
	void indexPoolToChildren(std::shared_ptr<VariationNode>node) {
		for (int i : node->indexPool) {
			auto newNode = std::make_shared<VariationNode>();
			newNode->parentNode = node;
			newNode->index = i;
			newNode->indexPool = {};
			for (int index : node->indexPool) {
				if (index == i)
					continue;
				newNode->indexPool.push_back(index);
			}
			node->children.push_back(newNode);
			indexPoolToChildren(newNode);
		}
	}
	void braveTraveller(std::shared_ptr<VariationNode>& node, std::vector<int>& result) {
		if (node->parentNode) {
			result.push_back(node->index);
			braveTraveller(node->parentNode, result);
		}
	}
	void variationTreeToVectors(std::shared_ptr<VariationNode>& node, std::vector<std::vector<int>>& result) {
		if (node->children.size() == 0) {
			result.push_back({});
			braveTraveller(node, result.back());
		}
		else {
			for (auto& child : node->children) {
				variationTreeToVectors(child, result);
			}
		}
	}

	void generateRandomVariablesValues(middle::GameState* gameState, middle::Id bubbleId, std::unordered_map<std::string, int>& result, int& valueCounter) {
		auto shape = middle::getShape(gameState, bubbleId.index);
		auto var = middle::getComponent<components::BubbleVariable>(shape);
		if (var && result.find(var->label) == result.end()) {
			result[var->label] = valueCounter++;
		}
		std::vector<middle::Id> children;
		middle::getChildren(gameState, bubbleId, children);
		for (middle::Id& id : children) {
			generateRandomVariablesValues(gameState, id, result, valueCounter);
		}
	}

	std::unordered_map<std::string, middle::Id> generateVariableOverrides(middle::GameState* gameState, middle::Id bubbleId, middle::Id algebraRootNodeId) {
		std::unordered_map<std::string, middle::Id> resultMap;

		std::unordered_map<std::string, std::vector<middle::Id>> varMap;
		getVariableStructuresMap(gameState, algebraRootNodeId, varMap);
		std::vector<middle::Id>uniqueSetOfMatchingStructures;

		for (auto& pair : varMap) {
			auto& variableLabel = pair.first;
			auto& variableStructureIds = pair.second;
			for (middle::Id& structureId : variableStructureIds) {
				getUniqueSetOfMatchingStructures(gameState, bubbleId, structureId, uniqueSetOfMatchingStructures);
			}
		}

		int count = uniqueSetOfMatchingStructures.size();

		if (count == 0) {
			return resultMap;
		}

		auto rootNode = std::make_shared<VariationNode>();
		for (int i = 0; i < count; ++i) {
			rootNode->indexPool.push_back(i);
		}
		indexPoolToChildren(rootNode);

		std::vector<std::vector<int>> variations;
		variationTreeToVectors(rootNode, variations);

		int varCount = varMap.size();

		// TODO
		// TODO 
		if (count < varCount) {
			return resultMap;
		}

		for (auto& variation : variations) {
			resultMap.clear();

			int varIndex = 0;
			for (auto& pair : varMap) {
				auto& variableLabel = pair.first;
				middle::Id& structureId = uniqueSetOfMatchingStructures[variation[varIndex]];
				resultMap[variableLabel] = structureId;
				++varIndex;

			}
			if (matchesStructureWithVariables(gameState, bubbleId, algebraRootNodeId, resultMap)) {
				return resultMap;
			}
		}

		resultMap.clear();
		return resultMap;
	}
	


	BubbleValue calculateBubbleValue(middle::GameState* gameState, middle::Id bubbleId, std::unordered_map<std::string, int>& variableValues)
	{

		BubbleValue result;

		auto& bubbleShape = middle::getShape(gameState, bubbleId.index);
		auto variable = middle::getComponent<components::BubbleVariable>(bubbleShape);
		auto node = middle::getComponent<components::AlgebraNode>(bubbleShape);
		auto bubbleUnit = middle::getComponent<components::BubbleUnit>(bubbleShape);

		if (bubbleUnit) {
			result.scale += bubbleUnit->value;
			return result;
		}
		if (variable) {
			result.scale = variableValues[variable->label];
			if (variable->isNegative) {
				result.scale = -result.scale;
			}
		}
		else if (node) {
			if (getStructureType(gameState, bubbleId) == components::AlgebraNodeType::VARIABLE) {
				result.scale = variableValues[node->variableLabel];
				if (node->isNegative) {
					result.scale = -result.scale;
				}
			}
			else {
				result.scale = node->value;
			}
			return result;
		}
		else {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, bubbleId, children);
			for (middle::Id& id : children) {
				auto& shape = middle::getShape(gameState, id.index);
				auto bubble = middle::getComponent<components::BubbleComponent>(shape);
				auto mul = middle::getComponent<components::BubbleMultiplyComponent>(shape);
				if (bubble) {
					BubbleValue value = calculateBubbleValue(gameState, shape.id, variableValues);
					result.scale += value.scale;
				}
				else if (mul) {
					std::vector<middle::Id> mulChildren;
					middle::getChildren(gameState, shape.id, mulChildren);
					BubbleValue mulResult;
					mulResult.scale = 1;
					if (mul->operationType == static_cast<int>(components::OperationType::MULTIPLICATION)) {
						for (int x = 0; x < mulChildren.size(); ++x) {
							BubbleValue val = calculateBubbleValue(gameState, mulChildren[x], variableValues);
							mulResult.scale *= val.scale;
						}
						result.scale += mulResult.scale;
					}
					else if (mul->operationType == static_cast<int>(components::OperationType::POWER)) {
						assert(mulChildren.size() == 2);
						middle::Id baseId = mulChildren[components::PowerRole::POWER_BASE];
						middle::Id exponentId = mulChildren[components::PowerRole::POWER_EXPONENT];
						BubbleValue baseVal = calculateBubbleValue(gameState, baseId, variableValues);
						BubbleValue exponentVal = calculateBubbleValue(gameState, exponentId, variableValues);
						float powResult = std::pow(baseVal.scale, exponentVal.scale);
						result.scale += powResult;
					}
				}
			}
		}

		auto& shape = middle::getShape(gameState, bubbleId.index);
		auto bubble = middle::getComponent<components::BubbleComponent>(shape);

		return result;
	}

	std::string getVariableLabel(middle::GameState* gameState, middle::Id id) {
		std::string result;
		auto& shape = middle::getShape(gameState, id.index);
		if (getStructureType(gameState, id) != components::AlgebraNodeType::VARIABLE) {
			assert(false);
		}
		auto node = middle::getComponent<components::AlgebraNode>(shape);
		auto var = middle::getComponent<components::BubbleVariable>(shape);
		auto bub = middle::getComponent<components::BubbleComponent>(shape);
		if (node) {
			if (node->isNegative) {
				result += "-";
			}
			result += node->variableLabel;
		}
		else {
			assert(var && bub);
			if (var->isNegative) {
				result += "-";
			}
			result += var->label;
		}
		return result;
	}

	void getVariableStructuresMap(middle::GameState* gameState, middle::Id structureId, std::unordered_map<std::string, std::vector<middle::Id>>& resultMap) {

		std::vector<middle::Id>children;
		middle::getChildren(gameState, structureId, children);

		for (middle::Id& childId : children) {
			if (getStructureType(gameState, childId) == components::AlgebraNodeType::VARIABLE) {
				std::string label = getVariableLabel(gameState, childId);
				if (resultMap.find(label) == resultMap.end()) {
					resultMap[label] = {};
				}
				resultMap[label].push_back(childId);
			}
			getVariableStructuresMap(gameState, childId, resultMap);
		}
	}

	middle::Id findMatchingBubbleWithVariables(middle::GameState* gameState, middle::Id containerId, middle::Id algebraNodeId, int targetDepth, std::unordered_map<std::string, middle::Id>& varOverrides, std::set<int>ignoreSet)
	{
		std::queue<middle::Id>idQueue;
		std::queue<int>depthQueue;
		idQueue.push(containerId);
		depthQueue.push(0);
		while (idQueue.size() > 0) {
			middle::Id currentId = idQueue.front();
			idQueue.pop();
			int currentDepth = depthQueue.front();
			depthQueue.pop();

			if (ignoreSet.find(currentId.index) != ignoreSet.end()) {
				continue;
			}

			if (currentDepth == targetDepth) {
				// return id of the first bubble or structure element that matches algebra node structure
				if (matchesStructureWithVariables(gameState, currentId, algebraNodeId, varOverrides)) {
					return currentId;
				}
			}

			auto& shape = middle::getShape(gameState, currentId.index);
			std::vector<middle::Id>children;
			middle::getChildren(gameState, shape.id, children);
			for (middle::Id& id : children) {
				idQueue.push(id);
				depthQueue.push(currentDepth + 1);
			}

		}
		return middle::Id();
	}

	middle::Id findMatchingStructureWithVariablesFromSibling(middle::GameState* gameState, middle::Id siblingId, middle::Id algebraNodeId, std::unordered_map<std::string, middle::Id>& varOverrides)
	{
		middle::Id parentId = middle::getParent(gameState, siblingId);
		if (parentId.index == middle::UNASSIGNED) {
			return middle::Id();
		}
		std::vector<middle::Id>children;
		middle::getChildren(gameState, parentId, children);
		for (middle::Id id : children) {
			if (siblingId == id) {
				continue;
			}
			if (matchesStructureWithVariables(gameState, id, algebraNodeId, varOverrides)) {
				return id;
			}
		}
		return middle::Id();
	}

	void findMatchingStructurePairWithVariables(middle::GameState* gameState, middle::Id containerId, middle::Id algebraNodeIdA, middle::Id algebraNodeIdB, int targetDepth, std::unordered_map<std::string, middle::Id>& varOverrides, middle::Id& resultIdA, middle::Id& resultIdB)
	{
		std::set<int>ignoreSet;
		while (true) {
			middle::Id idACandidate = findMatchingBubbleWithVariables(gameState, containerId, algebraNodeIdA, targetDepth, varOverrides, ignoreSet);
			if (idACandidate.index == middle::UNASSIGNED) {
				return;
			}
			middle::Id idBCandidate = findMatchingStructureWithVariablesFromSibling(gameState, idACandidate, algebraNodeIdB, varOverrides);
			if (idBCandidate.index != UNASSIGNED) {
				resultIdA = idACandidate;
				resultIdB = idBCandidate;
				return;
			}
			ignoreSet.insert(idACandidate.index);
		}
	}

	void getRelativeDepth(middle::GameState* gameState, middle::Id nodeId, middle::Id nodeRootId, int& depth) {
		if (nodeId == nodeRootId) {
			return;
		}
		middle::Id parentId = middle::getParent(gameState, nodeId);
		++depth;
		getRelativeDepth(gameState, parentId, nodeRootId, depth);
	}
	bool matchesBottomUp(middle::GameState* gameState, middle::Id bubbleId, middle::Id nodeId, middle::Id nodeRootId, std::unordered_map<std::string, middle::Id>& varOverrides) {
		if (!matchesStructureWithVariables(gameState, bubbleId, nodeId, varOverrides)) {
			return false;
		}
		if (nodeId == nodeRootId) {
			return true;
		}
		middle::Id bubbleParentId = middle::getParent(gameState, bubbleId);
		middle::Id nodeParentId = middle::getParent(gameState, nodeId);
		return matchesBottomUp(gameState, bubbleParentId, nodeParentId, nodeRootId, varOverrides);
	}
	middle::Id findMatchingBubble(middle::GameState* gameState, middle::Id bubbleRootId, middle::Id nodeStartPointId, middle::Id nodeRootId, std::unordered_map<std::string, middle::Id>& varOverrides)
	{
		std::set<int>ignoreSet;
		return findMatchingBubble(gameState, bubbleRootId, nodeStartPointId, nodeRootId, varOverrides, ignoreSet);
	}
	void findMatchingPairBubbles(middle::GameState* gameState, middle::Id bubbleRootId, middle::Id nodeStartPointAId, middle::Id nodeStartPointBId, middle::Id nodeRootId, std::unordered_map<std::string, middle::Id>& varOverrides, middle::Id& resultIdA, middle::Id& resultIdB) {
		std::set<int>ignoreSet;
		while (true) {
			middle::Id idACandidate = findMatchingBubble(gameState, bubbleRootId, nodeStartPointAId, nodeRootId, varOverrides, ignoreSet);
			if (idACandidate.index == middle::UNASSIGNED) {
				return;
			}
			middle::Id idBCandidate = findMatchingFromSibling(gameState, nodeStartPointBId, idACandidate, varOverrides);
			if (idBCandidate.index != UNASSIGNED) {
				resultIdA = idACandidate;
				resultIdB = idBCandidate;
				return;
			}
			ignoreSet.insert(idACandidate.index);
		}
	}

	middle::Id findMatchingFromSibling(middle::GameState* gameState, middle::Id nodeId, middle::Id siblingId, std::unordered_map<std::string, middle::Id>& varOverrides)
	{
		middle::Id parentId = middle::getParent(gameState, siblingId);
		if (parentId.index == middle::UNASSIGNED) {
			return middle::Id();
		}
		std::vector<middle::Id>children;
		middle::getChildren(gameState, parentId, children);
		for (middle::Id id : children) {
			if (siblingId == id) {
				continue;
			}
			if (matchesStructureWithVariables(gameState, id, nodeId, varOverrides)) {
				return id;
			}
		}
		return middle::Id();
	}

	middle::Id findMatchingBubble(middle::GameState* gameState, middle::Id bubbleRootId, middle::Id nodeStartPointId, middle::Id nodeRootId, std::unordered_map<std::string, middle::Id>& varOverrides, std::set<int>& ignoreSet)
	{
		int depth = 0;
		getRelativeDepth(gameState, nodeStartPointId, nodeRootId, depth);
		std::vector<middle::Id> bubbleStartPoints;
		findMembersAtDepth(gameState, bubbleRootId, depth, bubbleStartPoints);
		for (middle::Id id : bubbleStartPoints) {
			if (ignoreSet.find(id.index) != ignoreSet.end()) {
				continue;
			}
			if (matchesBottomUp(gameState, id, nodeStartPointId, nodeRootId, varOverrides)) {
				return id;
			}
		}
		return middle::Id();
	}

	void negate(middle::GameState* gameState, middle::Id id) {
		auto& shape = middle::getShape(gameState, id.index);
		auto unit = middle::getComponent<components::BubbleUnit>(shape);
		if (unit) {
			unit->value = -unit->value;
			return;
		}
		auto var = middle::getComponent<components::BubbleVariable>(shape);
		if (var) {
			var->isNegative = !var->isNegative;
			return;
		}
		auto loop = middle::getComponent<components::LoopSociety>(shape);
		auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(shape);

		// negate first only
		if (mulComp) {
			negate(gameState, loop->loopMemberIds[0]);
			return;
		}

		for (middle::Id childId : loop->loopMemberIds) {
			negate(gameState, childId);
		}
	}

	void invert(middle::GameState* gameState, middle::Id id)
	{
		auto& shape = middle::getShape(gameState, id.index);
		auto bub = middle::getComponent<components::BubbleComponent>(shape);
	}

	components::AlgebraNodeType getStructureType(middle::GameState* gameState, middle::Id id) {
		auto& shape = middle::getShape(gameState, id.index);
		if (middle::getComponent<components::BubbleVariable>(shape)) {
			return components::AlgebraNodeType::VARIABLE;
		}
		if (middle::getComponent<components::BubbleComponent>(shape)) {
			return components::AlgebraNodeType::BUBBLE;
		}
		if (middle::getComponent<components::BubbleUnit>(shape)) {
			return components::AlgebraNodeType::UNIT;
		}
		if (middle::getComponent<components::BubbleMultiplyComponent>(shape)) {
			return components::AlgebraNodeType::MULTIPLICATION;
		}
		if (middle::getComponent<components::FractionalComponent>(shape)) {
			return components::AlgebraNodeType::FRACTION;
		}
		if (middle::getComponent<components::BubbleEqualsComponent>(shape)) {
			return components::AlgebraNodeType::EQUALS;
		}
		auto algebraNode = middle::getComponent<components::AlgebraNode>(shape);
		if (algebraNode) {
			return static_cast<components::AlgebraNodeType>(algebraNode->type);
		}
		assert(false);
	}

	int findDepth(middle::GameState* gameState, middle::Id id)
	{
		std::stack < middle::Id> parents;
		parents.push(id);
		int depth = 0;
		while (parents.size() > 0) {
			middle::Id currentId = parents.top();
			parents.pop();

			auto& parentShape = middle::getShape(gameState, currentId.index);
			if (middle::getComponent<components::InputVariable>(parentShape)) {
				return depth - 1;
			}
			if (middle::getComponent<components::TopDogBubbleTag>(parentShape)) {
				return depth;
			}
			middle::Id parentId = middle::getParent(gameState, currentId);
			if (parentId.index == middle::UNASSIGNED) {
				return depth;
			}

			++depth;

			parents.push(parentId);
		}
		assert(false && "top dog not found");
	}

	int findBubbleDepth(middle::GameState* gameState, middle::Id id)
	{
		std::stack < middle::Id> parents;
		parents.push(id);
		int depth = 0;
		while (parents.size() > 0) {
			middle::Id currentId = parents.top();
			parents.pop();

			auto& parentShape = middle::getShape(gameState, currentId.index);
			if (middle::getComponent<components::InputVariable>(parentShape)) {
				return depth - 1;
			}
			if (middle::getComponent<components::TopDogBubbleTag>(parentShape)) {
				return depth;
			}
			middle::Id parentId = middle::getParent(gameState, currentId);
			if (parentId.index == middle::UNASSIGNED) {
				return depth;
			}

			auto type = getStructureType(gameState, id);
			if (middle::getComponent<components::BubbleComponent>(parentShape) || middle::getComponent<components::BubbleUnit>(parentShape)) {
				++depth;
			}

			parents.push(parentId);
		}
		assert(false && "top dog not found");
	}



	bool isBubbleWithValueOne(middle::GameState* gameState, middle::Id id)
	{
		auto& shape = middle::getShape(gameState, id.index);
		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		if (!bubble) {
			return false;
		}
		auto loop = middle::getComponent<components::LoopSociety>(shape);
		if (loop->loopMemberIds.size() != 1) {
			return false;
		}
		auto& firstChild = middle::getShape(gameState, loop->loopMemberIds[0].index);
		auto variable = middle::getComponent<components::BubbleVariable>(firstChild);
		if (variable) {
			return false;
		}
		auto unit = middle::getComponent<components::BubbleUnit>(firstChild);
		if (unit) {
			return unit->value == 1;
		}
		return false;
	}

	bool isBubbleWithValueOneNegative(middle::GameState* gameState, middle::Id id)
	{
		auto& shape = middle::getShape(gameState, id.index);
		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		if (!bubble) {
			return false;
		}
		auto loop = middle::getComponent<components::LoopSociety>(shape);
		if (loop->loopMemberIds.size() != 1) {
			return false;
		}
		auto& firstChild = middle::getShape(gameState, loop->loopMemberIds[0].index);
		auto variable = middle::getComponent<components::BubbleVariable>(firstChild);
		if (variable) {
			return false;
		}
		auto unit = middle::getComponent<components::BubbleUnit>(firstChild);
		if (unit) {
			return unit->value == -1;
		}
		return false;
	}

	middle::Id bubbleToStructure(middle::GameState* gameState, middle::Id bubbleId)
	{
		// create root

		// push id and root structure to stacks
		std::stack<middle::Id>realBubbleStack;
		realBubbleStack.push(bubbleId);
		std::vector<middle::Id>newNodes;
		std::unordered_map<int, middle::Id>parentMap;

		// iterate bubble tree downward and build algebra structure
		while (realBubbleStack.size() > 0) {
			middle::Id realBubbleId = realBubbleStack.top();
			realBubbleStack.pop();

			auto& realBubbleShape = middle::getShape(gameState, realBubbleId.index);

			middle::Shape nodeShapeProto;
			auto node = middle::addComponent<components::AlgebraNode>(nodeShapeProto);
			auto nodeLoop = middle::addComponent<components::LoopSociety>(nodeShapeProto);
			node->type = static_cast<int>(getStructureType(gameState, realBubbleId));
			middle::Shape& nodeShape = middle::registerShape(gameState, nodeShapeProto);
			newNodes.push_back(nodeShape.id);
			parentMap[realBubbleId.index] = nodeShape.id;

			if (node->type == static_cast<int>(components::AlgebraNodeType::VARIABLE)) {
				auto varComp = middle::getComponent<components::BubbleVariable>(realBubbleShape);
				node->value = varComp->isNegative ? -1 : 1;
				node->variableLabel = varComp->label;
				node->isNegative = varComp->isNegative;
			}
			if (node->type == static_cast<int>(components::AlgebraNodeType::UNIT)) {
				UnitValue value = unitValue(gameState, realBubbleId);
				node->value = value.scale;
			}

			nodeLoop = middle::getComponent<components::LoopSociety>(nodeShape);

			// reparent
			if (newNodes.size() > 1) {
				middle::Id realParentId = middle::getParent(gameState, realBubbleId);
				middle::Id nodeParentId = parentMap[realParentId.index];
				nodeLoop->parentLoopId = nodeParentId;
				auto& nodeParentShape = middle::getShape(gameState, nodeParentId.index);
				auto nodeParentLoop = middle::getComponent<components::LoopSociety>(nodeParentShape);
				nodeParentLoop->loopMemberIds.push_back(nodeShape.id);
			}

			std::vector<middle::Id>realChildren;
			middle::getChildren(gameState, realBubbleId, realChildren);

			for (int i = 0; i < realChildren.size(); ++i) {
				middle::Id realChildId = realChildren[i];
				auto& realChildShape = middle::getShape(gameState, realChildId.index);
				realBubbleStack.push(realChildId);
			}
		}

		return newNodes[0];
	}

	void bubbleToStructureBranch(middle::GameState* gameState, middle::Id startPointBubbleId, middle::Id bubbleRootId, middle::Id& startPointNodeId, middle::Id& rootNodeId)
	{
		// push id and root structure to stacks
		std::stack<middle::Id>realBubbleStack;
		realBubbleStack.push(bubbleRootId);
		std::vector<middle::Id>newNodes;
		std::unordered_map<int, middle::Id>parentMap;

		// iterate bubble tree downward and build algebra structure
		while (realBubbleStack.size() > 0) {
			middle::Id realBubbleStackTopId = realBubbleStack.top();
			realBubbleStack.pop();

			auto& realBubbleShape = middle::getShape(gameState, realBubbleStackTopId.index);

			middle::Shape nodeShapeProto;
			auto node = middle::addComponent<components::AlgebraNode>(nodeShapeProto);
			auto nodeLoop = middle::addComponent<components::LoopSociety>(nodeShapeProto);
			node->type = static_cast<int>(getStructureType(gameState, realBubbleStackTopId));

			middle::Shape& newNodeShape = middle::registerShape(gameState, nodeShapeProto);
			newNodes.push_back(newNodeShape.id);
			parentMap[realBubbleStackTopId.index] = newNodeShape.id;

			if (realBubbleStackTopId == startPointBubbleId) {
				startPointNodeId = newNodeShape.id;
			}
			if (realBubbleStackTopId == bubbleRootId) {
				rootNodeId = newNodeShape.id;
			}

			if (node->type == static_cast<int>(components::AlgebraNodeType::VARIABLE)) {
				auto varComp = middle::getComponent<components::BubbleVariable>(realBubbleShape);
				node->value = varComp->isNegative ? -1 : 1;
				node->variableLabel = varComp->label;
				node->isNegative = varComp->isNegative;
			}
			if (node->type == static_cast<int>(components::AlgebraNodeType::UNIT)) {
				UnitValue value = unitValue(gameState, realBubbleStackTopId);
				node->value = value.scale;
			}

			nodeLoop = middle::getComponent<components::LoopSociety>(newNodeShape);

			// reparent
			if (newNodes.size() > 1) {
				middle::Id realParentId = middle::getParent(gameState, realBubbleStackTopId);
				middle::Id nodeParentId = parentMap[realParentId.index];
				nodeLoop->parentLoopId = nodeParentId;
				auto& nodeParentShape = middle::getShape(gameState, nodeParentId.index);
				auto nodeParentLoop = middle::getComponent<components::LoopSociety>(nodeParentShape);
				nodeParentLoop->loopMemberIds.push_back(newNodeShape.id);
			}

			std::vector<middle::Id>realChildren;
			middle::getChildren(gameState, realBubbleStackTopId, realChildren);

			for (int i = 0; i < realChildren.size(); ++i) {
				middle::Id realChildId = realChildren[i];
				auto& realChildShape = middle::getShape(gameState, realChildId.index);
				realBubbleStack.push(realChildId);
			}
		}
	}



	middle::Shape newBubble(middle::GameState* gameState, const Vector3& targetPos) {
		middle::Shape newBubbleShape;
		middle::addComponent<components::BubbleComponent>(newBubbleShape);
		middle::addComponent<components::MouseGrabbable>(newBubbleShape);
		middle::addComponent<components::MouseSelectable>(newBubbleShape);
		middle::addComponent<components::MouseIntersectable>(newBubbleShape);
		middle::addComponent<components::LoopTag>(newBubbleShape);
		middle::addComponent<components::LoopSociety>(newBubbleShape);
		middle::addComponent<components::PhysicsData>(newBubbleShape);
		middle::addComponent<components::Layer>(newBubbleShape);
		auto circle = middle::addComponent<components::Circle>(newBubbleShape);
		circle->radius = variableRadius;
		auto position = middle::addComponent<components::LocalPosition>(newBubbleShape);
		position->pos = targetPos;
		middle::addComponent<components::LocalScale>(newBubbleShape);
		middle::addComponent<components::GlobalTransform>(newBubbleShape);
		return newBubbleShape;
	}

	middle::Shape newUnit(middle::GameState* gameState, const Vector3& targetPos, bool isNegative)
	{
		middle::Shape newUnitShape;
		auto unit = middle::addComponent<components::BubbleUnit>(newUnitShape);
		unit->value = isNegative ? -1 : 1;
		middle::addComponent<components::BubbleComponent>(newUnitShape);
		middle::addComponent<components::MouseGrabbable>(newUnitShape);
		middle::addComponent<components::MouseIntersectable>(newUnitShape);
		middle::addComponent<components::MouseSelectable>(newUnitShape);
		middle::addComponent<components::LoopSociety>(newUnitShape);
		middle::addComponent<components::PhysicsData>(newUnitShape);
		middle::addComponent<components::Layer>(newUnitShape);
		middle::addComponent<components::Circle>(newUnitShape);
		auto circle = middle::addComponent<components::Circle>(newUnitShape);
		circle->radius = unitRadius;
		auto sphere = middle::addComponent<components::Sphere>(newUnitShape);
		sphere->radius = unitRadius;
		auto position = middle::addComponent<components::Position>(newUnitShape);
		position->posX = targetPos.x;
		position->posY = targetPos.y;
		position->posZ = targetPos.z;
		return newUnitShape;
	}

	middle::Shape newVariable(middle::GameState* gameState, const std::string& label, const Vector3& targetPos, bool isNegative)
	{
		middle::Shape variableProto = newBubble(gameState, targetPos);
		auto varComp = middle::addComponent<components::BubbleVariable>(variableProto);
		varComp->label = label;
		varComp->isNegative = isNegative;
		auto circle = middle::getComponent<components::Circle>(variableProto);
		circle->radius = variableRadius;
		return variableProto;
	}

	middle::Shape newExponent(middle::GameState* gameState, const Vector3& targetPos)
	{
		middle::Shape bubble = newBubble(gameState, targetPos);
		middle::addComponent<components::ExponentComponent>(bubble);
		return bubble;
	}

	middle::Shape newEquals(middle::GameState* gameState, const Vector3& targetPos)
	{
		middle::Shape newBubbleShape;
		middle::addComponent<components::BubbleEqualsComponent>(newBubbleShape);
		middle::addComponent<components::MouseGrabbable>(newBubbleShape);
		middle::addComponent<components::MouseSelectable>(newBubbleShape);
		middle::addComponent<components::MouseIntersectable>(newBubbleShape);
		middle::addComponent<components::LoopTag>(newBubbleShape);
		middle::addComponent<components::LoopSociety>(newBubbleShape);
		auto position = middle::addComponent<components::Position>(newBubbleShape);
		position->posX = targetPos.x;
		position->posY = targetPos.y;
		position->posZ = targetPos.z;
		return newBubbleShape;
	}

	middle::Shape newMultiplication(middle::GameState* gameState, const Vector3& targetPos)
	{
		middle::Shape newBubbleShape;
		auto mulComp =middle::addComponent<components::BubbleMultiplyComponent>(newBubbleShape);
		mulComp->operationType = components::OperationType::MULTIPLICATION;
		middle::addComponent<components::MouseGrabbable>(newBubbleShape);
		middle::addComponent<components::MouseSelectable>(newBubbleShape);
		middle::addComponent<components::MouseIntersectable>(newBubbleShape);
		middle::addComponent<components::LoopTag>(newBubbleShape);
		middle::addComponent<components::LoopSociety>(newBubbleShape);
		auto position = middle::addComponent<components::Position>(newBubbleShape);
		position->posX = targetPos.x;
		position->posY = targetPos.y;
		position->posZ = targetPos.z;
		return newBubbleShape;
	}

	middle::Shape newPower(middle::GameState* gameState, const Vector3& targetPos)
	{
		middle::Shape newBubbleShape;
		auto mulComp =middle::addComponent<components::BubbleMultiplyComponent>(newBubbleShape);
		mulComp->operationType = components::OperationType::POWER;
		middle::addComponent<components::MouseGrabbable>(newBubbleShape);
		middle::addComponent<components::MouseSelectable>(newBubbleShape);
		middle::addComponent<components::MouseIntersectable>(newBubbleShape);
		middle::addComponent<components::LoopTag>(newBubbleShape);
		middle::addComponent<components::LoopSociety>(newBubbleShape);
		auto position = middle::addComponent<components::Position>(newBubbleShape);
		position->posX = targetPos.x;
		position->posY = targetPos.y;
		position->posZ = targetPos.z;
		return newBubbleShape;
	}

	middle::Id newBubbleWithIntValue(middle::GameState* gameState, int value, const Vector3& targetPos)
	{
		int s = std::abs(value);
		middle::Id containerId;
		if (s > 1) {
			middle::Shape bubbleProto = newBubble(gameState, targetPos);
			middle::Shape& bubbleShape = middle::registerShape(gameState, bubbleProto);
			containerId = bubbleShape.id;
		}
		bool isNegative = value < 0;
		for (int i = 0; i < s; ++i) {
			middle::Shape unitProto = newUnit(gameState, targetPos + Vector3{ i * 0.1f, 0,0 }, isNegative);
			middle::Shape& unitShape = middle::registerShape(gameState, unitProto);
			if (s > 1) {
				middle::EditorActionReparent(containerId.index, unitShape.id.index).execute(gameState);
			}
			else {
				containerId = unitShape.id;
			}
		}
		return containerId;
	}

	middle::Id newFraction(middle::GameState* gameState, const Vector3& targetPos, int dividend)
	{
		middle::Shape newFractionProto;
		middle::addComponent<components::FractionalComponent>(newFractionProto);
		auto loop = middle::addComponent<components::LoopSociety>(newFractionProto);
		middle::addComponent<components::LoopTag>(newFractionProto);
		middle::addComponent<components::MouseGrabbable>(newFractionProto);
		middle::addComponent<components::MouseIntersectable>(newFractionProto);
		middle::addComponent<components::MouseSelectable>(newFractionProto);
		auto position = middle::addComponent<components::Position>(newFractionProto);

		auto& newFractionShape = middle::registerShape(gameState, newFractionProto);

		position->posX = targetPos.x;
		position->posY = targetPos.y;
		position->posZ = targetPos.z;

		for (int i = 0; i < dividend; ++i) {
			auto newUnitProto = newUnit(gameState, targetPos);
			auto& newUnitShape = middle::registerShape(gameState, newUnitProto);

			auto unitComp = middle::getComponent<components::BubbleUnit>(newUnitShape);
			// set everything other than bottom one as 0
			if (i < dividend - 1) {
				unitComp->value = 0;
			}
			else {
				unitComp->value = 1;
			}

			auto reparent = middle::EditorActionReparent(newFractionShape.id.index, newUnitShape.id.index);
			reparent.execute(gameState);
		}
		return newFractionShape.id;
	}

	middle::Id fractionQuotient(middle::GameState* gameState, middle::Id& fractionId) {
		std::vector<middle::Id> fractionChildren;
		auto& fractionShape = middle::getShape(gameState, fractionId.index);
		assert(middle::getComponent<components::FractionalComponent>(fractionShape));
		middle::getChildren(gameState, fractionId, fractionChildren);
		for (middle::Id& fractionPartId : fractionChildren) {
			auto& unitShape = middle::getShape(gameState, fractionPartId.index);
			auto unit = middle::getComponent<components::BubbleUnit>(unitShape);
			if (!unit || unit->value != 0) {
				return fractionPartId;
			}
		}
		assert(false);
	}

	middle::Id containerize(middle::GameState* gameState, middle::Id id)
	{
		Vector3 targetPos = middle::getShapePosition(gameState, id.index);
		middle::Shape bubbleProto = newBubble(gameState, targetPos);
		middle::Shape& newParent = middle::registerShape(gameState, bubbleProto);
		middle::EditorActionReparent(newParent.id.index, id.index).execute(gameState);
		return newParent.id;
	}


	middle::Id shapeToFraction(middle::GameState* gameState, middle::Id shapeId, const Vector3& targetPos, int dividend)
	{
		auto& shape = middle::getShape(gameState, shapeId.index);
		auto fraction = middle::getComponent<components::FractionalComponent>(shape);
		auto multiplication = middle::getComponent<components::BubbleMultiplyComponent>(shape);
		auto exponent = middle::getComponent<components::ExponentComponent>(shape);

		if (fraction) {
			int fractionSize = bubble::fractionUnitCount(gameState, shapeId);
			dividend *= fractionSize;
			middle::Id fractionShapeId = bubble::newFraction(gameState, targetPos, dividend);
			middle::Id oldQuotientId = bubble::fractionQuotient(gameState, shapeId);
			middle::Id genericQuotientId = bubble::fractionQuotient(gameState, fractionShapeId);
			middle::Id quotientCopyId = middle::deepCopyShape(gameState, oldQuotientId.index);
			bubbleActions::Replace(genericQuotientId, quotientCopyId).execute(gameState);
			return fractionShapeId;
		}
		// multiplication or root needs to be contained in a bubble
		else if (multiplication || exponent) {
			middle::Id fractionShapeId = bubble::newFraction(gameState, targetPos, dividend);
			middle::Id genericQuotientId = bubble::fractionQuotient(gameState, fractionShapeId);
			middle::Shape newContainerBubbleProto = bubble::newBubble(gameState, targetPos);
			middle::Shape& newContainerBubble = middle::registerShape(gameState, newContainerBubbleProto);
			middle::Id copyMultiplicationId = middle::deepCopyShape(gameState, shapeId.index);
			middle::EditorActionReparent(newContainerBubble.id.index, copyMultiplicationId.index).execute(gameState);
			bubbleActions::Replace(genericQuotientId, newContainerBubble.id).execute(gameState);
			return fractionShapeId;
		}
		// other cases
		else {
			middle::Id shapeCopyId = middle::deepCopyShape(gameState, shapeId.index);
			middle::Id fractionShapeId = bubble::newFraction(gameState, targetPos, dividend);
			middle::Id genericQuotientId = bubble::fractionQuotient(gameState, fractionShapeId);
			bubbleActions::Replace(genericQuotientId, shapeCopyId).execute(gameState);
			return fractionShapeId;
		}
	}




}
