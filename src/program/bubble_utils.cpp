#include "bubble_utils.h"
#include "middle_component_table.h"
#include "BubbleComponent.h"
#include "middle_shape_utils.h"

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

}
