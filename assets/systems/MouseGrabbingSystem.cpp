#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "MouseIntersectable.h"
#include "middle_shape_utils.h"
#include "MouseGrabbable.h"
#include "Position.h"
#include "PlacementComponent.h"
#include "LoopSociety.h"
#include "editor_actions.h"
#include "GridElement.h"
#include "component_utils.h"

namespace MouseGrabbingSystem {

	class MouseGrabbingSystem : public middle::MiddleGameplaySystem {
	public:
		MouseGrabbingSystem() {
			systemUpdateType = middle::SystemUpdateType::PREFRAME;
			systemModeType = middle::SystemModeType::EDITOR;
		}

		components::CompCache* grabbableCache;
		components::CompCache* placableCache;


		void init(middle::GameState* gameState) {
			grabbableCache = middle::newCompCache(gameState);
			grabbableCache->addType<components::MouseGrabbable>();
			grabbableCache->addType<components::Position>();
			placableCache = middle::newCompCache(gameState);
			placableCache->addType<components::PlacementComponent>();
			placableCache->addType<components::Position>();
		}

		void dragging(middle::GameState* gameState, middle::Shape& shape) {
			Vector3 pos = middle::getShapePosition(gameState, shape.id.index);
			auto grid = middle::getComponent<components::GridElement>(shape);

			if (!grid) {
				Vector3 cameraPos = gameState->editorState.camera.position;
				float objYDistance = std::abs(pos.y - cameraPos.y);
				float yDistance = std::abs(cameraPos.y);
				if (yDistance == 0)
					yDistance = 0.001f;
				Vector3 xzVel = Vector3Scale(gameState->input.mouseXZ_PlaneVelocity, objYDistance / yDistance);
				dragShape(gameState, shape.id.index, xzVel);
			}
			else {
				Vector3 targetPos = gameState->input.mouseXZ_PlanePos;
				middle::moveShape(gameState, shape.id.index, targetPos - pos);
			}
		}

		void releasing(middle::GameState* gameState) {
			// update move action with new positions for redo to work
			middle::EditorActionMove* lastMoveAction = nullptr;
			// find last move action
			for (int i = gameState->editorState.actionHistory.size() - 1; i >= 0; --i) {
				middle::EditorActionContainer* lastAction = gameState->editorState.actionHistory[i].get();
				lastMoveAction = dynamic_cast<middle::EditorActionMove*>(lastAction);
				if (lastMoveAction != nullptr) {
					break;
				}
			}
			assert(lastMoveAction);
			lastMoveAction->newPositions.resize(lastMoveAction->selectedShapes.size());
			for (int j = 0; j < lastMoveAction->selectedShapes.size(); ++j) {
				auto& selectedShape = middle::getShape(gameState, lastMoveAction->selectedShapes[j]);
				auto position = middle::getComponent<components::Position>(selectedShape);
				lastMoveAction->newPositions[j] = { position->posX, position->posY, position->posZ };
			}
			gameState->editorState.grabbing = false;
		}

		void update(middle::GameState* gameState) override {

			// setup editor action for movement
			if (gameState->editorState.selectCount > 0 && gameState->input.grabDown && !gameState->editorState.grabbing) {
				middle::queueEditorAction(gameState, std::make_shared<middle::EditorActionMove>(middle::getSelectedShapes(gameState)));
				gameState->editorState.grabbing = true;
				return;
			}

			auto grabbableIt = grabbableCache->begin<components::MouseGrabbable>();
			auto positionIt = grabbableCache->begin<components::Position>();
			for (int i = 0; i < grabbableCache->getSize(); ++i) {
				auto& shape = middle::getShape(gameState, grabbableCache->relevantIdVector[i].index);
				auto grabbable = *grabbableIt;
				auto position = *positionIt;

				if (grabbable && middle::isShapeSelected(gameState, i) && gameState->input.grabDown) {
					grabbable->grabbing = true;
				}
				else if (grabbable && !gameState->input.grabDown) {
					grabbable->grabbing = false;
				}

				if (grabbable->grabbing) {
					dragging(gameState, shape);
				}
			}

			auto placableIt = placableCache->begin<components::PlacementComponent>();
			for (int i = 0; i < placableCache->getSize(); ++i) {
				auto placable = *placableIt;
				auto& shape = middle::getShape(gameState, placableCache->relevantIdVector[i].index);
				if (gameState->input.mouseClicked) {
					middle::queueComponentDeletion<components::PlacementComponent>(gameState, shape.id);
					std::vector<middle::Id>members;
					middle::getAllChildren(gameState, shape.id, members);
					for (middle::Id& childId : members) {
						middle::Shape& child = middle::getShape(gameState, childId.index);
						middle::queueComponentDeletion<components::PlacementComponent>(gameState, childId);
					}
				}

				if (placable->grabbing) {
					dragging(gameState, shape);
				}
			}

			if (gameState->input.grabReleased && gameState->editorState.grabbing) {
				releasing(gameState);
			}

		}
	};

	static middle::SystemRegistrar<MouseGrabbingSystem> reg("MouseGrabbingSystem");

}
