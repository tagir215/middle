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

	components::CompCache* inputCache;
	components::CompCache* outputCache;
	components::CompCache* bubbleCache;

	void init(middle::GameState* gameState) {
		inputCache = middle::newCompCache(gameState);
		inputCache->addType<components::InputVariable>();
		inputCache->addType<components::MouseIntersectable>();
		outputCache = middle::newCompCache(gameState);
		outputCache->addType<components::OutputVariable>();
		outputCache->addType<components::MouseIntersectable>();
		bubbleCache = middle::newCompCache(gameState);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::MouseIntersectable>();
	}

	void copy(middle::GameState* gameState, middle::Id toCopyId) {
		middle::queueAction(gameState, std::make_shared<middle::CustomAction>([toCopyId](middle::GameState* gameState) {
			middle::Id copyId = middle::deepCopyShape(gameState, toCopyId.index, middle::UNASSIGNED);
			auto& copyShape = middle::getShape(gameState, copyId.index);
			auto copyLoop = middle::getComponent<components::LoopSociety>(copyShape);
			// set copy intersecting as false since, its copied
			middle::deleteComponent<components::MouseIntersectable>(copyShape);
			copyLoop->parentLoopId = middle::Id();
			gameState->bubbleAlgebraState.grabbedId = copyId;
			}));
	}

	void variableTransfer(middle::GameState* gameState, middle::Id& grabbedId, bool& doDelete) {
		// variable transfer

		auto& grabbedShape = middle::getShape(gameState, grabbedId.index);
		auto grabbedInput = middle::getComponent<components::InputVariable>(grabbedShape);
		auto grabbedOutput = middle::getComponent<components::OutputVariable>(grabbedShape);

		auto inputIt = inputCache->begin<components::InputVariable>();
		auto intersectableIt = inputCache->begin<components::MouseIntersectable>();
		for (int i = 0; i < inputCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, inputCache->relevantIdVector[i].index);
			if (shape.id == grabbedId) {
				continue;
			}
			auto otherInput = *inputIt;
			auto intersectable = *intersectableIt;
			if (!intersectable->intersectingTop) {
				continue;
			}

			if (otherInput && grabbedInput) {
				otherInput->unitRef = grabbedInput->unitRef;
				otherInput->label = grabbedInput->label;
			}
			else if (otherInput && grabbedOutput) {
				otherInput->unitRef = grabbedOutput->unitRef;
				otherInput->label = grabbedOutput->label;
			}
		}

		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		auto bubbleIntersectableIt = bubbleCache->begin<components::MouseIntersectable>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);
			auto otherBubble = *bubbleIt;
			auto intersectable = *bubbleIntersectableIt;
			if (!intersectable->intersectingTop) {
				continue;
			}

			if (otherBubble && grabbedInput) {
				grabbedInput->snapId = shape.id;
				bubbleActions::UpdateVariable(grabbedInput->label, shape.id).execute(gameState);
				doDelete = false;
			}
		}

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

					variableTransfer(gameState, grabbedId, doDelete);

					if (doDelete) {
						middle::queueAction(gameState, std::make_shared<middle::EditorActionDeleteSingle>(shape.id));
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
		auto inputIt = inputCache->begin<components::InputVariable>();
		auto intersectableIt = inputCache->begin<components::MouseIntersectable>();
		for (int i = 0; i < inputCache->getSize(); ++i) {
			auto input = *inputIt;
			auto intersectable = *intersectableIt;
			if (intersectable->intersecting) {
				auto& shape = middle::getShape(gameState, inputCache->relevantIdVector[i].index);
				middle::Id toCopyId = shape.id;
				copy(gameState, toCopyId);
			}
		}

		auto outputIt = outputCache->begin<components::OutputVariable>();
		auto outputIntersectableIt = outputCache->begin<components::MouseIntersectable>();
		for (int i = 0; i < outputCache->getSize(); ++i) {
			auto ouptut = *outputIt;
			auto intersectable = *outputIntersectableIt;
			if (intersectable->intersecting) {
				auto& shape = middle::getShape(gameState, outputCache->relevantIdVector[i].index);
				middle::Id toCopyId = shape.id;
				copy(gameState, toCopyId);
			}
		}

	}
};

static middle::SystemRegistrar<VariableLinkingSystem> reg("VariableLinkingSystem");
