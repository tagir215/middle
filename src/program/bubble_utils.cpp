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

namespace bubble {

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
		return middle::Id();
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

	std::vector<middle::Id> algebraContainerChildren(middle::Shape& shape) {
		auto loop = middle::getComponent<components::LoopSociety>(shape);
		if (loop) {
			return loop->loopMemberIds;
		}
		auto algebraNode = middle::getComponent<components::AlgebraNode>(shape);
		if (algebraNode) {
			return algebraNode->children;
		}
		assert(false);
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
		std::vector<middle::Id>childrenA = algebraContainerChildren(shapeA);
		std::vector<middle::Id>childrenB = algebraContainerChildren(shapeB);
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
		result.scale = unit->value;
		auto variable = middle::getComponent<components::BubbleVariable>(shape);
		if (variable) {
			result.variableLabel = variable->label;
		}
		return result;
	}

	int fractionUnitCount(middle::GameState* gameState, middle::Id& fractionId)
	{
		auto& shape = middle::getShape(gameState, fractionId.index);
		auto loop = middle::getComponent<components::LoopSociety>(shape);
		return loop->loopMemberIds.size();
	}

	bool matchingBubbles(middle::GameState* gameState, middle::Id& idA, middle::Id idB) {
		auto& shapeA = middle::getShape(gameState, idA.index);
		auto& shapeB = middle::getShape(gameState, idB.index);
		auto unitA = middle::getComponent<components::BubbleUnit>(shapeA);
		auto unitB = middle::getComponent<components::BubbleUnit>(shapeB);

		// check that units equal
		if (unitA && unitB) {
			return unitEquals(gameState, idA, idB);
		}

		// if both are non units, so are some kind of containers
		if (!unitA && !unitB) {
			if (!containerStructureEquals(gameState, idA, idB)) {
				return false;
			}
			std::vector<middle::Id>childrenA = algebraContainerChildren(shapeA);
			std::vector<middle::Id>childrenB = algebraContainerChildren(shapeB);
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
		// if node type is variable, bubble can be basically anything
		if (algebraNodeType == components::AlgebraNodeType::VARIABLE) {
			return true;
		}

		auto& bubbleShape = middle::getShape(gameState, bubbleId.index);
		auto& structureRootShape = middle::getShape(gameState, algebraNodeId.index);

		if (!aUnit && !bUnit) {
			// containers need to match if algebra node is a container
			if (!containerStructureEquals(gameState, bubbleId, algebraNodeId)) {
				return false;
			}
			std::vector<middle::Id>childrenA = algebraContainerChildren(bubbleShape);
			std::vector<middle::Id>childrenB = algebraContainerChildren(structureRootShape);
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

	middle::Id findMatchingStructureWithVariables(middle::GameState* gameState, middle::Id containerId, middle::Id algebraNodeId)
	{
		std::queue<middle::Id>idStack;
		idStack.push(containerId);
		while (idStack.size() > 0) {
			middle::Id currentId = idStack.front();
			idStack.pop();

			// return id of the first bubble or structure element that matches algebra node structure
			if (matchesStructureWithVariables(gameState, currentId, algebraNodeId)) {
				return currentId;
			}
			auto& shape = middle::getShape(gameState, currentId.index);
			std::vector<middle::Id>children = algebraContainerChildren(shape);
			for (middle::Id& id : children) {
				idStack.push(id);
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

	middle::Id bubbleToStructure(middle::GameState* gameState, middle::Id bubbleId)
	{
		// create root
		middle::Shape rootNodeShapeProto;
		auto rootNode = middle::addComponent<components::AlgebraNode>(rootNodeShapeProto);
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
			currentNode->children.resize(realChildren.size());

			for (int i = 0; i < realChildren.size(); ++i) {
				middle::Id realChildId = realChildren[i];
				auto& realChildShape = middle::getShape(gameState, realChildId.index);
				middle::Shape nodeShapeProto;
				auto node = middle::addComponent<components::AlgebraNode>(nodeShapeProto);
				node->type = static_cast<int>(getStructureType(gameState, realChildId));
				middle::Shape& nodeShape = middle::registerShape(gameState, nodeShapeProto);

				if (node->type == static_cast<int>(components::AlgebraNodeType::UNIT) || node->type == static_cast<int>(components::AlgebraNodeType::VARIABLE)) {
					BubbleValue value = unitValue(gameState, realChildId);
					node->value = value.scale;
					node->variableLabel = value.variableLabel;
				}

				// refresh pointer
				currentNode = middle::getComponent<components::AlgebraNode>(currentNodeShape);
				currentNode->children[i] = nodeShape.id;

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
		middle::addComponent<components::MouseIntersectable>(newBubbleShape);
		middle::addComponent<components::LoopTag>(newBubbleShape);
		middle::addComponent<components::LoopSociety>(newBubbleShape);
		middle::addComponent<components::PhysicsData>(newBubbleShape);
		auto circle = middle::addComponent<components::Circle>(newBubbleShape);
		circle->radius = 10;
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
		middle::addComponent<components::LoopSociety>(newUnitShape);
		middle::addComponent<components::PhysicsData>(newUnitShape);
		auto sphere = middle::addComponent<components::Sphere>(newUnitShape);
		sphere->radius = 2;
		auto position = middle::addComponent<components::Position>(newUnitShape);
		position->posX = targetPos.x;
		position->posY = targetPos.y;
		position->posZ = targetPos.z;
		return newUnitShape;
	}

	middle::Id newFraction(middle::GameState* gameState, const Vector3& targetPos, int dividend)
	{
		middle::Shape newFractionProto;
		middle::addComponent<components::FractionalComponent>(newFractionProto);
		auto loop = middle::addComponent<components::LoopSociety>(newFractionProto);
		middle::addComponent<components::LoopTag>(newFractionProto);
		middle::addComponent<components::MouseGrabbable>(newFractionProto);
		middle::addComponent<components::MouseIntersectable>(newFractionProto);
		auto position = middle::addComponent<components::Position>(newFractionProto);

		auto& newFractionShape = middle::registerShape(gameState, newFractionProto);

		position->posX = targetPos.x;
		position->posY = targetPos.y;
		position->posZ = targetPos.z;

		const float fractionUnitSpacing = 10;
		float height = fractionUnitSpacing * dividend - dividend;
		Vector3 referencePos = targetPos;
		referencePos.z += height * 0.5f;
		for (int i = 0; i < dividend; ++i) {
			auto newUnitProto = newUnit(gameState, referencePos);
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
			referencePos.z -= fractionUnitSpacing;
		}
		return newFractionShape.id;
	}

	middle::Id fractionQuotient(middle::GameState* gameState, middle::Id& fractionId) {
		std::vector<middle::Id> fractionChildren;
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
		// hmm...?
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
		// multiplication needs to be contained in a bubble
		else if (multiplication) {
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
