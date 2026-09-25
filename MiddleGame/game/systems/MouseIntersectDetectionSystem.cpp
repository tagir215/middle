#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "MidComp/MouseIntersectable.h"
#include "middle_shape_utils.h"
#include "MidComp/Sphere.h"
#include "MidComp/GlobalTransform.h"
#include "middle_math.h"
#include "MidComp/Constraint.h"
#include "JointEntity.h"
#include "LoopEntity.h"
#include "ConstraintEntity.h"
#include "MidComp/LoopTag.h"
#include "MidComp/Reference.h"
#include "MidComp/SystemReference.h"
#include "MidComp/ComponentReference.h"
#include "MidComp/PlacementComponent.h"
#include "MidComp/HiddenTag.h"
#include "MidComp/IntersectingTag.h"
#include "MidComp/GlobalRadius.h"
#include "component_utils.h"
namespace MouseIntersectDetectionSystem {

	class MouseIntersectDetectionSystem : public middle::MiddleGameplaySystem {
	public:
		components::CompCache* intersectableCache;
		components::CompCache* intersectingCache;

		void init(middle::GameState* gameState) {
			systemUpdateType = middle::SystemUpdateType::PREFRAME;
			systemModeType = middle::SystemModeType::EDITOR;
			updatePriority = 1;

			intersectingCache = middle::newCompCache(gameState, systemName);
			intersectingCache->addType<components::IntersectingTag>();
			intersectingCache->addType<components::MouseIntersectable>();
			intersectingCache->addType<components::GlobalTransform>();
			intersectableCache = middle::newCompCache(gameState, systemName);
			intersectableCache->addType<components::MouseIntersectable>();
			intersectableCache->addType<components::GlobalTransform>();
			intersectableCache->addType<components::IntersectingTag>(components::NOTINTERESTED);
		}

		bool isIntersecting(middle::GameState* gameState, middle::Id id, const midMath::Vector3& pos) {
			auto& shape = middle::getShape(gameState, id.index);
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

			midMath::Vector3 intersectPos;
			bool isIntersecting = midMath::RayCastLineSphere(pos, radius, gameState->middleState.activeCamera.position,
				gameState->middleState.activeCamera.position + gameState->middleState.input.mouseDir, intersectPos);

			return isIntersecting;
		}

		void update(middle::GameState* gameState) override {

			auto intersectingTransformIt = intersectingCache->begin<components::GlobalTransform>();
			for (middle::Id id : intersectingCache->relevantIdVector) {
				auto transform = *intersectingTransformIt;
				midMath::Vector3 pos = transform->pos;

				if (!isIntersecting(gameState, id, pos)) {
					middle::queueComponentDeletion<components::IntersectingTag>(gameState, id);
				}
				else {
					auto tag = middle::getComp<components::IntersectingTag>(gameState, id);
					if (tag) {
						tag->intersectingTop = true;
						++tag->framesIntersected;
					}
				}
			}

			auto intersectableTransformIt = intersectableCache->begin<components::GlobalTransform>();
			for (middle::Id id : intersectableCache->relevantIdVector) {
				auto transform = *intersectableTransformIt;
				midMath::Vector3 pos = transform->pos;

				if (isIntersecting(gameState, id, pos)) {
					middle::attachComponent<components::IntersectingTag>(gameState, id);
				}
			}


		}
	};

	static middle::SystemRegistrar<MouseIntersectDetectionSystem> reg("MouseIntersectDetectionSystem");
}
