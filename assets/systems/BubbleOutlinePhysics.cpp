#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "BubbleComponent.h"
#include "Position.h"
#include "PhysicsData.h"
#include "LoopSociety.h"

class BubbleOutlinePhysics : public middle::MiddleGameplaySystem {
public:
	BubbleOutlinePhysics() {
		systemModeType = middle::SystemModeType::ENGINE;
	}

	void applyForce(middle::GameState* gameState, middle::Shape& node, const Vector3& force) {
		auto pData = middle::getComponent<components::PhysicsData>(node);
		assert(pData);
		pData->velX += force.x * gameState->frameTime;
		// no y component for 2d game
		pData->velZ += force.z * gameState->frameTime;
	}

	std::vector<Vector3> getFieldPositions(middle::GameState* gameState, middle::Shape& bubble) {
		std::vector<Vector3>fieldPositions;
		std::vector<middle::Id>children;
		middle::getChildren(gameState, bubble.id, children);

		for(middle::Id& id : children){
			middle::Shape& shape = middle::getShape(gameState, id.index);

			auto physics = middle::getComponent<components::PhysicsData>(shape);
			if (!physics)
				continue;

			auto position = middle::getComponent<components::Position>(shape);
			assert(position);

			fieldPositions.push_back({ position->posX, position->posY, position->posZ });
		}
		return fieldPositions;
	}

	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {


			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			if (!bubble)
				return;

			auto bubblePosition = middle::getComponent<components::Position>(shape);
			assert(bubblePosition);

			Vector3 bubbleCenter = { bubble->centerX, bubble->centerY, bubble->centerZ };
			Vector3 bubblePos = { bubblePosition->posX, bubblePosition->posY, bubblePosition->posZ };

			// BUBBLE GRAVITY

			auto loopSociety = middle::getComponent<components::LoopSociety>(shape);

			for (middle::Id& childId : loopSociety->loopMemberIds) {
				middle::Shape& child = middle::getShape(gameState, childId.index);
				auto physics = middle::getComponent<components::PhysicsData>(child);
				if (!physics)
					continue;
				auto childPosition = middle::getComponent<components::Position>(child);
				const float gravityForce = 20.2f;
				Vector3 childPos = { childPosition->posX, childPosition->posY, childPosition->posZ };
				Vector3 gravityAxis = Vector3Normalize(bubblePos - childPos);
				if (Vector3LengthSqr(gravityAxis) == 0) {
					gravityAxis = { 1, 0, 0 };
				}
				Vector3 force = Vector3Scale(gravityAxis, gravityForce);
				//applyForce(gameState, child, force);
			}


			// OUTLINE PHYSICS

			std::vector<Vector3>fieldPositions = getFieldPositions(gameState, shape);


			std::vector<middle::Id>& outlineNodes = bubble->outline;

			for (middle::Id& id : outlineNodes) {
				middle::Shape& node = middle::getShape(gameState, id.index);
				auto position = middle::getComponent<components::Position>(node);
				auto nodePhysics = middle::getComponent<components::PhysicsData>(node);

				float magSqr = 
					nodePhysics->velX * nodePhysics->velX 
					+ nodePhysics->velY * nodePhysics->velY 
					+ nodePhysics->velZ * nodePhysics->velZ;


				nodePhysics->damX = 0.1f;
				nodePhysics->damY = 0.1f;
				nodePhysics->damZ = 0.1f;

				assert(position);

				const float maxVel = 10000;

				const float maxForce = 10000;
				float speedSqrt = Vector3LengthSqr({ nodePhysics->velX, nodePhysics->velY, nodePhysics->velZ });
				float speedRatio = speedSqrt / maxVel;

				float maxDist = bubble->length * 10;
				float maxDistSqr = maxDist * maxDist;

				Vector3 nodePos = { position->posX, position->posY, position->posZ };

				for (Vector3& fieldPos : fieldPositions) {
					float sqrDistance = Vector3DistanceSqr(nodePos, fieldPos);

					Vector3 axisFromCenter = Vector3Normalize(nodePos - bubbleCenter);
					Vector3 axis = Vector3Normalize(nodePos - fieldPos);

					if (Vector3DotProduct(axisFromCenter, axis) < 0) {
						continue;
					}

					if (sqrDistance > maxDistSqr) {
						continue;
					}

					float distance = std::sqrt(sqrDistance);
					float error = maxDist - distance;
					float normalizedError = error / maxDist;

					int power = 90;

					float strengthRatio = (std::powf(normalizedError, power) / 1.0f);

					Vector3 force = Vector3Scale(axis, strengthRatio * maxForce);

					//middle::RenderItem debugLine;
					//debugLine.type = middle::RenderItemType::LINE;
					//debugLine.linePointA = fieldPos;
					//debugLine.linePointB = fieldPos + force;
					//gameState->renderData.push_back(debugLine);

					applyForce(gameState, node, force);
				}

			}


			// translate bubble nodes toward centroid
			Vector3 outlineCentroid = { 0,0,0 };
			for (auto& id : outlineNodes) {
				auto& node = middle::getShape(gameState, id.index);
				auto position = middle::getComponent<components::Position>(node);
				outlineCentroid += {position->posX, position->posY, position->posZ};
			}
			outlineCentroid = outlineCentroid / outlineNodes.size();

			Vector3 displacement = bubbleCenter - outlineCentroid;

			for (auto& id : outlineNodes) {
				auto& node = middle::getShape(gameState, id.index);
				auto position = middle::getComponent<components::Position>(node);
				position->posX += displacement.x;
				position->posY += displacement.y;
				position->posZ += displacement.z;
			}

			});

	}
};

static middle::SystemRegistrar<BubbleOutlinePhysics> reg("BubbleOutlinePhysics");
