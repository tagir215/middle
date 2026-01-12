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
#include "SystemReference.h"

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

				// line intersect

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
					auto intersectComponent = middle::getComponent<components::MouseIntersectable>(shape);
					intersectComponent->intersecting = mouseIntersect;
					continue;
				}

				// intersect centroid

				if (middle::getComponent<components::LoopTag>(shape)) {
					auto loop = middle::getComponent<components::LoopSociety>(shape);
					Vector3 centroid = middle::getLoopCentroid(gameState, i);
					Vector3 intersectPos;
					bool isIntersecting = middle::RayCastLineSphere(centroid, middle::DEF_RADIUS_LOOP_INDICATOR, gameState->editorState.camera.position, gameState->editorState.camera.position + gameState->input.mouseDir, intersectPos);
					intersectComponent->intersecting = isIntersecting;
					continue;
				}


				// intersect sphere

				auto position = middle::getComponent<components::Position>(shape);
				if (!position)
					continue;

				auto sphere = middle::getComponent<components::Sphere>(shape);
				auto reference = middle::getComponent<components::Reference>(shape);
				auto system = middle::getComponent<components::SystemReference>(shape);
				Vector3 pos = { position->posX, position->posY, position->posZ };

				float radius = 0;
				if (sphere) {
					radius = sphere->radius;
				}
				if (reference) {
					radius = middle::DEF_RADIUS_REFERENCE_INDICATOR;
				}
				if (system) {
					radius = middle::DEF_RADIUS_SYSTEM;
				}

				Vector3 intersectPos;
				bool isIntersecting = middle::RayCastLineSphere(pos, radius, gameState->editorState.camera.position, 
					gameState->editorState.camera.position + gameState->input.mouseDir, intersectPos);
				intersectComponent->intersecting = isIntersecting;
			}
		}
	};

	static middle::SystemRegistrar<MouseIntersectDetectionSystem> reg("MouseIntersectDetectionSystem");
}
