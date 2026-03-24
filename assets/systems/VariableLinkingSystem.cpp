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

class VariableLinkingSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* inputCache;
	components::CompCache* outputCache;
	components::CompCache* bubbleCache;
	components::CompCache* procCache;

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

	void travelStruct(middle::GameState* gameState, middle::Id id) {
		auto& structureShape = middle::getShape(gameState, id.index);
		auto structComp = middle::getComponent<components::AlgebraNode>(structureShape);

		for (middle::Id id : structComp->children) {
			travelStruct(gameState, id);
		}
	}

	void variableTransfer(middle::GameState* gameState, middle::Id& grabbedId) {
		// variable transfer

		auto& grabbedShape = middle::getShape(gameState, grabbedId.index);
		auto grabbedInput = middle::getComponent<components::InputVariable>(grabbedShape);

		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		auto bubbleIntersectableIt = bubbleCache->begin<components::MouseIntersectable>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);
			auto otherBubble = *bubbleIt;
			auto intersectable = *bubbleIntersectableIt;
			if (!intersectable->intersectingTop) {
				continue;
			}

			middle::Id structureId = bubble::bubbleToStructure(gameState, shape.id);

			// set bubble reference
			auto ref = middle::getComponent<components::IdRef>(grabbedShape);
			auto& ogShape = middle::getShape(gameState, ref->idRef.index);
			auto ogInput = middle::getComponent<components::InputVariable>(ogShape);
			ogInput->structureId = structureId;
			ogInput->structureDepth = bubble::findDepth(gameState, shape.id);
			ogInput->topDogContainer = bubble::findTopDog(gameState, shape.id);
			middle::attachComponent<components::Highlight>(gameState, ogShape.id);
		}

	}

	void update(middle::GameState* gameState) override {

		middle::Id& grabbedId = gameState->bubbleAlgebraState.grabbedId;

		auto procIt = procCache->begin<components::ProcedureContainer>();
		components::ProcedureContainer* procContainer = *procIt;
		assert(procContainer);

		if (!gameState->input.mouseHeld) {

			if (grabbedId.index != middle::UNASSIGNED) {
				auto& shape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);

				auto grabbedInputVariable = middle::getComponent<components::InputVariable>(shape);
				auto grabbedOutputVariable = middle::getComponent<components::OutputVariable>(shape);

				if (grabbedInputVariable || grabbedOutputVariable) {

					variableTransfer(gameState, grabbedId);

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
				//reset input
				input->structureId = middle::Id();
				input->unitRef = middle::Id();
				auto& shape = middle::getShape(gameState, inputCache->relevantIdVector[i].index);
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
