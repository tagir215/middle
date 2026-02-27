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
#include "IdRef.h"

namespace bubble {

	bool pointIntersectBubble(middle::GameState* gameState, middle::Shape& bubbleShape, const Vector3& point)
	{
		auto bubbleComponent = middle::getComponent<components::BubbleComponent>(bubbleShape);
		assert(bubbleComponent);
		auto ref = middle::getComponent<components::IdRef>(bubbleShape);
		if (!ref || ref->idRef.index == middle::UNASSIGNED) {
			return false;
		}

		auto& bubbleContainer = middle::getShape(gameState, ref->idRef.index);
		components::LoopSociety* loop = middle::getComponent<components::LoopSociety>(bubbleContainer);
		std::vector<middle::Id> outlineNodes = getNodes(gameState, loop);

		for (int i = 0; i < outlineNodes.size(); ++i) {
			int indexA = i - 1;
			int indexB = i;
			if (i == 0) {
				indexA = outlineNodes.size() - 1;
			}

			auto& idA = outlineNodes[indexA];
			auto& idB = outlineNodes[indexB];
			Vector3 posA = middle::getShapePosition(gameState, idA.index);
			Vector3 posB = middle::getShapePosition(gameState, idB.index);
			Vector3 dir = posB - posA;
			// 2d normal
			Vector3 normal = { -dir.z, 0 , dir.x };

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
}
