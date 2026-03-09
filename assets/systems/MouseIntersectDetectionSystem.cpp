#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
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
#include "ComponentReference.h"
#include "PlacementComponent.h"
#include "HiddenTag.h"

namespace MouseIntersectDetectionSystem {

	class MouseIntersectDetectionSystem : public middle::MiddleGameplaySystem {
	public:
		MouseIntersectDetectionSystem() {
			systemUpdateType = middle::SystemUpdateType::PREFRAME;
			systemModeType = middle::SystemModeType::EDITOR;
		}

		components::CompCache* lineIntersectableCache;
		components::CompCache* intersectableCache;

		void init(middle::GameState* gameState) {
			lineIntersectableCache = middle::newCompCache(gameState);
			lineIntersectableCache->addType<components::MouseIntersectable>();
			lineIntersectableCache->addType<components::Constraint>();
			intersectableCache = middle::newCompCache(gameState);
			intersectableCache->addType<components::MouseIntersectable>();
		}

		void update(middle::GameState* gameState) override {

			auto intersectableIt = lineIntersectableCache->begin<components::MouseIntersectable>();
			auto constraintIt = lineIntersectableCache->begin<components::Constraint>();
			for (int i = 0; i < lineIntersectableCache->getSize(); ++i) {
				middle::Shape& shape = middle::getShape(gameState, lineIntersectableCache->relevantIdVector[i].index);
				auto constraint = *constraintIt;
				auto intersectComponent = *intersectableIt;
				auto hidden = middle::getComponent<components::HiddenTag>(shape);
				if (hidden)
					continue;
				auto placable = middle::getComponent<components::PlacementComponent>(shape);
				if (placable)
					continue;

				bool wasIntersecting = intersectComponent->intersecting;
				intersectComponent->wasIntersecting = wasIntersecting;

				// in constraint creation mode, can't select constraints, only spheres to create constraints to
				if (gameState->editorState.creationMode == middle::CreationMode::CONSTRAINT_MODE)
					continue;
				auto& instanceA = getShape(gameState, constraint->idA.index);
				auto& instanceB = getShape(gameState, constraint->idB.index);
				Vector3 posA = middle::getShapePosition(gameState, constraint->idA.index);
				Vector3 posB = middle::getShapePosition(gameState, constraint->idB.index);
				bool mouseIntersect = middle::PointIntersectLineZX_Plane(gameState->input.mouseXZ_PlanePos, posA, posB, middle::DEF_LINE_PADDING_H, middle::DEF_LINE_PADDING_V);
				intersectComponent->intersecting = mouseIntersect;
				intersectComponent->intersectingTop = mouseIntersect;
			}

			bool foundIntersecting = false;
			auto sphereIntersectableIt = intersectableCache->begin<components::MouseIntersectable>();
			for (int i = 0; i < intersectableCache->getSize(); ++i) {
				middle::Shape& shape = middle::getShape(gameState, intersectableCache->relevantIdVector[i].index);
				auto intersectable = *sphereIntersectableIt;
				auto hidden = middle::getComponent<components::HiddenTag>(shape);
				if (hidden)
					continue;
				auto placable = middle::getComponent<components::PlacementComponent>(shape);
				if (placable)
					continue;
				auto position = middle::getComponent<components::Position>(shape);
				if (!position)
					continue;
				auto constraint = middle::getComponent<components::Constraint>(shape);
				if (constraint) {
					continue;
				}

				auto sphere = middle::getComponent<components::Sphere>(shape);
				auto reference = middle::getComponent<components::Reference>(shape);
				auto system = middle::getComponent<components::SystemReference>(shape);
				auto compRef = middle::getComponent<components::ComponentReference>(shape);
				auto loopTag = middle::getComponent<components::LoopTag>(shape);


				float radius = 0;
				if (sphere) {
					radius = sphere->radius;
				}
				if (reference || compRef) {
					radius = middle::DEF_RADIUS_REFERENCE_INDICATOR;
				}
				if (system) {
					radius = middle::DEF_RADIUS_SYSTEM;
				}
				if (loopTag) {
					radius = middle::DEF_RADIUS_LOOP_INDICATOR;
				}

				Vector3 pos = middle::getShapePosition(gameState, shape.id.index);

				Vector3 intersectPos;
				bool isIntersecting = middle::RayCastLineSphere(pos, radius, gameState->activeCamera.position,
					gameState->activeCamera.position + gameState->input.mouseDir, intersectPos);
				intersectable->intersecting = isIntersecting;

				if (isIntersecting) {
					if (!foundIntersecting) {
						intersectable->intersectingTop = true;
					}
					foundIntersecting = true;
				}
				else {
					intersectable->intersectingTop = false;
				}
			}


		}
	};

	static middle::SystemRegistrar<MouseIntersectDetectionSystem> reg("MouseIntersectDetectionSystem");
}
