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
#include "editor_actions.h"
#include "MouseGrabbable.h"
#include "CodeFunction.h"
#include "IfComponent.h"
#include "ScopeComponent.h"
#include "ProcedureContainer.h"
#include "InputVariable.h"
#include "component_utils.h"
#include "ProcedureComponent.h"
#include "Layer.h"

class CodeBlockSystem : public middle::MiddleGameplaySystem {
public:
	CodeBlockSystem() {
		systemModeType = middle::SystemModeType::GAMEPLAY;
	}

	components::CompCache* placementCache;
	components::CompCache* grabbableCache;
	components::CompCache* scopeCache;
	components::CompCache* procScopeCache;
	components::CompCache* procedureCache;
	components::CompCache* codeBlockCache;
	components::CompCache* inventoryCache;
	components::CompCache* blockCache;

	void init(middle::GameState* gameState) {
		placementCache = middle::newCompCache(gameState, systemName);
		placementCache->addType < components::PlacementComponent>();
		grabbableCache = middle::newCompCache(gameState, systemName);
		grabbableCache->addType < components::MouseGrabbable>();
		scopeCache = middle::newCompCache(gameState, systemName);
		scopeCache->addType <components::ScopeComponent>();
		scopeCache->addType<components::MouseIntersectable>();
		scopeCache->addType<components::LoopSociety>();
		procScopeCache = middle::newCompCache(gameState, systemName);
		procScopeCache->addType<components::LoopSociety>();
		procScopeCache->addType<components::ProcedureComponent>();
		procedureCache = middle::newCompCache(gameState, systemName);
		procedureCache->addType<components::ProcedureContainer>();
		codeBlockCache = middle::newCompCache(gameState, systemName);
		codeBlockCache->addType<components::CodeBlock>();
		inventoryCache = middle::newCompCache(gameState, systemName);
		inventoryCache->addType<components::Inventory>();
		blockCache = middle::newCompCache(gameState, systemName);
		blockCache->addType<components::CodeBlock>();
		blockCache->addType<components::MouseIntersectable>();
	}

	void update(middle::GameState* gameState) override {

		auto placementIt = placementCache->begin<components::PlacementComponent>();
		for (int i = 0; i < placementCache->getSize(); ++i) {
			auto placement = *placementIt;
			if (placement->grabbing) {
				auto& shape = middle::getShape(gameState, placementCache->relevantIdVector[i].index);
				Vector3 currentPos = middle::getShapePosition(gameState, shape.id.index);
				Vector3 targetPos = gameState->input.mouseXZ_PlanePos;
				middle::moveShape(gameState, shape.id.index, targetPos - currentPos);
			}
		}

		auto grabbableIt = grabbableCache->begin<components::MouseGrabbable>();
		for (int i = 0; i < grabbableCache->getSize(); ++i) {
			auto grabbable = *grabbableIt;
			if (grabbable->grabbing) {
				auto& shape = middle::getShape(gameState, grabbableCache->relevantIdVector[i].index);
				Vector3 currentPos = middle::getShapePosition(gameState, shape.id.index);
				Vector3 targetPos = gameState->input.mouseXZ_PlanePos;
				middle::moveShape(gameState, shape.id.index, targetPos - currentPos);
			}
		}

		// get reference to proccontainer
		components::ProcedureContainer* procContainer = nullptr;
		if (procedureCache->getSize() > 0) {
			auto procIt = procedureCache->begin<components::ProcedureContainer>();
			procContainer = *procIt;
		}

		// update procedure size
		std::vector<middle::Id>procChildren;
		if (procScopeCache->getSize() > 0) {
			middle::getChildren(gameState, procScopeCache->relevantIdVector[0], procChildren);
			procContainer->size = procChildren.size();
		}

		// PROCEDURE POINTER POSITION, WARNING WARNING EXTERNAL SYSTEM
		auto blockIntersectableIt = blockCache->begin<components::MouseIntersectable>();
		for (int i = 0; i < blockCache->getSize(); ++i) {
			auto intersectable = *blockIntersectableIt;
			if (!intersectable->intersecting) {
				continue;
			}
			middle::Id intersectedId = blockCache->relevantIdVector[i];
			int targetSize = -1;
			for (int j = 0; j < procContainer->size; ++j) {
				middle::Id blockId = procChildren[j];
				if (blockId == intersectedId) {
					procContainer->targetActionStackSize = j + 1;
					procContainer->updateInputs = true;
					break;
				}
			}
		}


		middle::Id grabbedId = gameState->bubbleAlgebraState.grabbedId;
		auto scopeIt = scopeCache->begin<components::ScopeComponent>();
		auto scopeIntersectableIt = scopeCache->begin<components::MouseIntersectable>();


		if (grabbedId.index != middle::UNASSIGNED) {
			for (int i = 0; i < scopeCache->getSize(); ++i) {
				auto scope = *scopeIt;
				std::vector<middle::Id>children;
				middle::getChildren(gameState, scopeCache->relevantIdVector[i], children);
				auto scopeIntersectable = *scopeIntersectableIt;
				auto& shape = middle::getShape(gameState, scopeCache->relevantIdVector[i].index);

				// removes from loop while grabbing and moving out of container
				// if grabbing while not intersecting remove from loop
				if (!scopeIntersectable->intersecting) {
					std::vector<middle::Id>children;
					middle::getAllChildren(gameState, shape.id, children);
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
					// 1 or first is added directly to the scope, others are added as siblings dragging on top of each sibling
					int childrenSize = children.size();
					if (grabbedCodeBlock && childrenSize == 0 && scopeIntersectable->intersectingTop) {
						auto reparent = middle::EditorActionReparent(shape.id.index, gameState->bubbleAlgebraState.grabbedId.index);
						reparent.execute(gameState);
					}
					// blocks (subject) after first one are added by intersecting children
					if (grabbedCodeBlock && childrenSize > 0) {
						for (int index = 0; index < childrenSize; ++index) {
							middle::Id& childId = children[index];
							assert(childrenSize == children.size());
							middle::Shape& childShape = middle::getShape(gameState, childId.index);
							auto intersectableChild = middle::getComponent<components::MouseIntersectable>(childShape);
							if (intersectableChild->intersecting) {
								auto reparent = middle::EditorActionReparent(shape.id.index, grabbedId.index);
								reparent.execute(gameState);

								int newIndex = index + 1;
								// new index can't go behind anything if its last
								if (newIndex == children.size()) {
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
							middle::Id& childId = children[index];
							middle::Shape& childShape = middle::getShape(gameState, childId.index);
							auto intersectableChild = middle::getComponent<components::MouseIntersectable>(childShape);
							std::vector<middle::Id>children;
							middle::getChildren(gameState, childShape.id, children);
							if (children.size() == 0 && intersectableChild->intersecting) {
								auto reparent = middle::EditorActionReparent(childId.index, grabbedId.index);
								reparent.execute(gameState);
								break;
							}
						}
					}

				}
			}
		}
		else {
			for (int i = 0; i < scopeCache->getSize(); ++i) {
				auto scope = *scopeIt;
				auto& shape = middle::getShape(gameState, scopeCache->relevantIdVector[i].index);

				std::vector<middle::Id>children;
				middle::getAllChildren(gameState, shape.id, children);
				for (middle::Id& childId : children) {
					auto& childShape = middle::getShape(gameState, childId.index);

					// only codeblock and functions can be moved
					auto codeBlock = middle::getComponent<components::CodeBlock>(childShape);
					auto codeFunction = middle::getComponent<components::CodeFunction>(childShape);
					if (!codeBlock && !codeFunction)
						continue;

					// check that parent is not an if block, cause then can't grab it
					middle::Id parentId = middle::getParent(gameState, childShape.id);
					auto parentShape = middle::getShape(gameState, parentId.index);
					auto ifBlock = middle::getComponent<components::IfComponent>(parentShape);
					if (ifBlock) {
						continue;
					}


					// if grabbing child from scope, can reorder it
					auto childGrabbable = middle::getComponent<components::MouseGrabbable>(childShape);
					auto childIntersectable = middle::getComponent<components::MouseIntersectable>(childShape);
					if (childGrabbable && childIntersectable && gameState->input.mouseHeld
						&& childIntersectable->intersectingTop
						&& gameState->bubbleAlgebraState.grabbedId.index == middle::UNASSIGNED) {

						auto removeFromLoop = middle::EditorActionRemoveFromLoop(childId.index);
						removeFromLoop.execute(gameState);
						middle::attachComponent<components::PlacementComponent>(gameState, childShape.id);
						gameState->bubbleAlgebraState.grabbedId = childId;
					}
				}
			}
		}

		// copy from inventory
		auto inventoryIt = inventoryCache->begin<components::Inventory>();
		for (int i = 0; i < inventoryCache->getSize(); ++i) {
			auto inventory = *inventoryIt;
			auto& shape = middle::getShape(gameState, inventoryCache->relevantIdVector[i].index);
			std::vector < middle::Id>children;
			middle::getAllChildren(gameState, shape.id, children);

			for (middle::Id childId : children) {
				auto& child = middle::getShape(gameState, childId.index);
				auto codeBlock = middle::getComponent<components::CodeBlock>(child);
				auto intersectable = middle::getComponent<components::MouseIntersectable>(child);
				if (intersectable->intersectingTop && gameState->input.mouseClicked) {
					middle::Id copyId = middle::deepCopyShape(gameState, childId.index, middle::UNASSIGNED);
					auto& copyShape = middle::getShape(gameState, copyId.index);
					auto placement = middle::attachComponent<components::PlacementComponent>(gameState, copyShape.id);
					placement->grabbing = true;
					auto removeLoop = middle::EditorActionRemoveFromLoop(copyId.index);
					removeLoop.execute(gameState);
					gameState->bubbleAlgebraState.grabbedId = copyId;
				}
			}
		}

		// delete shape if havent added to container
		if (gameState->input.mouseReleased && gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED) {
			auto& grabbedShape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);

			auto codeBlock = middle::getComponent<components::CodeBlock>(grabbedShape);
			auto functionBlock = middle::getComponent<components::CodeFunction>(grabbedShape);

			if (!codeBlock && !functionBlock)
				return;

			middle::Id parentId = middle::getParent(gameState, grabbedShape.id);
			if (parentId.index == middle::UNASSIGNED) {
				middle::queueAction(gameState, std::make_shared<middle::EditorActionDeleteSingle>(grabbedShape.id));
			}
			else {
				middle::queueComponentDeletion<components::PlacementComponent>(gameState, grabbedShape.id);
			}
			gameState->bubbleAlgebraState.grabbedId = middle::Id();
		}


	}
};

static middle::SystemRegistrar<CodeBlockSystem> reg("CodeBlockSystem");
