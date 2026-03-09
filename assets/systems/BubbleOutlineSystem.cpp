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
#include "PlacementComponent.h"
#include <random>
#include "editor_actions.h"
#include "BubbleRef.h" 
#include "DependencyComponent.h"
#include "bubble_utils.h"
#include "component_utils.h"

class BubbleOutlineSystem : public middle::MiddleGameplaySystem {
public:
	BubbleOutlineSystem(){
		systemModeType = middle::SystemModeType::ENGINE;
	}

	void init(middle::GameState* gameState) {

	}

	const float bubbleOutlineWidthMargin = 10;
	const float distBetweenNodes = 5;
	const float nodeRadius = 0.1f;

	struct LongestDistanceCouple {
		middle::Id idA;
		middle::Id idB;
		Vector3 posA;
		Vector3 posB;
		float distanceSqr = -1;
		Vector3 axis;
		bool initialized = false;
	};

	middle::Shape& newNodeShape(middle::GameState* gameState, const Vector3& pos) {

		middle::Shape& outlineShape = middle::addGhostShape(gameState);
		auto sphere = middle::attachComponent<components::Sphere>(gameState, outlineShape.id);
		auto posComp = middle::attachComponent<components::Position>(gameState, outlineShape.id);
		auto physicsComp = middle::attachComponent<components::PhysicsData>(gameState, outlineShape.id);
		auto loop = middle::attachComponent<components::LoopSociety>(gameState, outlineShape.id);
		posComp->posX = pos.x;
		posComp->posY = pos.y;
		posComp->posZ = pos.z;
		sphere->radius = nodeRadius;
		//physicsComp->damX = 0.1f;
		//physicsComp->damY = 0.1f;
		//physicsComp->damZ = 0.1f;
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

	void removeNode(middle::GameState* gameState, middle::Id& id, float distBetweenNodes) {
		auto& shape = middle::getShape(gameState, id.index);
		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		assert(bubble);
		auto bubbleRef = middle::getComponent<components::BubbleRef>(shape);
		auto bubbleContainer = middle::getShape(gameState, bubbleRef->idRef.index);
		auto loop = middle::getComponent<components::LoopSociety>(bubbleContainer);

		const int minNodeCount = 5;
		std::vector<middle::Id> outlineNodes = bubble::getNodes(gameState, loop);

		int outlineNodesSize = outlineNodes.size();
		int random = rand() % (outlineNodesSize - 2) + 1;
		const int nodeToRemoveIndex = random;
		middle::Id nodeIdToRemove = outlineNodes[nodeToRemoveIndex];

		std::vector<int>connectedIndexes = middle::findConnectedConstraints(gameState, nodeIdToRemove);

		assert(connectedIndexes.size() == 2);
		auto& constraintShapeToRemove = middle::getShape(gameState, connectedIndexes[0]);
		auto& constraintShapeToEdit = middle::getShape(gameState, connectedIndexes[1]);
		auto constraintToRemove = middle::getComponent<components::Constraint>(constraintShapeToRemove);
		auto constraintToEdit = middle::getComponent<components::Constraint>(constraintShapeToEdit);

		middle::Id nodeToReplace = constraintToRemove->idA == nodeIdToRemove ? constraintToRemove->idB : constraintToRemove->idA;
		if (constraintToEdit->idA == nodeIdToRemove) {
			constraintToEdit->idA = nodeToReplace;
		}
		if (constraintToEdit->idB == nodeIdToRemove) {
			constraintToEdit->idB = nodeToReplace;
		}

		middle::queueAction(gameState, std::make_shared<middle::EditorActionDeleteSingle>(constraintShapeToRemove.id));
		middle::queueAction(gameState, std::make_shared<middle::EditorActionDeleteSingle>(nodeIdToRemove));
	}


	void addNode(middle::GameState* gameState, middle::Id& id, float distBetweenNodes) {
		auto& shape = middle::getShape(gameState, id.index);
		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		assert(bubble);
		auto bubbleRef = middle::getComponent<components::BubbleRef>(shape);
		auto bubbleContainer = middle::getShape(gameState, bubbleRef->idRef.index);
		auto loop = middle::getComponent<components::LoopSociety>(bubbleContainer);

		// get constraint to break
		std::vector<middle::Id>outlineNodes = bubble::getNodes(gameState, loop);
		std::vector<middle::Id>outlineConstraints = bubble::getConstraints(gameState, loop);

		auto& constraintToBreakShape = middle::getShape(gameState, outlineConstraints.back().index);
		auto constraintToBreak = middle::getComponent<components::Constraint>(constraintToBreakShape);
		constraintToBreak->targetDistance = distBetweenNodes;

		middle::Shape& leftNeighborShape = middle::getShape(gameState, constraintToBreak->idA.index);
		middle::Shape& rightNeighborShape = middle::getShape(gameState, constraintToBreak->idB.index);


		// get center pos 
		Vector3 posLeft = middle::getShapePosition(gameState, leftNeighborShape.id.index);
		Vector3 posRight = middle::getShapePosition(gameState, rightNeighborShape.id.index);
		Vector3 centroid = Vector3Scale(posLeft + posRight, 0.5f);

		// create new node
		middle::Shape& newNode = newNodeShape(gameState, centroid);
		middle::EditorActionReparent(bubbleContainer.id.index, newNode.id.index).execute(gameState);


		// edit old constraint
		if (leftNeighborShape.id == constraintToBreak->idA) {
			constraintToBreak->idB = newNode.id;
		}
		else {
			constraintToBreak->idA = newNode.id;
		}

		// create new constraint
		middle::Shape& newConstraintShape = middle::addGhostShape(gameState);
		auto newConstraint = middle::attachComponent<components::Constraint>(gameState, newConstraintShape.id);
		middle::attachComponent<components::LoopSociety>(gameState, newConstraintShape.id);
		// update pointer after resizing array  
		constraintToBreak = middle::getComponent<components::Constraint>(constraintToBreakShape);

		newConstraint->idA = newNode.id;
		newConstraint->idB = rightNeighborShape.id;
		newConstraint->targetDistance = constraintToBreak->targetDistance;
		middle::EditorActionReparent(bubbleContainer.id.index, newConstraintShape.id.index).execute(gameState);

	}

	void calculateMargin(middle::GameState* gameState, middle::Id& id, float& result) {
		middle::Shape& shape = middle::getShape(gameState, id.index);
		auto loop = middle::getComponent<components::LoopSociety>(shape);
		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		assert(loop);
		assert(bubble);

		result += bubbleOutlineWidthMargin * 2;

		for (middle::Id childId : loop->loopMemberIds) {
			middle::Shape& childShape = middle::getShape(gameState, childId.index);
			auto childBubble = middle::getComponent<components::BubbleComponent>(childShape);
			if (childBubble) {
				calculateMargin(gameState, childId, result);
			}
		}

	}


	void update(middle::GameState* gameState) override {

		std::vector<middle::Id> shapeList;

		middle::loopInstances(gameState, [gameState, &shapeList, this](int i, middle::Shape& shape) {
			middle::Id shapeId = shape.id;
			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			if (!bubble)
				return true;

			shapeList.clear();
			middle::getAllChildrenWithComp(gameState, shape.id, shapeList, middle::getTypeId<components::Position>());

			LongestDistanceCouple distanceCouple;
			LongestDistanceCouple perpCouple;
			if (shapeList.size() >= 2) {
				distanceCouple = findPointsWithLongestDistanceBetween(gameState, shapeList);
				perpCouple = coupleWithLongestDistanceAtAxis(gameState, shapeList, distanceCouple.axis);
			}
			else if(shapeList.size() == 1) {
				const float minRadiusSq = 4;
				distanceCouple.axis = { 1,0,0 };
				distanceCouple.distanceSqr = minRadiusSq;
				distanceCouple.initialized = true;
				perpCouple.axis = { 0,0,1 };
				perpCouple.distanceSqr = minRadiusSq;
				perpCouple.initialized = true;
			}
			else {
				return true;
			}

			assert(distanceCouple.distanceSqr != 0);


			float length = std::sqrtf(distanceCouple.distanceSqr);
			float width = std::sqrtf(perpCouple.distanceSqr);

			Vector3 toPerp = perpCouple.posA - distanceCouple.posA;
			Vector3 perpAxis = perpCouple.axis;
			float dot = Vector3DotProduct(toPerp, perpAxis);
			Vector3 offsetSide = Vector3Scale(perpAxis, dot);
			Vector3 center = distanceCouple.posA + Vector3Scale(distanceCouple.axis, length * 0.5f) + offsetSide;
			Vector3 offsetToCenter = Vector3Scale(perpCouple.axis, width * 0.5f);
			center += offsetToCenter;

			if (shapeList.size() == 1) {
				center = middle::getShapePosition(gameState, shapeList[0].index);
			}

			assert(!std::isnan(center.x));

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

			// extra mragin from inner bubbles outlines
			float extraMargin = 0;
			calculateMargin(gameState, shape.id, extraMargin);

			const float adjustedWidth = width + extraMargin;
			const float adjustedLength = length + extraMargin;

			float lengthMargin = adjustedWidth;
			float axisLength = length + adjustedWidth - lengthMargin;
			float bubbleEndPointRadius = adjustedWidth * 0.5f;
			float circumference = 2 * PI * bubbleEndPointRadius + 2 * axisLength;
			int nodeCount = circumference / distBetweenNodes;
			const int maxNodeCount = 60;
			const int minNodeCount = 10;
			if (nodeCount > maxNodeCount) {
				nodeCount = maxNodeCount;
			}
			if (nodeCount < minNodeCount) {
				nodeCount = minNodeCount;
			}
			bubble->nodeCountTarget = nodeCount;

			bubble->distBetweenNodes = distBetweenNodes;
			bubble->endRadius = bubbleEndPointRadius;

			bool initialize = false;
			auto bubbleRef = middle::getComponent<components::BubbleRef>(shape);
			if (!bubbleRef) {
				bubbleRef = middle::attachComponent<components::BubbleRef>(gameState, shape.id);
			}

			if (bubbleRef->idRef.index == middle::UNASSIGNED) {
				initialize = true;
				middle::Shape& bubbleContainer = middle::addGhostShape(gameState);
				bubbleRef->idRef = bubbleContainer.id;
				auto dependency = middle::attachComponent<components::DependencyComponent>(gameState, bubbleContainer.id);
				dependency->idRef = shape.id;
				middle::attachComponent<components::LoopSociety>(gameState, bubbleContainer.id);
			}


			auto& outlineContainer = middle::getShape(gameState, bubbleRef->idRef.index);
			auto loop = middle::getComponent<components::LoopSociety>(outlineContainer);

			// bubble initialization
			if (initialize) {
				if (axisLength < 0) axisLength = 0;
				// 2d perp for now
				Vector3 perpAxis = { -distanceCouple.axis.z, 0, distanceCouple.axis.x };

				// create first arc
				Vector3 centerLineEnd = center + Vector3Scale(distanceCouple.axis, axisLength * 0.5f);
				Vector3 outlineStart = centerLineEnd + Vector3Scale(perpAxis, bubbleEndPointRadius);
				float adjustedToEvenDistBetweenNodes = circumference / nodeCount;


				float arcLength = PI * bubbleEndPointRadius;
				float angleBetweenNodes = bubble->distBetweenNodes / bubbleEndPointRadius;
				Vector3 dirVec = outlineStart - centerLineEnd;
				Vector3 rotateAxis = { 0,-1,0 };

				float arcTravelled = 0;
				Vector3 nextPos = { 0,0,0 };

				while (arcTravelled < arcLength) {
					nextPos = centerLineEnd + dirVec;
					middle::Shape& outlineShape = newNodeShape(gameState, nextPos);

					dirVec = Vector3RotateByAxisAngle(dirVec, rotateAxis, -angleBetweenNodes);
					arcTravelled += angleBetweenNodes * bubbleEndPointRadius;

					middle::EditorActionReparent(outlineContainer.id.index, outlineShape.id.index).execute(gameState);
				}

				// create first side
				float lengthTravelled = arcTravelled - arcLength;
				Vector3 transVec = Vector3Scale(distanceCouple.axis, -adjustedToEvenDistBetweenNodes);
				nextPos = centerLineEnd + Vector3Scale(perpAxis, -bubbleEndPointRadius);
				nextPos += Vector3Scale(distanceCouple.axis, -lengthTravelled);
				while (lengthTravelled < axisLength) {
					middle::Shape& outlineShape = newNodeShape(gameState, nextPos);
					middle::EditorActionReparent(outlineContainer.id.index, outlineShape.id.index).execute(gameState);
					lengthTravelled += adjustedToEvenDistBetweenNodes;
					nextPos = nextPos + transVec;
				}


				// create second arc
				arcTravelled = lengthTravelled - axisLength;
				centerLineEnd = center - Vector3Scale(distanceCouple.axis, axisLength * 0.5f);
				outlineStart = centerLineEnd + Vector3Scale(perpAxis, -bubbleEndPointRadius);
				dirVec = outlineStart - centerLineEnd;
				float alreadyRotated = arcTravelled / -bubbleEndPointRadius;
				dirVec = Vector3RotateByAxisAngle(dirVec, rotateAxis, alreadyRotated);
				while (arcTravelled < arcLength) {
					nextPos = centerLineEnd + dirVec;
					middle::Shape& outlineShape = newNodeShape(gameState, nextPos);

					dirVec = Vector3RotateByAxisAngle(dirVec, rotateAxis, -angleBetweenNodes);
					arcTravelled += angleBetweenNodes * bubbleEndPointRadius;
					middle::EditorActionReparent(outlineContainer.id.index, outlineShape.id.index).execute(gameState);
				}


				// create second side
				lengthTravelled = arcTravelled - arcLength;
				nextPos = centerLineEnd + Vector3Scale(perpAxis, bubbleEndPointRadius);
				nextPos += Vector3Scale(distanceCouple.axis, lengthTravelled);
				transVec = Vector3Scale(distanceCouple.axis, adjustedToEvenDistBetweenNodes);
				// stop one early to avoid duplicating the first node
				while (lengthTravelled < axisLength - adjustedToEvenDistBetweenNodes) {
					middle::Shape& outlineShape = newNodeShape(gameState, nextPos);
					middle::EditorActionReparent(outlineContainer.id.index, outlineShape.id.index).execute(gameState);
					nextPos = nextPos + transVec;
					lengthTravelled += adjustedToEvenDistBetweenNodes;
				}


				loop = middle::getComponent<components::LoopSociety>(outlineContainer);
				std::vector<middle::Id>nodes = bubble::getNodes(gameState, loop);
				// generate constraints
				for (int i = 0; i < nodes.size(); ++i) {
					int iA = i;
					int iB = i - 1;
					if (iB < 0) {
						iB = nodes.size() - 1;
					}

					auto& idA = nodes[iA];
					auto& idB = nodes[iB];
					Vector3 posA = middle::getShapePosition(gameState, idA.index);
					Vector3 posB = middle::getShapePosition(gameState, idB.index);

					auto& constraintShape = middle::addGhostShape(gameState);
					auto constraintLoop = middle::attachComponent<components::LoopSociety>(gameState, constraintShape.id);
					auto constraint = middle::attachComponent<components::Constraint>(gameState, constraintShape.id);
					constraint->targetDistance = Vector3Distance(posA, posB);
					constraint->idA = idA;
					constraint->idB = idB;
					constraint->stiffness = 1.0f;
					constraint->biasFactor = 0.2f;
					assert(constraint->targetDistance > 0);
					middle::EditorActionReparent(outlineContainer.id.index, constraintShape.id.index).execute(gameState);
				}
			}


			loop = middle::getComponent<components::LoopSociety>(outlineContainer);
			std::vector<middle::Id>nodes = bubble::getNodes(gameState, loop);

			if (nodeCount - 1 < nodes.size()) {
				removeNode(gameState, shapeId, distBetweenNodes);
			}

			if (nodeCount - 1 > nodes.size()) {
				addNode(gameState, shapeId, distBetweenNodes);
			}

			return true;
			});
	}
};

static middle::SystemRegistrar<BubbleOutlineSystem> reg("BubbleOutlineSystem");
