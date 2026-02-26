#include "bubble_utils.h"
#include "middle_component_table.h"
#include "BubbleComponent.h"
#include "middle_shape_utils.h"
#include "LoopSociety.h"
#include "Rectangle.h"
#include "InputVariable.h"
#include "OutputVariable.h"
#include "Button.h"
#include "MouseIntersectable.h"

namespace bubble {

	bool pointIntersectBubble(middle::GameState* gameState, middle::Shape& bubbleShape, const Vector3& point)
	{
		auto bubbleComponent = middle::getComponent<components::BubbleComponent>(bubbleShape);
		assert(bubbleComponent);

		for (int i = 0; i < bubbleComponent->outlineNodes.size(); ++i) {
			int indexA = i - 1;
			int indexB = i;
			if (i == 0) {
				indexA = bubbleComponent->outlineNodes.size() - 1;
			}

			auto& idA = bubbleComponent->outlineNodes[indexA];
			auto& idB = bubbleComponent->outlineNodes[indexB];
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

}
