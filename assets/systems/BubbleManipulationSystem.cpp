#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "BubbleComponent.h"
#include "MouseGrabbable.h"
#include "Position.h"
#include "PhysicsData.h"

class BubbleManipulationSystem : public middle::MiddleGameplaySystem {

	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {

			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			if (!bubble)
				return;

			auto grabbable = middle::getComponent<components::MouseGrabbable>(shape);
			assert(grabbable);

			std::vector<middle::Id>& outline = bubble->outline;

			Vector3 outlineCentroid = {0,0,0};
			for (auto& id : outline) {
				auto& node = middle::getShape(gameState, id.index);
				auto position = middle::getComponent<components::Position>(node);
				outlineCentroid += {position->posX, position->posY, position->posZ};
			}
			outlineCentroid = outlineCentroid / outline.size();

			Vector3 bubbleCenter = { bubble->centerX, bubble->centerY, bubble->centerZ };
			Vector3 displacement = bubbleCenter - outlineCentroid;

			for (auto& id : outline) {
				auto& node = middle::getShape(gameState, id.index);
				auto position = middle::getComponent<components::Position>(node);
				position->posX += displacement.x;
				position->posY += displacement.y;
				position->posZ += displacement.z;
			}


			});
	}

};

static middle::SystemRegistrar<BubbleManipulationSystem> reg("BubbleManipulationSystem");
