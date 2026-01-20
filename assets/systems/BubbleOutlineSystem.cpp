#pragma once
#include "game_state.h"
#include "registrars.h"
#include "middle_shape_utils.h"
#include "LoopSociety.h"
#include "LoopTag.h"
#include "BubbleComponent.h"

class BubbleOutlineSystem : public middle::MiddleGameplaySystem {

	void populateWithChildren(
		middle::GameState* gameState, std::vector<middle::Id>* shapeList, middle::Id& id) {
		auto& shape = middle::getShape(gameState, id.index);
		auto loopSociety = middle::getComponent<components::LoopSociety>(shape);
		assert(loopSociety);

		for (middle::Id& childId : loopSociety->loopMemberIds) {
			middle::Shape& childShape = middle::getShape(gameState, childId.index);
			auto childLoopSociety = middle::getComponent<components::LoopSociety>(childShape);
			if (childLoopSociety->loopMemberIds.size() == 0) {
				shapeList->push_back(childId);
			}
			else {
				populateWithChildren(gameState, shapeList, childId);
			}
		}
	}

	struct LongestDistanceCouple {
		middle::Id idA;
		middle::Id idB;
		Vector3 posA;
		Vector3 posB;
		float distanceSqr = -1;
		Vector3 axis;
		bool initialized = false;
	};

	LongestDistanceCouple findPointsWithLongestDistanceBetween(middle::GameState* gameState, 
		std::vector<middle::Id>& ids) {
		LongestDistanceCouple result;
		for (int i = 0; i < ids.size(); ++i) {
			for (int j = i; j < ids.size(); ++j){
				middle::Id idA = ids[i];
				middle::Id idB = ids[j];
				auto& shapeA = middle::getShape(gameState, idA.index);
				auto& shapeB = middle::getShape(gameState, idB.index);
				Vector3 posA = middle::getShapePosition(gameState, idA.index);
				Vector3 posB = middle::getShapePosition(gameState, idB.index);
				float distSqr = Vector3DistanceSqr(posA, posB);
				if (distSqr > result.distanceSqr) {
					result.idA = idA;
					result.idB = idB;
					result.posA = posA;
					result.posB = posB;
					result.distanceSqr = distSqr;
					result.initialized = true;
					result.axis = posB - posA;
				}
			}
		}
		result.axis = Vector3Normalize(result.axis);
		return result;
	}

	LongestDistanceCouple coupleWithLongestDistanceAtAxis(middle::GameState* gameState, 
		std::vector<middle::Id>& ids, const Vector3& axis) {
		LongestDistanceCouple result;
		for (int i = 0; i < ids.size(); ++i) {
			for (int j = i; j < ids.size(); ++j){
				middle::Id idA = ids[i];
				middle::Id idB = ids[j];
				auto& shapeA = middle::getShape(gameState, idA.index);
				auto& shapeB = middle::getShape(gameState, idB.index);
				Vector3 posA = middle::getShapePosition(gameState, idA.index);
				Vector3 posB = middle::getShapePosition(gameState, idB.index);
				Vector3 dir = posB - posA;
				float dot = Vector3DotProduct(dir, axis);
				float distSqr = dot * dot;

				if (distSqr > result.distanceSqr) {
					result.idA = idA;
					result.idB = idB;
					result.posA = posA;
					result.posB = posB;
					result.distanceSqr = distSqr;
					result.initialized = true;
					result.axis = dir;
				}
			}
		}
		result.axis = Vector3Normalize(result.axis);
		return result;
	}

	void update(middle::GameState* gameState) override {
		std::vector<middle::Id> shapeList;
		middle::loopInstances(gameState, [gameState, &shapeList, this](int i, middle::Shape& shape) {
			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			if (!bubble)
				return;
			shapeList.clear();
			populateWithChildren(gameState, &shapeList, shape.id);

			LongestDistanceCouple distanceCouple = findPointsWithLongestDistanceBetween(gameState, shapeList);
			LongestDistanceCouple perpCouple = coupleWithLongestDistanceAtAxis(gameState, 
				shapeList, distanceCouple.axis);

			float length = std::sqrtf(distanceCouple.distanceSqr);
			float width = std::sqrtf(perpCouple.distanceSqr);
			Vector3 toCenter1 = distanceCouple.posA + Vector3Scale(distanceCouple.axis, length * 0.5f);
			Vector3 perpCoupleCentroid = Vector3Scale(perpCouple.posA + perpCouple.posB, 0.5f);
			Vector3 toPerpFromA = perpCoupleCentroid - distanceCouple.posA;
			float dot = Vector3DotProduct(toPerpFromA, perpCouple.axis);
			Vector3 toCenter2 = Vector3Scale(perpCouple.axis, dot);

			Vector3 center = distanceCouple.posA + toCenter1 + toCenter2;

			bubble->centerX = center.x;
			bubble->centerY = center.y;
			bubble->centerZ = center.z;
			bubble->axisX = distanceCouple.axis.x;
			bubble->axisY = distanceCouple.axis.y;
			bubble->axisZ = distanceCouple.axis.z;
			bubble->length = length;
			bubble->width = width;
			});
	}
};

static middle::SystemRegistrar<BubbleOutlineSystem> reg("BubbleOutlineSystem");
