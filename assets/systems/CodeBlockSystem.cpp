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
#include "MouseGrabbable.h"
#include "CodeFunction.h"
#include "IfComponent.h"

class CodeBlockSystem : public middle::MiddleGameplaySystem {
public:
	CodeBlockSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
	}

	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {

			auto placement = middle::getComponent<components::PlacementComponent>(shape);
			auto grabbable = middle::getComponent<components::MouseGrabbable>(shape);
			auto procedure = middle::getComponent<components::ProcedureComponent>(shape);

			// code block moving
			if ((placement && placement->grabbing) || (grabbable && grabbable->grabbing)) {
				Vector3 currentPos = middle::getShapePosition(gameState, shape.id.index);
				Vector3 targetPos = gameState->input.mouseXZ_PlanePos;
				middle::moveShape(gameState, shape.id.index, targetPos - currentPos);
			}

			// add and remove from/to procedure container 
			middle::Id grabbedId = gameState->bubbleAlgebraState.grabbedId;
			if (procedure && grabbedId.index != middle::UNASSIGNED && shape.id != grabbedId) {
				auto procedureIntersectable = middle::getComponent<components::MouseIntersectable>(shape);
				auto procedureLoop = middle::getComponent<components::LoopSociety>(shape);

				// removes from loop while grabbing and moving out of container
				// if grabbing while not intersecting remove from loop
				if (!procedureIntersectable->intersecting) {
					std::vector<middle::Id>children;
					middle::getChildren(gameState, shape.id, children);
					for (middle::Id& childId : children) {
						auto& childShape = middle::getShape(gameState, childId.index);
						// if moving away while placing cancel the addition by removing from loop
						auto childPlacement = middle::getComponent<components::PlacementComponent>(childShape);
						if (childPlacement) {
							auto removeFromLoop = middle::EditorActionRemoveFromLoop(childId.index);
							removeFromLoop.execute(gameState);
							break;
						}
					}
				}
				else {

					auto& grabbedShape = middle::getShape(gameState, grabbedId.index);
					auto grabbedCodeBlock = middle::getComponent<components::CodeBlock>(grabbedShape);
					auto grabbedCodeFunction = middle::getComponent<components::CodeFunction>(grabbedShape);

					// CODEBLOCK
					// 1 or first is added directly to the proc, others are added as siblings dragging on top of each sibling
					int childrenSize = procedureLoop->loopMemberIds.size();
					if (grabbedCodeBlock && childrenSize == 0 && procedureIntersectable->intersectingTop) {
						auto reparent = middle::EditorActionReparent(shape.id.index, gameState->bubbleAlgebraState.grabbedId.index);
						reparent.execute(gameState);
					}
					// blocks (subject) after first one are added by intersecting children
					if (grabbedCodeBlock && childrenSize > 0) {
						for (int index = 0; index < childrenSize; ++index) {
							middle::Id& childId = procedureLoop->loopMemberIds[index];
							assert(childrenSize == procedureLoop->loopMemberIds.size());
							middle::Shape& childShape = middle::getShape(gameState, childId.index);
							auto intersectableChild = middle::getComponent<components::MouseIntersectable>(childShape);
							if (intersectableChild->intersecting) {
								auto reparent = middle::EditorActionReparent(shape.id.index, grabbedId.index);
								reparent.execute(gameState);
								procedureLoop = middle::getComponent<components::LoopSociety>(shape);
								assert(procedureLoop->loopMemberIds.size() >= childrenSize);

								int newIndex = index + 1;
								// new index can't go behind anything if its last
								if (newIndex == procedureLoop->loopMemberIds.size()) {
									newIndex = index;
								}
								auto moveIndex = middle::EditorActionChangeLoopMemberIndex(shape.id.index, grabbedId.index, newIndex);
								moveIndex.execute(gameState);
								break;
							}
						}
					}
					// CODEFUNCTION
					// if function is hovered above code block snap to it
					if (grabbedCodeFunction) {
						for (int index = 0; index < childrenSize; ++index) {
							middle::Id& childId = procedureLoop->loopMemberIds[index];
							middle::Shape& childShape = middle::getShape(gameState, childId.index);
							auto intersectableChild = middle::getComponent<components::MouseIntersectable>(childShape);
							auto loopChild = middle::getComponent<components::LoopSociety>(childShape);
							if (loopChild->loopMemberIds.size() == 0 && intersectableChild->intersecting) {
								auto reparent = middle::EditorActionReparent(childId.index, grabbedId.index);
								reparent.execute(gameState);
								break;
							}
						}
					}

				}

			}

			// grab placed component
			if (procedure && grabbedId.index == middle::UNASSIGNED) {
				std::vector<middle::Id>children;
				middle::getChildren(gameState, shape.id, children);
				for (middle::Id& childId : children) {
					auto& childShape = middle::getShape(gameState, childId.index);

					// check that parent is not an if block, cause then can't grab it
					auto childLoop = middle::getComponent<components::LoopSociety>(childShape);
					auto parentShape = middle::getShape(gameState, childLoop->parentLoopId.index);
					auto ifBlock = middle::getComponent<components::IfComponent>(parentShape);
					if (ifBlock) {
						continue;
					}


					// if grabbing child from procedure, can reorder it
					auto childGrabbable = middle::getComponent<components::MouseGrabbable>(childShape);
					auto childIntersectable = middle::getComponent<components::MouseIntersectable>(childShape);
					assert(childGrabbable);
					if (gameState->input.mouseHeld
						&& childIntersectable->intersectingTop
						&& gameState->bubbleAlgebraState.grabbedId.index == middle::UNASSIGNED) {

						auto removeFromLoop = middle::EditorActionRemoveFromLoop(childId.index);
						removeFromLoop.execute(gameState);
						middle::addComponent<components::PlacementComponent>(childShape);
						gameState->bubbleAlgebraState.grabbedId = childId;
					}
				}
			}

			// copy from inventory
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
						auto placement = middle::addComponent<components::PlacementComponent>(copyShape);
						placement->grabbing = true;
						auto removeLoop = middle::EditorActionRemoveFromLoop(copyId.index);
						removeLoop.execute(gameState);
						gameState->bubbleAlgebraState.grabbedId = copyId;
					}
				}
			}

			});


		// delete shape if havent added to container
		if (gameState->input.mouseReleased && gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED) {
			auto& grabbedShape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);
			auto loop = middle::getComponent<components::LoopSociety>(grabbedShape);
			if (loop->parentLoopId.index == middle::UNASSIGNED) {
				middle::deleteShapeRecursive(gameState, grabbedShape.id.index);
			}
			else {
				middle::deleteComponent<components::PlacementComponent>(grabbedShape);
			}
			gameState->bubbleAlgebraState.grabbedId = middle::Id();
		}


	}
};

static middle::SystemRegistrar<CodeBlockSystem> reg("CodeBlockSystem");
