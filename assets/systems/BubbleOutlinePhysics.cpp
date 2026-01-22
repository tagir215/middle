#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "BubbleComponent.h"
#include "Position.h"
#include "PhysicsData.h"

class BubbleOutlinePhysics : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {

		std::vector<Vector3>fieldPositions;
		middle::loopInstances(gameState, [gameState, &fieldPositions](int i, middle::Shape& shape) {

			if (middle::isGhostShape(i))
				return;

			auto physics = middle::getComponent<components::PhysicsData>(shape);
			if (!physics)
				return;

			auto position = middle::getComponent<components::Position>(shape);
			assert(position);

			fieldPositions.push_back({ position->posX, position->posY, position->posZ });


			});

		middle::loopInstances(gameState, [gameState, &fieldPositions](int i, middle::Shape& shape) {

			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			if (!bubble)
				return;


			std::vector<middle::Id>& outlineNodes = bubble->outline;
			for (middle::Id& id : outlineNodes) {
				middle::Shape& node = middle::getShape(gameState, id.index);
				auto position = middle::getComponent<components::Position>(node);
				assert(position);

				const float maxDist = 100;
				float maxDistSqr = maxDist * maxDist;
				const float maxForce = 100000;

				Vector3 nodePos = { position->posX, position->posY, position->posZ };
				for (Vector3& fieldPos : fieldPositions) {
					float sqrDistance = Vector3DistanceSqr(nodePos, fieldPos);
					if (sqrDistance > maxDistSqr) {
						continue;
					}

					float distance = std::sqrt(sqrDistance);

					float error = maxDist - distance;
					float errorSqr = error * error;
					float strengthRatio = ( std::powf(errorSqr, 8) / std::powf(maxDistSqr, 8) );
					Vector3 axis = Vector3Normalize(nodePos - fieldPos);
					Vector3 force = Vector3Scale(axis, strengthRatio * maxForce);
					// remove y component for 2d game
					force.y = 0;

					//middle::RenderItem debugLine;
					//debugLine.type = middle::RenderItemType::LINE;
					//debugLine.linePointA = fieldPos;
					//debugLine.linePointB = fieldPos + force;
					//gameState->renderData.push_back(debugLine);

					auto pData = middle::getComponent<components::PhysicsData>(node);
					assert(pData);
					pData->velX += force.x * gameState->frameTime;
					pData->velY += force.y * gameState->frameTime;
					pData->velZ += force.z * gameState->frameTime;
				}

			}

			});

	}
};

static middle::SystemRegistrar<BubbleOutlinePhysics> reg("BubbleOutlinePhysics");
