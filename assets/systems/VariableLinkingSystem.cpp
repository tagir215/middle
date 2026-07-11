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
#include "ProcedureTargetTag.h"
#include "InitializedTag.h"
#include "bubble_colors.h"

class VariableLinkingSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* inputCache;
	components::CompCache* outputCache;
	components::CompCache* bubbleCache;
	components::CompCache* procCache;
	components::CompCache* algebraCache;
	components::CompCache* procedureTargetCache;
	components::CompCache* procedureInputCache;

	void init(middle::GameState* gameState) {
		inputCache = middle::newCompCache(gameState, systemName);
		inputCache->addType<components::InputVariable>();
		inputCache->addType<components::MouseIntersectable>();
		outputCache = middle::newCompCache(gameState, systemName);
		outputCache->addType<components::OutputVariable>();
		outputCache->addType<components::MouseIntersectable>();
		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::MouseIntersectable>();
		procCache = middle::newCompCache(gameState, systemName);
		procCache->addType<components::ProcedureContainer>();
		algebraCache = middle::newCompCache(gameState, systemName);
		algebraCache->addType<components::AlgebraNode>();
		procedureTargetCache = middle::newCompCache(gameState, systemName);
		procedureTargetCache->addType<components::ProcedureTargetTag>();
		procedureTargetCache->addType<components::InitializedTag>(components::NOTINTERESTED);
		procedureInputCache = middle::newCompCache(gameState, systemName);
		procedureInputCache->addType<components::ProcedureInputVariable>();
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

		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		auto bubbleIntersectableIt = bubbleCache->begin<components::MouseIntersectable>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto& intersectingShape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);

			auto otherBubble = *bubbleIt;
			auto intersectable = *bubbleIntersectableIt;
			if (!intersectable->intersectingTop) {
				continue;
			}

			// in mul case we take the siblings as constraining factor
			std::vector<middle::Id>children;
			middle::Id parentId = middle::getParent(gameState, intersectingShape.id);
			if (parentId.index != middle::UNASSIGNED) {
				middle::Shape& parentShape = middle::getShape(gameState, parentId.index);
				auto mul = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);
				if (mul) {
					middle::getChildren(gameState, parentId, children);
				}
			}

			// set bubble reference
			auto ref = middle::getComponent<components::IdRef>(grabbedShape);
			assert(isValidId(gameState, ref->idRef));
			auto& ogShape = middle::getShape(gameState, ref->idRef.index);
			auto ogInput = middle::getComponent<components::InputVariable>(ogShape);

			bool highlighted = false;

			// update structure, unless inputting it during normal level
			if (procContainer->editMode) {
				middle::Id nodeRoot, startPointNode;
				bubble::bubbleToStructureBranch(gameState, intersectingShape.id, procContainer->bubbleRef, startPointNode, nodeRoot);
				ogInput->rootNodeId = nodeRoot;
				ogInput->startPointNodeId = startPointNode;
				middle::queueAction(gameState, std::make_shared<middle::EditorActionReparent>(ogShape.id.index, nodeRoot.index));

				middle::queueComponentAttachment<components::Highlight>(gameState, ogShape.id);
				highlighted = true;
				++procContainer->targetActionStackSize;
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

		if (procedureTargetCache->getSize() > 0) {
			middle::Id procTargetId = procedureTargetCache->relevantIdVector[0];
			assert(procedureInputCache->getSize() > 0);
			middle::Id procInputId = procedureInputCache->relevantIdVector[0];
			auto& procInputShape = middle::getShape(gameState, procInputId.index);
			auto procInput = middle::getComponent<components::InputVariable>(procInputShape);
			assert(procInput);

			procInput->unitRef = procTargetId;
			procContainer->bubbleRef = procTargetId;

			middle::Id startPointNodeId, rootNodeId;
			bubble::bubbleToStructureBranch(gameState, procTargetId, procTargetId, startPointNodeId, rootNodeId);
			procInput->startPointNodeId = startPointNodeId;
			procInput->rootNodeId = rootNodeId;
			// reparent algebra node to input, for automatic deletion and serialization
			middle::queueAction(gameState, std::make_shared<middle::EditorActionReparent>(procInputId.index, rootNodeId.index));

			middle::attachComponent<components::InitializedTag>(gameState, procTargetId);
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
