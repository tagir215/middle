#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "InputVariable.h"
#include "MouseIntersectable.h"
#include "LoopSociety.h"
#include "PlacementComponent.h"
#include "BubbleComponent.h"
#include "editor_actions.h"
#include "OutputVariable.h"
#include "bubble_actions.h"
#include "component_utils.h"

class VariableLinkingSystem : public middle::MiddleGameplaySystem {

public:
	void init(middle::GameState* gameState) {

	}

	void update(middle::GameState* gameState) override {

		middle::Id& grabbedId = gameState->bubbleAlgebraState.grabbedId;

		if (!gameState->input.mouseHeld) {

			if (grabbedId.index != middle::UNASSIGNED) {
				auto& shape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);

				auto grabbedInputVariable = middle::getComponent<components::InputVariable>(shape);
				auto grabbedOutputVariable = middle::getComponent<components::OutputVariable>(shape);

				if (grabbedInputVariable || grabbedOutputVariable) {

					bool doDelete = true;

					// variable transfer
					middle::loopInstances(gameState, [gameState, this, &grabbedId, &doDelete, grabbedInputVariable, grabbedOutputVariable](int i, middle::Shape& shape) {
						if (shape.id == grabbedId)
							return true;
						auto otherInputVariable = middle::getComponent<components::InputVariable>(shape);
						auto otherBubble = middle::getComponent<components::BubbleComponent>(shape);
						auto otherIntersectable = middle::getComponent<components::MouseIntersectable>(shape);
						if (!otherInputVariable && !otherBubble)
							return true;
						if (!otherIntersectable)
							return true;

						if (!otherIntersectable->intersectingTop) {
							return true;
						}

						if (otherInputVariable && grabbedInputVariable) {
							otherInputVariable->unitRef = grabbedInputVariable->unitRef;
							otherInputVariable->label = grabbedInputVariable->label;
						}
						else if (otherInputVariable && grabbedOutputVariable) {
							otherInputVariable->unitRef = grabbedOutputVariable->unitRef;
							otherInputVariable->label = grabbedOutputVariable->label;
						}
						else if (otherBubble && grabbedInputVariable) {
							grabbedInputVariable->snapId = shape.id;
							bubbleActions::UpdateVariable(grabbedInputVariable->label, shape.id).execute(gameState);
							doDelete = false;
						}
						return true;
						});

					if (doDelete) {
						middle::deleteShape(gameState, shape.id.index);
					}

					gameState->bubbleAlgebraState.grabbedId = middle::Id();
				}
			}
			return;
		}

		// dragging grabbed variable
		if (grabbedId.index != middle::UNASSIGNED) {
			auto& shape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);
			auto inputVariable = middle::getComponent<components::InputVariable>(shape);
			auto outputVariable = middle::getComponent<components::OutputVariable>(shape);
			if (inputVariable || outputVariable) {
				Vector3 targetPos = gameState->input.mouseXZ_PlanePos;
				Vector3 currentPos = middle::getShapePosition(gameState, grabbedId.index);
				middle::moveShape(gameState, grabbedId.index, targetPos - currentPos);
			}

			return;
		}

		// copy and set as grabbed
		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {
			auto inputVariable = middle::getComponent<components::InputVariable>(shape);
			auto outputVariable = middle::getComponent<components::OutputVariable>(shape);

			if (!inputVariable && !outputVariable)
				return true;

			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			if (!intersectable)
				return true;

			if (intersectable->intersecting) {
				middle::Id copyId = middle::deepCopyShape(gameState, shape.id.index, middle::UNASSIGNED);
				auto& copyShape = middle::getShape(gameState, copyId.index);
				auto copyLoop = middle::getComponent<components::LoopSociety>(copyShape);
				// set copy intersecting as false since, its copied
				middle::deleteComponent<components::MouseIntersectable>(copyShape);
				copyLoop->parentLoopId = middle::Id();
				gameState->bubbleAlgebraState.grabbedId = copyId;
			}
			return true;
			});

	}
};

static middle::SystemRegistrar<VariableLinkingSystem> reg("VariableLinkingSystem");
