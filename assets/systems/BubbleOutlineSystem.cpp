#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "LoopSociety.h"
#include "LoopTag.h"
#include "BubbleComponent.h"
#include "Sphere.h"
#include "Position.h"
#include "Constraint.h"
#include "PhysicsData.h"

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

	middle::Shape& newNodeEntity(middle::GameState* gameState, const Vector3& pos) {
		const float r = 1;

		middle::Shape& outlineShape = middle::addGhostShape(gameState);
		auto sphere = middle::addComponent<components::Sphere>(outlineShape);
		auto posComp = middle::addComponent<components::Position>(outlineShape);
		auto physicsComp = middle::addComponent<components::PhysicsData>(outlineShape);
		posComp->posX = pos.x;
		posComp->posY = pos.y;
		posComp->posZ = pos.z;
		sphere->radius = r;
		return outlineShape;
	}

	LongestDistanceCouple findPointsWithLongestDistanceBetween(middle::GameState* gameState,
		std::vector<middle::Id>& ids) {
		LongestDistanceCouple result;
		for (int i = 0; i < ids.size(); ++i) {
			for (int j = i; j < ids.size(); ++j) {
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
		Vector3 rotateAxis = Vector3CrossProduct(axis, { 0,-1,0 });

		for (int i = 0; i < ids.size(); ++i) {
			for (int j = i; j < ids.size(); ++j) {
				middle::Id idA = ids[i];
				middle::Id idB = ids[j];
				auto& shapeA = middle::getShape(gameState, idA.index);
				auto& shapeB = middle::getShape(gameState, idB.index);
				Vector3 posA = middle::getShapePosition(gameState, idA.index);
				Vector3 posB = middle::getShapePosition(gameState, idB.index);
				Vector3 dir = posB - posA;
				float dot = Vector3DotProduct(dir, rotateAxis);
				if (dot < 0) {
					rotateAxis = Vector3Negate(rotateAxis);
				}
				float distSqr = dot * dot;

				if (distSqr > result.distanceSqr) {
					result.idA = idA;
					result.idB = idB;
					result.posA = posA;
					result.posB = posB;
					result.distanceSqr = distSqr;
					result.initialized = true;
					result.axis = rotateAxis;
				}
			}
		}
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

			Vector3 toPerp = perpCouple.posA - distanceCouple.posA;
			Vector3 perpAxis = perpCouple.axis;
			float dot = Vector3DotProduct(toPerp, perpAxis);
			Vector3 offsetSide = Vector3Scale(perpAxis, dot);
			Vector3 center = distanceCouple.posA + Vector3Scale(distanceCouple.axis, length * 0.5f) + offsetSide;
			Vector3 offsetToCenter = Vector3Scale(perpCouple.axis, width * 0.5f);
			center += offsetToCenter;

			bubble->centerX = center.x;
			bubble->centerY = center.y;
			bubble->centerZ = center.z;
			bubble->axisX = distanceCouple.axis.x;
			bubble->axisY = distanceCouple.axis.y;
			bubble->axisZ = distanceCouple.axis.z;
			bubble->length = length;
			bubble->width = width;

			bubble->aX = perpCouple.posA.x;
			bubble->aY = perpCouple.posA.y;
			bubble->aZ = perpCouple.posA.z;
			bubble->bX = perpCouple.posB.x;
			bubble->bY = perpCouple.posB.y;
			bubble->bZ = perpCouple.posB.z;


			if (bubble->outline.size() == 0) {
				const float widthMargin = 10;
				float lengthMargin = width;
				const float distBetweenNodes = 10;
				float axisLength = length - lengthMargin;
				if (axisLength < 0) axisLength = 0;
				// 2d perp for now
				Vector3 perpAxis = { -distanceCouple.axis.z, 0, distanceCouple.axis.x };

				// create first arc
				Vector3 centerLineEnd = center + Vector3Scale(distanceCouple.axis, axisLength * 0.5f);
				float r = width * 0.5f + widthMargin;
				Vector3 outlineStart = centerLineEnd + Vector3Scale(perpAxis, r);
				float circumference = 2 * PI * r + 2 * axisLength;
				int nodeCount = circumference / distBetweenNodes;
				float adjustedToEvenDistBetweenNodes = circumference / nodeCount;
				float arcLength = PI * r;
				float angleBetweenNodes = adjustedToEvenDistBetweenNodes / r;
				Vector3 dirVec = outlineStart - centerLineEnd;
				Vector3 rotateAxis = { 0,-1,0 };

				float arcTravelled = 0;
				Vector3 nextPos = { 0,0,0 };

				while (arcTravelled < arcLength) {
					nextPos = centerLineEnd + dirVec;
					middle::Shape& outlineShape = newNodeEntity(gameState, nextPos);

					dirVec = Vector3RotateByAxisAngle(dirVec, rotateAxis, -angleBetweenNodes);
					arcTravelled += angleBetweenNodes * r;

					bubble->outline.push_back(outlineShape.id);
				}

				// create first side
				float lengthTravelled = arcTravelled - arcLength;
				Vector3 transVec = Vector3Scale(distanceCouple.axis, -adjustedToEvenDistBetweenNodes);
				nextPos = centerLineEnd + Vector3Scale(perpAxis, -r);
				nextPos += Vector3Scale(distanceCouple.axis, -lengthTravelled);
				while (lengthTravelled < axisLength) {
					middle::Shape& outlineShape = newNodeEntity(gameState, nextPos);
					bubble->outline.push_back(outlineShape.id);
					lengthTravelled += adjustedToEvenDistBetweenNodes;
					nextPos = nextPos + transVec;
				}


				// create second arc
				arcTravelled = lengthTravelled - axisLength;
				centerLineEnd = center - Vector3Scale(distanceCouple.axis, axisLength * 0.5f);
				outlineStart = centerLineEnd + Vector3Scale(perpAxis, -r);
				dirVec = outlineStart - centerLineEnd;
				float alreadyRotated = arcTravelled / -r;
				dirVec = Vector3RotateByAxisAngle(dirVec, rotateAxis, alreadyRotated);
				while (arcTravelled < arcLength) {
					nextPos = centerLineEnd + dirVec;
					middle::Shape& outlineShape = newNodeEntity(gameState, nextPos);

					dirVec = Vector3RotateByAxisAngle(dirVec, rotateAxis, -angleBetweenNodes);
					arcTravelled += angleBetweenNodes * r;

					bubble->outline.push_back(outlineShape.id);
				}


				// create second side
				lengthTravelled = arcTravelled - arcLength;
				nextPos = centerLineEnd + Vector3Scale(perpAxis, r);
				nextPos += Vector3Scale(distanceCouple.axis, lengthTravelled);
				transVec = Vector3Scale(distanceCouple.axis, adjustedToEvenDistBetweenNodes);
				while (lengthTravelled < axisLength) {
					middle::Shape& outlineShape = newNodeEntity(gameState, nextPos);
					bubble->outline.push_back(outlineShape.id);
					nextPos = nextPos + transVec;
					lengthTravelled += adjustedToEvenDistBetweenNodes;
				}




				// generate constraints
				for (int i = 0; i < bubble->outline.size(); ++i) {
					int iA = i;
					int iB = i - 1;
					if (iB < 0) {
						iB = bubble->outline.size() - 1;
					}

					auto& idA = bubble->outline[iA];
					auto& idB = bubble->outline[iB];
					Vector3 posA = middle::getShapePosition(gameState, idA.index);
					Vector3 posB = middle::getShapePosition(gameState, idB.index);

					auto& constraintShape = middle::addGhostShape(gameState);
					auto constraint = middle::addComponent<components::Constraint>(constraintShape);
					constraint->targetDistance = Vector3Distance(posA, posB);
					constraint->idA = idA;
					constraint->idB = idB;
				}
			}





			});
	}
};

static middle::SystemRegistrar<BubbleOutlineSystem> reg("BubbleOutlineSystem");
