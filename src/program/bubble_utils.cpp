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

namespace bubble {
	float unitRadius = 2;
	float variableRadius = 20;
	float bubbleMinRadius = 10;
	float variableTextFontSize = 25;

	bool pointIntersectBubble(middle::GameState* gameState, middle::Shape& bubbleShape, const Vector3& point)
	{
		auto bubbleComponent = middle::getComponent<components::BubbleComponent>(bubbleShape);
		assert(bubbleComponent);
		auto ref = middle::getComponent<components::BubbleRef>(bubbleShape);
		if (!ref || ref->idRef.index == middle::UNASSIGNED) {
			return false;
		}
		Vector3 center = middle::getShapePosition(gameState, bubbleShape.id.index);

		auto& bubbleContainer = middle::getShape(gameState, ref->idRef.index);
		components::LoopSociety* loop = middle::getComponent<components::LoopSociety>(bubbleContainer);
		std::vector<middle::Id> outlineConstraints = getConstraints(gameState, loop);

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
		auto loop = middle::getComponent<components::LoopSociety>(shape);
		*leftX = 100000;
		*rightX = -100000;
		*bottomZ = *leftX;
		*topZ = *rightX;
		for (const middle::Id& childId : loop->loopMemberIds) {
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
		auto loop = middle::getComponent<components::LoopSociety>(shape);
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

		for (const middle::Id& childId : loop->loopMemberIds) {
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


	std::vector<middle::Id>getNodes(middle::GameState* gameState, components::LoopSociety* loop) {
		std::vector<middle::Id>ids;
		for (middle::Id& id : loop->loopMemberIds) {
			auto& childShape = middle::getShape(gameState, id.index);
			if (middle::getComponent<components::Sphere>(childShape)) {
				ids.push_back(id);
			}
		}
		return ids;
	}

	std::vector<middle::Id>getConstraints(middle::GameState* gameState, components::LoopSociety* loop) {
		std::vector<middle::Id>ids;
		for (middle::Id& id : loop->loopMemberIds) {
			auto& childShape = middle::getShape(gameState, id.index);
			if (middle::getComponent<components::Constraint>(childShape)) {
				ids.push_back(id);
			}
		}
		return ids;
	}

	middle::Id findBubbleWithPatern(middle::GameState* gameState, middle::Id containerBubble)
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
		auto& shape = middle::getShape(gameState, id.index);
		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		bubble->inverse = !bubble->inverse;
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
		auto fraction = middle::getComponent<components::FractionalComponent>(shape);
		auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);

		if (fraction) {
			auto loop = middle::getComponent<components::LoopSociety>(shape);
			for (middle::Id id : loop->loopMemberIds) {
				middle::Shape& shape = middle::getShape(gameState, id.index);
				if (isIntersecting(gameState, shape)) {
					return true;
				}
			}
			return false;
		}
		else if (!intersectable) {
			return false;
		}

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

	bool exponentEquals(middle::GameState* gameState, middle::Id& idA, middle::Id& idB) {
		auto& shapeA = middle::getShape(gameState, idA.index);
		auto& shapeB = middle::getShape(gameState, idB.index);
		auto rootA = middle::getComponent<components::ExponentComponent>(shapeA);
		auto rootB = middle::getComponent<components::ExponentComponent>(shapeB);
		auto nodeA = middle::getComponent<components::AlgebraNode>(shapeA);
		auto nodeB = middle::getComponent<components::AlgebraNode>(shapeB);

		// only idB is allowed to be AlgebraNode type
		assert(!nodeA);

		if (rootA && rootB) {
			return rootA->power == rootB->power
				&& rootA->isInverse == rootB->isInverse;
		}
		if (rootA && nodeB) {
			return rootA->power == nodeB->power
				&& rootA->isInverse == nodeB->isInverse;
		}
		return false;
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
			if (bubbleA->inverse != bubbleB->inverse) {
				return false;
			}
			return true;
		}
		if (bubbleA && nodeB) {
			if (varA) {
				if (varA->isNegative != nodeB->isNegative || varA->label != nodeB->variableLabel) {
					return false;
				}
			}
			if (bubbleA->inverse != nodeB->isInverse) {
				return false;
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
		auto rootA = middle::getComponent<components::ExponentComponent>(shapeA);
		auto rootB = middle::getComponent<components::ExponentComponent>(shapeB);
		auto nodeA = middle::getComponent<components::AlgebraNode>(shapeA);
		auto nodeB = middle::getComponent<components::AlgebraNode>(shapeB);
		// idB is allowed to be AlgebraNode, but not idA
		assert(!nodeA);
		auto typeA = getStructureType(gameState, idA);
		auto typeB = getStructureType(gameState, idB);

		// if one is inverse other is not return false
		if (bubbleA && bubbleB) {
			if (!bubblePropertiesEqual(gameState, idA, idB)) {
				return false;
			}
		}

		// if either is root return false if not equaling
		if (typeA == components::AlgebraNodeType::ROOT || typeB == components::AlgebraNodeType::ROOT) {
			if (!exponentEquals(gameState, idA, idB)) {
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

	bool matchesStructureWithVariables(middle::GameState* gameState, middle::Id bubbleId, middle::Id algebraNodeId, std::unordered_map<std::string, middle::Id>& varOverrides) {
		auto algebraNodeType = getStructureType(gameState, algebraNodeId);

		// replace node from algebra structure with overriding value
		if (varOverrides.size() > 0 && algebraNodeType == components::AlgebraNodeType::VARIABLE) {
			auto& nodeShape = middle::getShape(gameState, algebraNodeId.index);
			auto node = middle::getComponent<components::AlgebraNode>(nodeShape);
			assert(varOverrides.find(node->variableLabel) != varOverrides.end());
			algebraNodeId = varOverrides[node->variableLabel];
			// update node type
			algebraNodeType = getStructureType(gameState, algebraNodeId);
		}

		auto bubbleType = getStructureType(gameState, bubbleId);
		bool aUnit = algebraNodeType == components::AlgebraNodeType::UNIT;
		bool bUnit = bubbleType == components::AlgebraNodeType::UNIT;


		if (aUnit && bUnit) {
			return unitEquals(gameState, bubbleId, algebraNodeId);
		}

		// if node type is variable and bubble is variable return true if both have same label, otherwise if algebra node is variable bubble can be anything
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

		// TODO .  in future allow the same kinds
		if (uniqueSetOfMatchingStructures.size() != varMap.size()) {
			return resultMap;
		}

		assert(uniqueSetOfMatchingStructures.size() <= varMap.size());

		int totalVarCount = 0;
		for (auto& pair : varMap) {
			totalVarCount += pair.second.size();
		}

		// TODO 
		int index = 0;
		for (auto& pair : varMap) {
			auto& variableLabel = pair.first;
			resultMap[variableLabel] = uniqueSetOfMatchingStructures[index];
			++index;
		}
		if (matchesStructureWithVariables(gameState, bubbleId, algebraRootNodeId, resultMap)) {
			return resultMap;
		}

		return resultMap;
	}


	BubbleValue calculateBubbleValue(middle::GameState* gameState, middle::Id bubbleId)
	{
		BubbleValue result;

		auto& bubbleShape = middle::getShape(gameState, bubbleId.index);
		auto variable = middle::getComponent<components::BubbleVariable>(bubbleShape);
		if (variable) {
			UnitValue value;
			value.variableLabel = variable->label;
			value.scale = variable->isNegative ? -1 : 1;
			result.variableValueMap[variable->label] = value;
			result.scale = 0;
			return result;
		}

		auto node = middle::getComponent<components::AlgebraNode>(bubbleShape);
		if (node) {
			UnitValue value;
			value.scale = node->value;
			value.variableLabel = node->variableLabel;
			result.variableValueMap[value.variableLabel] = value;
			result.scale = 0;
			return result;
		}

		std::vector<middle::Id>children;
		middle::getChildren(gameState, bubbleId, children);
		for (middle::Id& id : children) {
			auto& shape = middle::getShape(gameState, id.index);
			auto unit = middle::getComponent<components::BubbleUnit>(shape);
			if (unit) {
				UnitValue value = unitValue(gameState, id);
				result.scale += value.scale;
			}
			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			if (bubble) {
				BubbleValue value = calculateBubbleValue(gameState, shape.id);
				result.scale += value.scale;
				for (auto& pair : value.variableValueMap) {
					if (result.variableValueMap.find(pair.first) == result.variableValueMap.end()) {
						result.variableValueMap[pair.first] = pair.second;
					}
					else {
						result.variableValueMap[pair.first].scale += pair.second.scale;
					}
				}

			}
			auto mul = middle::getComponent<components::BubbleMultiplyComponent>(shape);
			if (mul) {
				std::vector<middle::Id> mulChildren;
				middle::getChildren(gameState, shape.id, mulChildren);
				BubbleValue mulResult = calculateBubbleValue(gameState, mulChildren[0]);
				for (int i = 1; i < mulChildren.size(); ++i) {
					BubbleValue mulResult2 = calculateBubbleValue(gameState, mulChildren[i]);
					for (auto& pair : mulResult2.variableValueMap) {
						if (mulResult.variableValueMap.find(pair.first) == mulResult.variableValueMap.end()) {
							mulResult.variableValueMap[pair.first] = pair.second;
						}
						else {
							mulResult.variableValueMap[pair.first].scale *= pair.second.scale;
							++mulResult.variableValueMap[pair.first].power;
						}
					}
					mulResult.scale *= mulResult2.scale;
				}
				result = mulResult;
			}
		}

		auto& shape = middle::getShape(gameState, bubbleId.index);
		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		auto exp = middle::getComponent<components::ExponentComponent>(shape);
		if (bubble->inverse) {
			if (result.scale != 0) {
				result.scale = 1.0f / result.scale;
			}
			for (auto& pair : result.variableValueMap) {
				auto& val = pair.second;
				if (val.scale != 0) {
					val.scale = 1.0f / val.scale;
				}
			}
		}
		if (exp) {
			float power = exp->isInverse ? 1.0f / exp->power : exp->power;
			result.scale = std::powf(result.scale, power);
			for (auto& pair : result.variableValueMap) {
				pair.second.power *= power;
			}
		}

		return result;
	}

	void getVariableStructuresMap(middle::GameState* gameState, middle::Id structureId, std::unordered_map<std::string, std::vector<middle::Id>>& resultMap) {

		std::vector<middle::Id>children;
		middle::getChildren(gameState, structureId, children);

		for (middle::Id& childId : children) {
			if (getStructureType(gameState, childId) == components::AlgebraNodeType::VARIABLE) {
				auto& childShape = middle::getShape(gameState, childId.index);
				auto node = middle::getComponent<components::AlgebraNode>(childShape);
				if (resultMap.find(node->variableLabel) == resultMap.end()) {
					resultMap[node->variableLabel] = {};
				}
				resultMap[node->variableLabel].push_back(childId);
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

	void negate(middle::GameState* gameState, middle::Id id) {
		auto& shape = middle::getShape(gameState, id.index);
		auto unit = middle::getComponent<components::BubbleUnit>(shape);
		if (unit) {
			unit->value = -unit->value;
			return;
		}
		auto loop = middle::getComponent<components::LoopSociety>(shape);
		for (middle::Id childId : loop->loopMemberIds) {
			negate(gameState, childId);
		}
	}

	components::AlgebraNodeType getStructureType(middle::GameState* gameState, middle::Id id) {
		auto& shape = middle::getShape(gameState, id.index);
		if (middle::getComponent<components::BubbleVariable>(shape)) {
			return components::AlgebraNodeType::VARIABLE;
		}
		if (middle::getComponent<components::BubbleComponent>(shape)
			|| middle::getComponent<components::BubbleAlgebraProblem>(shape)) {
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
			if (middle::getComponent<components::TopDogBubbleTag>(parentShape)) {
				return depth;
			}
			if (middle::getComponent<components::InputVariable>(parentShape)) {
				return depth - 1;
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

	middle::Id findTopDog(middle::GameState* gameState, middle::Id id)
	{
		std::stack < middle::Id> parents;
		parents.push(id);
		while (parents.size() > 0) {
			middle::Id currentId = parents.top();
			parents.pop();
			if (currentId.index == middle::UNASSIGNED) {
				return middle::Id();
			}
			auto& parentShape = middle::getShape(gameState, currentId.index);
			if (middle::getComponent<components::TopDogBubbleTag>(parentShape)) {
				return parentShape.id;
			}
			middle::Id parentId = middle::getParent(gameState, currentId);
			parents.push(parentId);
		}
		assert(false && "top dog not found");
	}

	middle::Id findAlgebraProblem(middle::GameState* gameState, middle::Id id) {
		std::stack < middle::Id> parents;
		parents.push(id);
		while (parents.size() > 0) {
			middle::Id currentId = parents.top();
			parents.pop();
			if (currentId.index == middle::UNASSIGNED) {
				return middle::Id();
			}
			auto& parentShape = middle::getShape(gameState, currentId.index);
			if (middle::getComponent<components::BubbleAlgebraProblem>(parentShape)) {
				return parentShape.id;
			}
			middle::Id parentId = middle::getParent(gameState, currentId);
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
		circle->radius = bubbleMinRadius;
		auto position = middle::addComponent<components::Position>(newBubbleShape);
		position->posX = targetPos.x;
		position->posY = targetPos.y;
		position->posZ = targetPos.z;
		return newBubbleShape;
	}

	middle::Shape newUnit(middle::GameState* gameState, const Vector3& targetPos)
	{
		middle::Shape newUnitShape;
		middle::addComponent<components::BubbleUnit>(newUnitShape);
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

	middle::Shape newVariable(middle::GameState* gameState, const std::string& label, const Vector3& targetPos)
	{
		middle::Shape variableProto = newBubble(gameState, targetPos);
		auto varComp = middle::addComponent<components::BubbleVariable>(variableProto);
		varComp->label = label;
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

	middle::Id newEquals(middle::GameState* gameState, middle::Id bubbleAId, middle::Id bubbleBId, const Vector3& targetPos)
	{
		middle::Shape equalsProto;
		middle::addComponent<components::BubbleEqualsComponent>(equalsProto);
		auto position = middle::addComponent<components::Position>(equalsProto);
		middle::addComponent<components::MouseIntersectable>(equalsProto);
		middle::addComponent<components::MouseGrabbable>(equalsProto);
		middle::addComponent<components::MouseSelectable>(equalsProto);
		middle::addComponent<components::LoopTag>(equalsProto);
		middle::addComponent<components::LoopSociety>(equalsProto);
		position->posX = targetPos.x;
		position->posY = targetPos.y;
		position->posZ = targetPos.z;
		auto& shape = middle::registerShape(gameState, equalsProto);
		middle::EditorActionReparent(shape.id.index, bubbleAId.index).execute(gameState);
		middle::EditorActionReparent(shape.id.index, bubbleBId.index).execute(gameState);
		return shape.id;
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
