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
#include "Highlight.h"
#include "IdRef.h"
#include "bubble_utils.h"
#include "ProcedureContainer.h"
#include "ProcedureInputVariable.h"

class VariableLinkingSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* inputCache;
	components::CompCache* outputCache;
	components::CompCache* bubbleCache;
	components::CompCache* procCache;
	components::CompCache* algebraCache;

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
		procCache = middle::newCompCache(gameState);
		procCache->addType<components::ProcedureContainer>();
		algebraCache = middle::newCompCache(gameState);
		algebraCache->addType<components::AlgebraNode>();

	}

	void copy(middle::GameState* gameState, middle::Id toCopyId) {
		middle::queueAction(gameState, std::make_shared<middle::CustomAction>([toCopyId](middle::GameState* gameState) {
			middle::Id copyId = middle::deepCopyShape(gameState, toCopyId.index, middle::UNASSIGNED);
			auto& copyShape = middle::getShape(gameState, copyId.index);
			auto copyLoop = middle::getComponent<components::LoopSociety>(copyShape);
			// set copy intersecting as false since, its copied
			middle::queueComponentDeletion<components::MouseIntersectable>(gameState, copyShape.id);
			auto idRef = middle::attachComponent<components::IdRef>(gameState, copyShape.id);
			idRef->idRef = toCopyId;
			copyLoop->parentLoopId = middle::Id();
			gameState->bubbleAlgebraState.grabbedId = copyId;
			}));
	}

	void variableTransfer(middle::GameState* gameState, middle::Id& grabbedId, components::ProcedureContainer* procContainer) {
		// variable transfer

		auto& grabbedShape = middle::getShape(gameState, grabbedId.index);
		auto grabbedInput = middle::getComponent<components::InputVariable>(grabbedShape);
		auto grabbedProcedureInput = middle::getComponent<components::ProcedureInputVariable>(grabbedShape);

		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		auto bubbleIntersectableIt = bubbleCache->begin<components::MouseIntersectable>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);
			auto otherBubble = *bubbleIt;
			auto intersectable = *bubbleIntersectableIt;
			if (!intersectable->intersectingTop) {
				continue;
			}

			// set bubble reference
			auto ref = middle::getComponent<components::IdRef>(grabbedShape);
			auto& ogShape = middle::getShape(gameState, ref->idRef.index);
			auto ogInput = middle::getComponent<components::InputVariable>(ogShape);

			bool highlighted = false;

			// update structure, unless inputting it during normal level
			if (procContainer->editMode) {
				middle::Id structureId = bubble::bubbleToStructure(gameState, shape.id);
				// reparent algebra node to input, for automatic deletion and serialization
				middle::queueAction(gameState, std::make_shared<middle::EditorActionReparent>(ogShape.id.index, structureId.index));
				ogInput->structureId = structureId;
				ogInput->structureDepth = bubble::findDepth(gameState, shape.id);
				middle::queueComponentAttachment<components::Highlight>(gameState, ogShape.id);
				highlighted = true;
			}

			// if its procedre input update unit ref here, and set proc container to point to the ref
			if(grabbedProcedureInput){
				if (bubble::matchesStructureWithVariables(gameState, shape.id, ogInput->structureId)) {
					ogInput->unitRef = shape.id;
					procContainer->bubbleRef = shape.id;
					procContainer->variableOverrides = bubble::generateVariableOverrides(gameState, shape.id, ogInput->structureId);

					if (!highlighted) {
						middle::queueComponentAttachment<components::Highlight>(gameState, ogShape.id);
					}
				}
				else {
					// TODO PRINT NOT MATCHING ERROR
				}
			}

		}

	}

	void update(middle::GameState* gameState) override {

		middle::Id& grabbedId = gameState->bubbleAlgebraState.grabbedId;

		components::ProcedureContainer* procContainer = nullptr;
		if (procCache->getSize() > 0) {
			auto procIt = procCache->begin<components::ProcedureContainer>();
			procContainer = *procIt;
			assert(procContainer);
		}
		else {
			return;
		}

		if (!gameState->input.mouseHeld) {

			if (grabbedId.index != middle::UNASSIGNED) {
				auto& shape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);

				auto grabbedInputVariable = middle::getComponent<components::InputVariable>(shape);
				auto grabbedOutputVariable = middle::getComponent<components::OutputVariable>(shape);

				if (grabbedInputVariable || grabbedOutputVariable) {

					variableTransfer(gameState, grabbedId, procContainer);

					procContainer->updateInputs = true;

					middle::queueAction(gameState, std::make_shared<middle::EditorActionDeleteSingle>(shape.id));

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

				auto procedureInput = middle::getComponent<components::ProcedureInputVariable>(shape);

				//reset input, if not procedureInput,  procedureInput can be edited if in edit mode
				if (!procedureInput || procContainer->editMode) {
					if (input->structureId.index != middle::UNASSIGNED) {
						middle::deleteShapeRecursive(gameState, input->structureId.index);
					}
					input->structureId = middle::Id();
					input->unitRef = middle::Id();
				}

				if (middle::getComponent<components::Highlight>(shape)) {
					middle::queueComponentDeletion<components::Highlight>(gameState, shape.id);
				}

				procContainer->updateInputs = true;

				middle::Id toCopyId = shape.id;
				copy(gameState, toCopyId);
			}
		}

	}
};

static middle::SystemRegistrar<VariableLinkingSystem> reg("VariableLinkingSystem");
