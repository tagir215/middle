#pragma once
#include "game_state.h"
#include "registrars.h"
#include "MouseIntersectable.h"
#include "middle_shape_utils.h"
#include "Sphere.h"
#include "Position.h"
#include "middle_math.h"
#include "Constraint.h"

namespace MouseIntersectDetectionSystem {

	class MouseIntersectDetectionSystem : public middle::MiddleGameplaySystem {
		void update(middle::GameState* gameState) override {

			for (int i = 0; i < gameState->shapes.size(); ++i) {
				middle::Shape& shape = gameState->shapes[i];

				bool wasIntersecting = middle::isMouseIntersectingShape(gameState, i);
				bool bContainer = isContainer(gameState, i);
				bool bSphere = isSphere(gameState, i);
				if (bSphere) {
					// when in constraint mode can only select actual spheres
					if (gameState->editorState.creationMode == middle::CreationMode::CONSTRAINT_MODE && !isSphere(gameState, i))
						return;

					auto sphere = middle::getComponent<components::Sphere>(shape);
					Vector3 pos = middle::getShapePosition(gameState, i);
					Vector3 intersectPos;
					bool isIntersecting = middle::RayCastLineSphere(pos, sphere->radius, gameState->editorState.initCamera.position, gameState->editorState.initCamera.position + gameState->input.mouseDir, intersectPos);
					auto intersectComponent = middle::getComponentAssert<components::MouseIntersectable>(shape);
					intersectComponent->intersecting = isIntersecting;
				}

				auto constraint = middle::getComponent<components::Constraint>(shape);
				if (constraint != nullptr) {
					// in constraint creation mode, can't select constraints, only spheres to create constraints to
					if (gameState->editorState.creationMode == middle::CreationMode::CONSTRAINT_MODE)
						return;
					auto& instanceA = getShape(gameState, constraint->indexA);
					auto& instanceB = getShape(gameState, constraint->indexB);
					Vector3 posA = middle::getShapePosition(gameState, constraint->indexA);
					Vector3 posB = middle::getShapePosition(gameState, constraint->indexB);
					bool mouseIntersect = middle::PointIntersectLineZX_Plane(gameState->input.mouseXZ_PlanePos, posA, posB, middle::DEF_LINE_PADDING_H, middle::DEF_LINE_PADDING_V);
					auto intersectComponent = middle::getComponentAssert<components::MouseIntersectable>(shape);
					intersectComponent->intersecting = mouseIntersect;
				}
			}
		}
	};

	static middle::SystemRegistrar<MouseIntersectDetectionSystem> reg("MouseIntersectDetectionSystem");
}
