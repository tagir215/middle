#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "BubbleComponent.h"
#include "Position.h"
#include "PhysicsData.h"

class BubbleOutlinePhysics : public middle::MiddleGameplaySystem {

	void applyForce(middle::GameState* gameState, middle::Shape& node, const Vector3& force) {
		auto pData = middle::getComponent<components::PhysicsData>(node);
		assert(pData);
		pData->velX += force.x * gameState->frameTime;
		// no y component for 2d game
		pData->velZ += force.z * gameState->frameTime;
	}

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

		middle::loopInstances(gameState, [gameState, &fieldPositions, this](int i, middle::Shape& shape) {

			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			if (!bubble)
				return;


			Vector3 bubbleCentroid = { bubble->centerX, bubble->centerY, bubble->centerZ };

			std::vector<middle::Id>& outlineNodes = bubble->outline;

			for (middle::Id& id : outlineNodes) {
				middle::Shape& node = middle::getShape(gameState, id.index);
				auto position = middle::getComponent<components::Position>(node);
				assert(position);

				const float maxDist = bubble->length * 10;
				float maxDistEvenForce = bubble->length * 2;

				float maxDistSqr = maxDist * maxDist;
				const float maxForce = 20000;
				const float evenMaxForce = 2000;

				Vector3 nodePos = { position->posX, position->posY, position->posZ };
				Vector3 evenAxis = Vector3Normalize(nodePos - bubbleCentroid);
				float evenForceRatio = 1 - (Vector3Distance(nodePos, bubbleCentroid) / maxDistEvenForce);

				Vector3 evenForce = Vector3Scale(evenAxis, evenMaxForce * evenForceRatio);
				applyForce(gameState, node, evenForce);

				for (Vector3& fieldPos : fieldPositions) {
					float sqrDistance = Vector3DistanceSqr(nodePos, fieldPos);

					if (sqrDistance > maxDistSqr) {
						continue;
					}

					Vector3 axis = Vector3Normalize(nodePos - fieldPos);

					float distance = std::sqrt(sqrDistance);
					float error = maxDist - distance;
					float normalizedError = error / maxDist;

					float strengthRatio = (std::powf(normalizedError, 40) / 1.0f);

					Vector3 force = Vector3Scale(axis, strengthRatio * maxForce);

					//middle::RenderItem debugLine;
					//debugLine.type = middle::RenderItemType::LINE;
					//debugLine.linePointA = fieldPos;
					//debugLine.linePointB = fieldPos + force;
					//gameState->renderData.push_back(debugLine);

					applyForce(gameState, node, force);
				}

			}

			});

	}
};

static middle::SystemRegistrar<BubbleOutlinePhysics> reg("BubbleOutlinePhysics");
