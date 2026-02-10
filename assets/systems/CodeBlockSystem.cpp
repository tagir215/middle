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

class CodeBlockSystem : public middle::MiddleGameplaySystem {
public:
	CodeBlockSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
	}


	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {

			auto procedure = middle::getComponent<components::ProcedureComponent>(shape);


			// procedure block child position updates
			if (procedure) {
				auto rectangle = middle::getComponent<components::Rectangle>(shape);
				auto loop = middle::getComponent<components::LoopSociety>(shape);
				float totalHeight = 0;
				const float zmargin = 4;
				for (middle::Id& childId : loop->loopMemberIds) {
					auto& childShape = middle::getShape(gameState, childId.index);
					auto childRect = middle::getComponent<components::Rectangle>(childShape);
					totalHeight += childRect->height + zmargin;
				}

				const float minHeight = 40;
				rectangle->height = totalHeight > minHeight ? totalHeight : minHeight;
				Vector3 referencePos;
				int size = loop->loopMemberIds.size();
				if (size > 1) {
					referencePos = middle::getShapePosition(gameState, loop->loopMemberIds[0].index);
				}
				else {
					referencePos = middle::getShapePosition(gameState, shape.id.index);
				}

				for (middle::Id& childId : loop->loopMemberIds) {
					auto& childShape = middle::getShape(gameState, childId.index);
					auto position = middle::getComponent<components::Position>(childShape);
					auto childRect = middle::getComponent<components::Rectangle>(childShape);
					position->posX = referencePos.x;
					position->posY = referencePos.y;
					position->posZ = referencePos.z;
					referencePos.z -= childRect->height + zmargin;
				}

				if (size > 1) {
					auto& child0Shape = middle::getShape(gameState, loop->loopMemberIds[0].index);
					Vector3 child0Pos = middle::getShapePosition(gameState, loop->loopMemberIds[0].index);
					auto child0Rect = middle::getComponent<components::Rectangle>(child0Shape);

					Vector3 top = child0Pos + Vector3{0,0,child0Rect->height * 0.5f};
					Vector3 toCenter = Vector3{ 0,0,-totalHeight * 0.5f + zmargin * 0.5f };
					Vector3 center = top + toCenter;

					auto procPosition = middle::getComponent<components::Position>(shape);
					procPosition->posX = center.x;
					procPosition->posY = center.y;
					procPosition->posZ = center.z;
				}

			}

			auto placement = middle::getComponent<components::PlacementComponent>(shape);
			auto grabbable = middle::getComponent<components::MouseGrabbable>(shape);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);


			// code block moving
			if ((placement && placement->grabbing) || (grabbable && grabbable->grabbing)) {
				Vector3 currentPos = middle::getShapePosition(gameState, shape.id.index);
				Vector3 targetPos = gameState->input.mouseXZ_PlanePos;
				middle::moveShape(gameState, shape.id.index, targetPos - currentPos);
			}

			// add and remove from/to procedure container 
			middle::Id grabbedId = gameState->bubbleAlgebraState.grabbedId;
			if (procedure && grabbedId.index != middle::UNASSIGNED && shape.id != grabbedId) {
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				std::vector<middle::Id>children;
				middle::getChildren(gameState, shape.id, children);
				// if grabbing while not intersecting remove from loop
				if (!intersectable->intersectingTop) {
					for (middle::Id& childId : children) {
						auto& childShape = middle::getShape(gameState, childId.index);
						// if moving away while placing cancel the addition by removing from loop
						auto childPlacement = middle::getComponent<components::PlacementComponent>(childShape);
						if (childPlacement) {
							auto removeFromLoop = middle::EditorActionRemoveFromLoop(childId.index);
							removeFromLoop.execute(gameState);
						}
					}
				}
				// 1 or first is added directly to the proc, others are added as siblings dragging on top of each sibling
				int childrenSize = children.size();
				if (childrenSize == 0 && intersectable->intersectingTop) {
					auto reparent = middle::EditorActionReparent(shape.id.index, gameState->bubbleAlgebraState.grabbedId.index);
					reparent.execute(gameState);
				}
				// blocks (subject) after first one are added by intersecting children
				if (childrenSize > 0) {
					for (int index = 0; index < childrenSize; ++index) {
						middle::Id& childId = children[index];
						middle::Shape& childShape = middle::getShape(gameState, childId.index);
						auto intersectableChild = middle::getComponent<components::MouseIntersectable>(childShape);
						if (intersectableChild->intersecting) {
							auto reparent = middle::EditorActionReparent(shape.id.index, grabbedId.index);
							reparent.execute(gameState);
							auto moveIndex = middle::EditorActionChangeLoopMemberIndex(shape.id.index, grabbedId.index, index + 1);
							moveIndex.execute(gameState);
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
