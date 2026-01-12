#pragma once
#include "game_state.h"
#include "registrars.h"
#include "middle_shape_utils.h"
#include "Sphere.h"
#include "Position.h"
#include "Constraint.h"
#include "PhysicsData.h"
#include "Color.h"
#include "MouseSelectable.h"
#include "MouseGrabbable.h"
#include "MouseIntersectable.h"
#include "LoopSociety.h"
#include "JointEntity.h"
#include "LoopEntity.h"
#include "ConstraintEntity.h"
#include "LoopTag.h"
#include "Reference.h"
#include "SystemReference.h"


class EditorRenderSetupSystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {
		gameState->renderData.clear();
		loopInstances(gameState, [gameState](int i, middle::Shape& shape) {


			auto sphere = middle::getComponent<components::Sphere>(shape);
			if (sphere) {
				auto pos = middle::getComponent<components::Position>(shape);
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				auto selectable = middle::getComponent<components::MouseSelectable>(shape);
				middle::RenderItem sphereItem;
				sphereItem.type = middle::RenderItemType::SPHERE;
				sphereItem.radius = sphere->radius;
				sphereItem.center = { pos->posX, pos->posY, pos->posZ };
				sphereItem.color = middle::JOINT_COLOR;
				if (intersectable && intersectable->intersecting) {
					sphereItem.color = middle::HOVERED_THING_COLOR;
				}
				gameState->renderData.push_back(sphereItem);

				if (selectable && selectable->selected) {
					middle::RenderItem selectItem;
					selectItem.type = middle::RenderItemType::RECTANGLE;
					selectItem.center = { 0,0,0 };
					selectItem.transform.translation = getShapePosition(gameState, i);
					selectItem.transform.scale = { 1,1,1 };
					selectItem.transform.rotation = { 0,0,0,0 };
					selectItem.widht = sphereItem.radius * 4;
					selectItem.height = sphereItem.radius * 4;
					selectItem.length = sphereItem.radius * 4;
					selectItem.color = ColorAlpha(WHITE, 0.4f);
					gameState->renderData.push_back(selectItem);
				}

				return;
			}

			auto constraint = middle::getComponent<components::Constraint>(shape);
			if (constraint) {
				auto selectable = middle::getComponent<components::MouseSelectable>(shape);
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				middle::RenderItem lineItem;
				lineItem.type = middle::RenderItemType::LINE;
				lineItem.linePointA = getShapePosition(gameState, constraint->indexA);
				lineItem.linePointB = getShapePosition(gameState, constraint->indexB);
				lineItem.color = middle::CONSTRAINT_COLOR;
				if (intersectable && intersectable->intersecting) {
					lineItem.color = middle::HOVERED_THING_COLOR;
				}
				gameState->renderData.push_back(lineItem);

				if (selectable && selectable->selected) {
					middle::RenderItem selectItem;
					selectItem.type = middle::RenderItemType::RECTANGLE;
					selectItem.center = { 0,0,0 };
					float height = Vector3Distance(lineItem.linePointA, lineItem.linePointB);
					Vector3 lineDir = Vector3Normalize(lineItem.linePointB - lineItem.linePointA);
					selectItem.widht = 1;
					selectItem.height = height;
					selectItem.length = 1;
					selectItem.color = ColorAlpha(WHITE, 0.4f);
					selectItem.transform.scale = { 1,1,1 };
					selectItem.transform.rotation = QuaternionFromVector3ToVector3({ 0,0,1 }, lineDir);
					selectItem.transform.translation = Vector3Scale(lineItem.linePointA + lineItem.linePointB, 0.5f);
					gameState->renderData.push_back(selectItem);
				}
				return;
			}

			auto reference = middle::getComponent<components::Reference>(shape);
			if (reference) {
				auto selectable = middle::getComponent<components::MouseSelectable>(shape);
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				auto position = middle::getComponent<components::Position>(shape);
				middle::RenderItem refItem;
				refItem.type = middle::RenderItemType::SPHERE;
				refItem.color = middle::REFERENCE_INDICATOR_COLOR;
				refItem.center = { position->posX, position->posY, position->posZ };
				refItem.radius = middle::DEF_RADIUS_REFERENCE_INDICATOR;
				if (intersectable && intersectable->intersecting) {
					refItem.color = middle::HOVERED_THING_COLOR;
				}
				gameState->renderData.push_back(refItem);

				if (selectable && selectable->selected) {
					middle::RenderItem selectItem;
					selectItem.type = middle::RenderItemType::RECTANGLE;
					selectItem.center = { 0,0,0 };
					selectItem.transform.translation = getShapePosition(gameState, i);
					selectItem.transform.scale = { 1,1,1 };
					selectItem.transform.rotation = { 0,0,0,0 };
					selectItem.widht = refItem.radius * 4;
					selectItem.height = refItem.radius * 4;
					selectItem.length = refItem.radius * 4;
					selectItem.color = ColorAlpha(WHITE, 0.4f);
					gameState->renderData.push_back(selectItem);
				}
			}

			if (!reference && middle::getComponent<components::LoopTag>(shape)) {
				auto loop = middle::getComponent<components::LoopSociety>(shape);
				auto selectable = middle::getComponent<components::MouseSelectable>(shape);
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				middle::RenderItem loopItem;
				Vector3 centroid = getLoopCentroid(gameState, i);
				loopItem.type = middle::RenderItemType::SPHERE;
				loopItem.center = centroid;
				loopItem.radius = middle::DEF_RADIUS_LOOP_INDICATOR;
				loopItem.color = middle::LOOP_INDICATOR_COLOR;
				if (intersectable && intersectable->intersecting) {
					loopItem.color = RED;
				}
				gameState->renderData.push_back(loopItem);

				if (selectable && selectable->selected) {
					middle::RenderItem selectItem;
					selectItem.type = middle::RenderItemType::RECTANGLE;
					selectItem.center = { 0,0,0 };
					selectItem.transform.translation = loopItem.center;
					selectItem.transform.scale = { 1,1,1 };
					selectItem.transform.rotation = { 0,0,0,0 };
					selectItem.widht = loopItem.radius * 4;
					selectItem.height = loopItem.radius * 4;
					selectItem.length = loopItem.radius * 4;
					selectItem.color = ColorAlpha(WHITE, 0.4f);
					gameState->renderData.push_back(selectItem);
				}
			}

			auto system = middle::getComponent<components::SystemReference>(shape);
			if (system) {
				auto selectable = middle::getComponent<components::MouseSelectable>(shape);
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				auto position = middle::getComponent<components::Position>(shape);
				middle::RenderItem systemItem;
				systemItem.type = middle::RenderItemType::SPHERE;
				systemItem.center = { position->posX, position->posY, position->posZ };
				systemItem.radius = middle::DEF_RADIUS_SYSTEM;
				systemItem.color = GREEN;
				if (intersectable && intersectable->intersecting) {
					systemItem.color = RED;
				}
				gameState->renderData.push_back(systemItem);

				if (selectable && selectable->selected) {
					middle::RenderItem selectItem;
					selectItem.type = middle::RenderItemType::RECTANGLE;
					selectItem.center = { 0,0,0 };
					selectItem.transform.translation = systemItem.center;
					selectItem.transform.scale = { 1,1,1 };
					selectItem.transform.rotation = { 0,0,0,0 };
					selectItem.widht = systemItem.radius * 4;
					selectItem.height = systemItem.radius * 4;
					selectItem.length = systemItem.radius * 4;
					selectItem.color = ColorAlpha(WHITE, 0.4f);
					gameState->renderData.push_back(selectItem);
				}

			}

			});

	}
};

static middle::SystemRegistrar<EditorRenderSetupSystem> reg("EditorRenderSetupSystem");
