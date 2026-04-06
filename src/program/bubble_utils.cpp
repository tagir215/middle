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
		Vector3 center = { bubbleComponent->centerX, bubbleComponent->centerY, bubbleComponent->centerZ };

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
		auto& shape = middle::getShape(gameState, id.index);
		auto loop = middle::getComponent<components::LoopSociety>(shape);
		int dividend = loop->loopMemberIds.size();
		if (dividend < 2) {
			return id;
		}
		Vector3 pos = middle::getShapePosition(gameState, id.index);
		middle::Id fractionId = shapeToFraction(gameState, id, pos, dividend);
		middle::Shape bubbleProto = newBubble(gameState, pos);
		middle::Shape& bubble = middle::registerShape(gameState, bubbleProto);
		middle::EditorActionReparent(bubble.id.index, fractionId.index).execute(gameState);
		return bubble.id;
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
		BubbleValue valueA = unitValue(gameState, idA);
		BubbleValue valueB = unitValue(gameState, idB);
		const float epsilon = 1e-4f;
		bool equalMagnitude = std::abs(valueA.scale - valueB.scale) < epsilon;
		bool equalLabel = valueA.variableLabel == valueB.variableLabel;
		return equalMagnitude && equalLabel;
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


	BubbleValue unitValue(middle::GameState* gameState, middle::Id& containerId)
	{
		BubbleValue result;
		middle::Shape& shape = middle::getShape(gameState, containerId.index);
		auto unit = middle::getComponent<components::BubbleUnit>(shape);
		auto node = middle::getComponent<components::AlgebraNode>(shape);
		if (unit) {
			result.scale = unit->value;
			auto variable = middle::getComponent<components::BubbleVariable>(shape);
			if (variable) {
				result.variableLabel = variable->label;
			}
		}
		if (node) {
			result.scale = node->value;
			result.variableLabel = node->variableLabel;
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

	bool matchingBubbles(middle::GameState* gameState, middle::Id& idA, middle::Id idB) {
		auto& shapeA = middle::getShape(gameState, idA.index);
		auto& shapeB = middle::getShape(gameState, idB.index);
		auto unitA = middle::getComponent<components::BubbleUnit>(shapeA);
		auto unitB = middle::getComponent<components::BubbleUnit>(shapeB);
		auto rootA = middle::getComponent<components::ExponentComponent>(shapeA);
		auto rootB = middle::getComponent<components::ExponentComponent>(shapeA);
		auto nodeA = middle::getComponent<components::AlgebraNode>(shapeA);
		// idB is allowed to be AlgebraNode, but not idA
		assert(!nodeA);

		// if either is root return false if not equaling
		if (rootA || rootB) {
			if (!exponentEquals(gameState, idA, idB)) {
				return false;
			}
		}

		// check that units equal
		if (unitA && unitB) {
			return unitEquals(gameState, idA, idB);
		}

		// if both are non units, so are some kind of containers
		if (!unitA && !unitB) {
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

	bool matchesStructureWithVariables(middle::GameState* gameState, middle::Id bubbleId, middle::Id algebraNodeId) {
		auto algebraNodeType = getStructureType(gameState, algebraNodeId);
		auto bubbleType = getStructureType(gameState, bubbleId);
		bool aUnit = algebraNodeType == components::AlgebraNodeType::UNIT;
		bool bUnit = bubbleType == components::AlgebraNodeType::UNIT;

		if (aUnit && bUnit) {
			return unitEquals(gameState, bubbleId, algebraNodeId);
		}

		// if node type is variable and bubble is variable return true if both have same label, otherwise if algebra node is variable bubble can be anything
		if (algebraNodeType == components::AlgebraNodeType::VARIABLE) {
			if (bubbleType == components::AlgebraNodeType::VARIABLE) {
				return unitEquals(gameState, bubbleId, algebraNodeId);
			}
			return true;
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
					if (matchesStructureWithVariables(gameState, id, childrenB[i])) {
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

	middle::Id findMatchingStructureWithVariables(middle::GameState* gameState, middle::Id containerId, middle::Id algebraNodeId, int targetDepth, std::set<int>ignoreSet)
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
				if (matchesStructureWithVariables(gameState, currentId, algebraNodeId)) {
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

	middle::Id findMatchingStructureWithVariablesFromSibling(middle::GameState* gameState, middle::Id siblingId, middle::Id algebraNodeId)
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
			if (matchesStructureWithVariables(gameState, id, algebraNodeId)) {
				return id;
			}
		}
		return middle::Id();
	}

	void findMatchingStructurePairWithVariables(middle::GameState* gameState, middle::Id containerId, middle::Id algebraNodeIdA, middle::Id algebraNodeIdB, int targetDepth, middle::Id& resultIdA, middle::Id& resultIdB)
	{
		std::set<int>ignoreSet;
		while (true) {
			middle::Id idACandidate = findMatchingStructureWithVariables(gameState, containerId, algebraNodeIdA, targetDepth, ignoreSet);
			if (idACandidate.index == middle::UNASSIGNED) {
				return;
			}
			middle::Id idBCandidate = findMatchingStructureWithVariablesFromSibling(gameState, idACandidate, algebraNodeIdB);
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
		if (middle::getComponent<components::BubbleComponent>(shape)) {
			return components::AlgebraNodeType::BUBBLE;
		}
		if (middle::getComponent<components::BubbleVariable>(shape)) {
			return components::AlgebraNodeType::VARIABLE;
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

			++depth;

			middle::Id parentId = middle::getParent(gameState, currentId);

			if (parentId.index == middle::UNASSIGNED) {
				return depth;
			}

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
			auto& parentShape = middle::getShape(gameState, currentId.index);
			if (middle::getComponent<components::TopDogBubbleTag>(parentShape)) {
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
		middle::Shape rootNodeShapeProto;
		auto rootNode = middle::addComponent<components::AlgebraNode>(rootNodeShapeProto);
		middle::addComponent<components::LoopSociety>(rootNodeShapeProto);
		rootNode->type = static_cast<int>(components::AlgebraNodeType::BUBBLE);
		middle::Shape& rootNodeShape = middle::registerShape(gameState, rootNodeShapeProto);

		// push id and root structure to stacks
		std::stack<middle::Id>realBubbleStack;
		realBubbleStack.push(bubbleId);
		std::stack<middle::Id>nodeStack;
		nodeStack.push(rootNodeShape.id);

		// iterate bubble tree downward and build algebra structure
		while (realBubbleStack.size() > 0) {
			middle::Id realBubbleId = realBubbleStack.top();
			realBubbleStack.pop();
			middle::Id currentNodeId = nodeStack.top();
			nodeStack.pop();

			std::vector<middle::Id>realChildren;
			middle::getChildren(gameState, realBubbleId, realChildren);

			auto& currentNodeShape = middle::getShape(gameState, currentNodeId.index);
			auto currentNode = middle::getComponent<components::AlgebraNode>(currentNodeShape);
			auto nodeLoop = middle::getComponent<components::LoopSociety>(currentNodeShape);
			nodeLoop->loopMemberIds.resize(realChildren.size());

			for (int i = 0; i < realChildren.size(); ++i) {
				middle::Id realChildId = realChildren[i];
				auto& realChildShape = middle::getShape(gameState, realChildId.index);
				middle::Shape nodeShapeProto;
				auto node = middle::addComponent<components::AlgebraNode>(nodeShapeProto);
				middle::addComponent<components::LoopSociety>(nodeShapeProto);
				node->type = static_cast<int>(getStructureType(gameState, realChildId));
				middle::Shape& nodeShape = middle::registerShape(gameState, nodeShapeProto);

				if (node->type == static_cast<int>(components::AlgebraNodeType::UNIT) || node->type == static_cast<int>(components::AlgebraNodeType::VARIABLE)) {
					BubbleValue value = unitValue(gameState, realChildId);
					node->value = value.scale;
					node->variableLabel = value.variableLabel;
				}

				// refresh pointer
				nodeLoop = middle::getComponent<components::LoopSociety>(currentNodeShape);
				auto currentLoop = middle::getComponent<components::LoopSociety>(nodeShape);
				nodeLoop->loopMemberIds[i] = nodeShape.id;
				currentLoop->parentLoopId = currentNodeShape.id;

				realBubbleStack.push(realChildId);
				nodeStack.push(nodeShape.id);
			}
		}

		return rootNodeShape.id;
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
		middle::Shape variableProto = newUnit(gameState, targetPos);
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
