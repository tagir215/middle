#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "Reference.h"
#include "Rectangle.h"
#include "Position.h"
#include "Text.h"
#include "LoopSociety.h"
#include "PlacementComponent.h"
#include "CodeBlock.h"
#include "Inventory.h"
#include "MouseIntersectable.h"
#include "ProcedureComponent.h"
#include "editor_actions.h"

class CodeBlockSystem : public middle::MiddleGameplaySystem {
public:
	CodeBlockSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
	}


	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {

			auto placement = middle::getComponent<components::PlacementComponent>(shape);
			if (placement) {
				Vector3 currentPos = middle::getShapePosition(gameState, shape.id.index);
				Vector3 targetPos = gameState->input.mouseXZ_PlanePos;
				middle::moveShape(gameState, shape.id.index, targetPos - currentPos);
			}

			auto procedure = middle::getComponent<components::ProcedureComponent>(shape);
			if (procedure && gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED) {
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				if (intersectable->intersecting) {
					auto reparent = middle::EditorActionReparent(shape.id.index, gameState->bubbleAlgebraState.grabbedId.index);
					reparent.execute(gameState);
				}
			}

			auto inventory = middle::getComponent<components::Inventory>(shape);
			if (inventory) {
				std::vector < middle::Id>children;
				middle::getChildren(gameState, shape.id, children);

				for (middle::Id childId : children) {
					auto& child = middle::getShape(gameState, childId.index);
					auto intersectable = middle::getComponent<components::MouseIntersectable>(child);
					if (intersectable->intersectingTop && gameState->input.mouseClicked) {
						middle::Id copyId = middle::deepCopyShape(gameState, childId.index, middle::UNASSIGNED);
						auto& copyShape = middle::getShape(gameState, copyId.index);
						middle::addComponent<components::PlacementComponent>(copyShape);
						auto removeLoop = middle::EditorActionRemoveFromLoop(copyId.index);
						removeLoop.execute(gameState);
						gameState->bubbleAlgebraState.grabbedId = copyId;
					}
				}
			}

			});


		if (gameState->input.mouseReleased && gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED) {
			auto& grabbedShape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);
			auto loop = middle::getComponent<components::LoopSociety>(grabbedShape);
			if (loop->parentLoopId.index == middle::UNASSIGNED) {
				middle::deleteShape(gameState, grabbedShape.id.index);
			}
			else {
				middle::deleteComponent<components::PlacementComponent>(grabbedShape);
			}
			gameState->bubbleAlgebraState.grabbedId = middle::Id();
		}


	}
};

static middle::SystemRegistrar<CodeBlockSystem> reg("CodeBlockSystem");
