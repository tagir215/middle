#pragma once
#include "game_state.h"
#include "registrars.h"
#include "MouseIntersectable.h"
#include "middle_shape_utils.h"
#include "Sphere.h"
#include "Position.h"
#include "middle_math.h"
#include "Constraint.h"
#include "JointEntity.h"
#include "LoopEntity.h"
#include "ConstraintEntity.h"
#include "LoopTag.h"
#include "Reference.h"

namespace MouseIntersectDetectionSystem {

	class MouseIntersectDetectionSystem : public middle::MiddleGameplaySystem {
		void update(middle::GameState* gameState) override {

			for (int i = 0; i < gameState->shapes.size(); ++i) {
				if (!middle::isShapeAlive(gameState, i))
					continue;

				middle::Shape& shape = gameState->shapes[i];

				auto intersectComponent = middle::getComponent<components::MouseIntersectable>(shape);
				if (!intersectComponent)
					continue;

				bool wasIntersecting = middle::isMouseIntersectingShape(gameState, i);
				intersectComponent->wasIntersecting = wasIntersecting;

				auto sphere = middle::getComponent<components::Sphere>(shape);
				if (sphere) {
					Vector3 pos = middle::getShapePosition(gameState, i);
					Vector3 intersectPos;
					bool isIntersecting = middle::RayCastLineSphere(pos, sphere->radius, gameState->editorState.camera.position, gameState->editorState.camera.position + gameState->input.mouseDir, intersectPos);
					intersectComponent->intersecting = isIntersecting;
					continue;
				}

				auto constraint = middle::getComponent<components::Constraint>(shape);
				if (constraint) {
					// in constraint creation mode, can't select constraints, only spheres to create constraints to
					if (gameState->editorState.creationMode == middle::CreationMode::CONSTRAINT_MODE)
						continue;
					auto& instanceA = getShape(gameState, constraint->indexA);
					auto& instanceB = getShape(gameState, constraint->indexB);
					Vector3 posA = middle::getShapePosition(gameState, constraint->indexA);
					Vector3 posB = middle::getShapePosition(gameState, constraint->indexB);
					bool mouseIntersect = middle::PointIntersectLineZX_Plane(gameState->input.mouseXZ_PlanePos, posA, posB, middle::DEF_LINE_PADDING_H, middle::DEF_LINE_PADDING_V);
					auto intersectComponent = middle::getComponentAssert<components::MouseIntersectable>(shape);
					intersectComponent->intersecting = mouseIntersect;
					continue;
				}

				if (middle::getComponent<components::LoopTag>(shape)) {
					auto loop = middle::getComponentAssert<components::LoopSociety>(shape);
					Vector3 centroid = middle::getLoopCentroid(gameState, i);
					Vector3 intersectPos;
					bool isIntersecting = middle::RayCastLineSphere(centroid, middle::DEF_RADIUS_LOOP_INDICATOR, gameState->editorState.camera.position, gameState->editorState.camera.position + gameState->input.mouseDir, intersectPos);
					intersectComponent->intersecting = isIntersecting;
					if (isIntersecting) {
						int a = 0;
					}
					continue;
				}

				if (middle::getComponent<components::Reference>(shape)) {
					Vector3 pos = middle::getShapePosition(gameState, i);
					Vector3 intersectPos;
					bool isIntersecting = middle::RayCastLineSphere(pos, middle::DEF_RADIUS_REFERENCE_INDICATOR, 
						gameState->editorState.camera.position, gameState->editorState.camera.position + gameState->input.mouseDir, intersectPos);
					intersectComponent->intersecting = isIntersecting;
					continue;
				}
			}
		}
	};

	static middle::SystemRegistrar<MouseIntersectDetectionSystem> reg("MouseIntersectDetectionSystem");
}
