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

			std::vector<Vector3>fieldPositions = getFieldPositions(gameState, shape);

			Vector3 bubbleCenter = { bubble->centerX, bubble->centerY, bubble->centerZ };

			std::vector<middle::Id>& outlineNodes = bubble->outline;

			for (middle::Id& id : outlineNodes) {
				middle::Shape& node = middle::getShape(gameState, id.index);
				auto position = middle::getComponent<components::Position>(node);
				assert(position);

				const float maxDist = bubble->length * 10;

				float maxDistSqr = maxDist * maxDist;
				const float maxForce = 20000;

				Vector3 nodePos = { position->posX, position->posY, position->posZ };

				for (Vector3& fieldPos : fieldPositions) {
					float sqrDistance = Vector3DistanceSqr(nodePos, fieldPos);

					Vector3 axisFromCenter = Vector3Normalize(nodePos - bubbleCenter);
					Vector3 axis = Vector3Normalize(nodePos - fieldPos);

					if (Vector3DotProduct(axisFromCenter, axis) < 0) {
						sqrDistance = 0.1f;
						axis = axisFromCenter;
					}

					if (sqrDistance > maxDistSqr) {
						continue;
					}

					float distance = std::sqrt(sqrDistance);
					float error = maxDist - distance;
					float normalizedError = error / maxDist;

					float strengthRatio = (std::powf(normalizedError, 90) / 1.0f);

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
