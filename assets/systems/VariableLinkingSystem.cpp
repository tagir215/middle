#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "InputVariable.h"
#include "MouseIntersectable.h"
#include "LoopSociety.h"
#include "PlacementComponent.h"

class VariableLinkingSystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {

		middle::Id& grabbedId = gameState->bubbleAlgebraState.grabbedId;

		if (!gameState->input.mouseHeld) {

			if (grabbedId.index != middle::UNASSIGNED) {
				auto& shape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);
				auto inputVariable = middle::getComponent<components::InputVariable>(shape);
				if (inputVariable) {
					middle::deleteShape(gameState, shape.id.index);
					gameState->bubbleAlgebraState.grabbedId = middle::Id();
				}
			}
			return;
		}

		if (grabbedId.index != middle::UNASSIGNED) {
			auto& shape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);
			auto inputVariable = middle::getComponent<components::InputVariable>(shape);
			if (inputVariable) {
				Vector3 targetPos = gameState->input.mouseXZ_PlanePos;
				Vector3 currentPos = middle::getShapePosition(gameState, grabbedId.index);
				middle::moveShape(gameState, grabbedId.index, targetPos - currentPos);
			}
			return;
		}

		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {
			auto inputVariable = middle::getComponent<components::InputVariable>(shape);

			if (!inputVariable)
				return;

			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			if (!intersectable)
				return;

			if (intersectable->intersecting) {
				middle::Id copyId = middle::deepCopyShape(gameState, shape.id.index, middle::UNASSIGNED);
				auto& copyShape = middle::getShape(gameState, copyId.index);
				auto copyLoop = middle::getComponent<components::LoopSociety>(copyShape);
				// set copy intersecting as false since, its copied
				auto copyIntersectable = middle::getComponent<components::MouseIntersectable>(copyShape);
				copyIntersectable->intersecting = false;
				copyIntersectable->intersectingTop = false;
				copyLoop->parentLoopId = middle::Id();
				gameState->bubbleAlgebraState.grabbedId = copyId;
			}
			});

	}
};

static middle::SystemRegistrar<VariableLinkingSystem> reg("VariableLinkingSystem");
