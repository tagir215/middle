#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
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
#include "ComponentReference.h"
#include "Text.h"
#include "HiddenTag.h"
#include "ConfigComponent.h"
#include "EditorConfigs.h"
#include "middle_math.h"

class EditorRenderSetupSystem : public middle::MiddleGameplaySystem {
public:
	EditorRenderSetupSystem() {
		systemUpdateType = middle::SystemUpdateType::RENDERING;
		systemModeType = middle::SystemModeType::EDITOR;
	}

	void init(middle::GameState* gameState) {

	}
	void update(middle::GameState* gameState) override {

		Color textColor = WHITE;
		Color systemColor = GREEN;
		Color selectionBoxColor = { WHITE.r, WHITE.g, WHITE.b, 40 };
		Color hoveredColor = middle::HOVERED_THING_COLOR;
		Color backgroundColor = middle::BACKGROUND_COLOR;
		Color jointColor = middle::JOINT_COLOR;
		Color constraintColor = middle::CONSTRAINT_COLOR;
		Color referenceColor = middle::REFERENCE_INDICATOR_COLOR;
		Color loopColor = middle::LOOP_INDICATOR_COLOR;
		Color loopItemColor = WHITE;
		Color configColor = ORANGE;
		Color cameraColor = BLUE;


		if (gameState->editorState.creationMode == middle::CreationMode::LOOP_MODE) {
			backgroundColor = GRAY;
			//textColor = GRAY;
			//systemColor = GRAY;
		}
		gameState->editorState.backgroundColor = backgroundColor;



		loopInstances(gameState, [&](int i, middle::Shape& shape) {

			auto hidden = middle::getComponent<components::HiddenTag>(shape);
			if (hidden)
				return true;

			auto position = middle::getComponent<components::Position>(shape);
			auto selectable = middle::getComponent<components::MouseSelectable>(shape);
			auto loopTag = middle::getComponent<components::LoopTag>(shape);
			auto loopSociety = middle::getComponent<components::LoopSociety>(shape);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			auto config = middle::getComponent<components::ConfigComponent>(shape);
			auto editorConfigs = middle::getComponent<components::EditorConfigs>(shape);

			if (editorConfigs) {
				if (editorConfigs->gridSize == 0)
					return true;
				// draw grid
				const Color CartesianColor = WHITE;
				Vector3 mouseXz = gameState->input.mouseXZ_PlanePos;
				Vector3 mouseGridPos = middle::gridPosition(mouseXz, editorConfigs->gridSize);

				const float visibleGridRadius = editorConfigs->gridSize * editorConfigs->visibleGridPointRadiusCount;
				float visibleGridRadiusSq = visibleGridRadius * visibleGridRadius;
				int startX = mouseGridPos.x - visibleGridRadius;
				int startZ = mouseGridPos.z - visibleGridRadius;
				for (float x = startX; x < startX + visibleGridRadius * 2; x += editorConfigs->gridSize) {
					for (float z = startZ; z < startZ + visibleGridRadius * 2; z += editorConfigs->gridSize) {
						float deltaX = mouseXz.x - x;
						float deltaZ = mouseXz.z - z;
						float distSq = deltaX * deltaX + deltaZ * deltaZ;
						float ratio = distSq / visibleGridRadiusSq;
						if (distSq < visibleGridRadiusSq) {
							middle::RenderItem gridSphere;
							gridSphere.type = middle::RenderItemType::SPHERE;
							gridSphere.center = { x, 0, z };
							gridSphere.color = CartesianColor;
							gridSphere.color.a = (1.0f - ratio) * 255;
							gridSphere.radius = editorConfigs->gridSize * 0.05f;
							gameState->renderData.push_back(gridSphere);
						}
					}
				}
			}


			if (config) {
				middle::RenderItem configSphere;
				configSphere.type = middle::RenderItemType::SPHERE;
				configSphere.center = { position->posX, position->posY, position->posZ };
				configSphere.radius = middle::DEF_RADIUS_SYSTEM;
				configSphere.color = configColor;
				if (intersectable && intersectable->intersecting) {
					configSphere.color = hoveredColor;
				}
				gameState->renderData.push_back(configSphere);

				if (selectable && selectable->selected) {
					middle::RenderItem selectItem;
					selectItem.type = middle::RenderItemType::RECTANGLE;
					selectItem.center = { 0,0,0 };
					selectItem.transform.translation = configSphere.center;
					selectItem.transform.scale = { 1,1,1 };
					selectItem.transform.rotation = { 0,0,0,0 };
					selectItem.width = configSphere.radius * 4;
					selectItem.height = configSphere.radius * 4;
					selectItem.length = configSphere.radius * 4;
					selectItem.color = selectionBoxColor;
					gameState->renderData.push_back(selectItem);
				}

				return true;
			}

			auto text = middle::getComponent<components::Text>(shape);
			if (text && text->visible) {
				assert(position);
				middle::RenderItem textItem;
				Vector3 offset = { text->offsetX, text->offsetY, text->offsetZ };
				textItem.type = middle::RenderItemType::TEXT;
				textItem.center = Vector3Add({ position->posX, position->posY, position->posZ }, offset);
				textItem.text = text->text;
				textItem.fontSize = text->fontSize;
				textItem.color = textColor;
				gameState->renderData.push_back(textItem);
			}


			auto componentRef = middle::getComponent<components::ComponentReference>(shape);
			if (componentRef) {
				assert(position);
				middle::RenderItem compRefItem;
				compRefItem.type = middle::RenderItemType::SPHERE;
				compRefItem.radius = middle::DEF_RADIUS_REFERENCE_INDICATOR;
				compRefItem.color = BLUE;
				compRefItem.center = { position->posX, position->posY, position->posZ };
				if (intersectable->intersecting) {
					compRefItem.color = hoveredColor;
				}
				gameState->renderData.push_back(compRefItem);
			}


			auto sphere = middle::getComponent<components::Sphere>(shape);
			if (sphere) {
				auto selectable = middle::getComponent<components::MouseSelectable>(shape);
				middle::RenderItem sphereItem;
				sphereItem.type = middle::RenderItemType::SPHERE;
				sphereItem.radius = sphere->radius;
				sphereItem.center = middle::getShapePosition(gameState, shape.id.index);
				sphereItem.color = jointColor;
				if (intersectable && intersectable->intersecting) {
					sphereItem.color = hoveredColor;
				}
				gameState->renderData.push_back(sphereItem);

				if (selectable && selectable->selected) {
					middle::RenderItem selectItem;
					selectItem.type = middle::RenderItemType::RECTANGLE;
					selectItem.center = { 0,0,0 };
					selectItem.transform.translation = getShapePosition(gameState, i);
					selectItem.transform.scale = { 1,1,1 };
					selectItem.transform.rotation = { 0,0,0,0 };
					selectItem.width = sphereItem.radius * 4;
					selectItem.height = sphereItem.radius * 4;
					selectItem.length = sphereItem.radius * 4;
					selectItem.color = selectionBoxColor;
					gameState->renderData.push_back(selectItem);
				}

			}

			auto constraint = middle::getComponent<components::Constraint>(shape);
			if (constraint) {
				auto selectable = middle::getComponent<components::MouseSelectable>(shape);
				middle::RenderItem lineItem;
				lineItem.type = middle::RenderItemType::LINE;
				lineItem.linePointA = getShapePosition(gameState, constraint->idA.index);
				lineItem.linePointB = getShapePosition(gameState, constraint->idB.index);
				lineItem.color = constraintColor;
				if (intersectable && intersectable->intersecting) {
					lineItem.color = hoveredColor;
				}
				gameState->renderData.push_back(lineItem);

				if (selectable && selectable->selected) {
					middle::RenderItem selectItem;
					selectItem.type = middle::RenderItemType::RECTANGLE;
					selectItem.center = { 0,0,0 };
					float height = Vector3Distance(lineItem.linePointA, lineItem.linePointB);
					Vector3 lineDir = Vector3Normalize(lineItem.linePointB - lineItem.linePointA);
					selectItem.width = 1;
					selectItem.height = height;
					selectItem.length = 1;
					selectItem.color = selectionBoxColor;
					selectItem.transform.scale = { 1,1,1 };
					selectItem.transform.rotation = QuaternionFromVector3ToVector3({ 0,0,1 }, lineDir);
					selectItem.transform.translation = Vector3Scale(lineItem.linePointA + lineItem.linePointB, 0.5f);
					gameState->renderData.push_back(selectItem);
				}
			}

			// render hierarchy indicators, 
			if (gameState->editorState.creationMode == middle::CreationMode::LOOP_MODE
				&& loopSociety
				&& intersectable
				&& intersectable->intersecting) {
				for (middle::Id& id : loopSociety->loopMemberIds) {
					Vector3 childPos = middle::getShapePosition(gameState, id.index);
					middle::RenderItem childItem;
					childItem.type = middle::RenderItemType::TEXT;
					childItem.color = loopItemColor;
					childItem.center = childPos;
					childItem.text = "child";
					gameState->renderData.push_back(childItem);
				}

				if (loopSociety->parentLoopId.index != middle::UNASSIGNED) {
					middle::Id& parentId = loopSociety->parentLoopId;
					Vector3 parentPos = middle::getShapePosition(gameState, parentId.index);
					middle::RenderItem parentItem;
					parentItem.type = middle::RenderItemType::TEXT;
					parentItem.color = loopItemColor;
					parentItem.center = parentPos;
					parentItem.text = "parent";
					gameState->renderData.push_back(parentItem);
				}
			}

			auto reference = middle::getComponent<components::Reference>(shape);
			if (reference) {
				auto selectable = middle::getComponent<components::MouseSelectable>(shape);
				middle::RenderItem refItem;
				refItem.type = middle::RenderItemType::SPHERE;
				refItem.color = referenceColor;
				refItem.center = { position->posX, position->posY, position->posZ };
				refItem.radius = middle::DEF_RADIUS_REFERENCE_INDICATOR;
				if (intersectable && intersectable->intersecting) {
					refItem.color = hoveredColor;
				}
				gameState->renderData.push_back(refItem);

				if (selectable && selectable->selected) {
					middle::RenderItem selectItem;
					selectItem.type = middle::RenderItemType::RECTANGLE;
					selectItem.center = { 0,0,0 };
					selectItem.transform.translation = getShapePosition(gameState, i);
					selectItem.transform.scale = { 1,1,1 };
					selectItem.transform.rotation = { 0,0,0,0 };
					selectItem.width = refItem.radius * 4;
					selectItem.height = refItem.radius * 4;
					selectItem.length = refItem.radius * 4;
					selectItem.color = selectionBoxColor;
					gameState->renderData.push_back(selectItem);
				}
			}

			if (!reference && loopTag) {
				auto selectable = middle::getComponent<components::MouseSelectable>(shape);
				middle::RenderItem loopItem;
				loopItem.type = middle::RenderItemType::SPHERE;
				loopItem.center = middle::getShapePosition(gameState, i);
				loopItem.radius = middle::DEF_RADIUS_LOOP_INDICATOR;
				loopItem.color = loopColor;
				if (intersectable && intersectable->intersecting) {
					loopItem.color = hoveredColor;
				}
				gameState->renderData.push_back(loopItem);

				if (selectable && selectable->selected) {
					middle::RenderItem selectItem;
					selectItem.type = middle::RenderItemType::RECTANGLE;
					selectItem.center = { 0,0,0 };
					selectItem.transform.translation = loopItem.center;
					selectItem.transform.scale = { 1,1,1 };
					selectItem.transform.rotation = { 0,0,0,0 };
					selectItem.width = loopItem.radius * 4;
					selectItem.height = loopItem.radius * 4;
					selectItem.length = loopItem.radius * 4;
					selectItem.color = selectionBoxColor;
					gameState->renderData.push_back(selectItem);
				}

				return true;
			}

			auto system = middle::getComponent<components::SystemReference>(shape);
			if (system) {
				auto selectable = middle::getComponent<components::MouseSelectable>(shape);
				assert(position);
				middle::RenderItem systemItem;
				systemItem.type = middle::RenderItemType::SPHERE;
				systemItem.center = middle::getShapePosition(gameState, i);
				systemItem.radius = middle::DEF_RADIUS_SYSTEM;
				systemItem.color = systemColor;
				if (intersectable && intersectable->intersecting) {
					systemItem.color = hoveredColor;
				}
				gameState->renderData.push_back(systemItem);

				if (selectable && selectable->selected) {
					middle::RenderItem selectItem;
					selectItem.type = middle::RenderItemType::RECTANGLE;
					selectItem.center = { 0,0,0 };
					selectItem.transform.translation = systemItem.center;
					selectItem.transform.scale = { 1,1,1 };
					selectItem.transform.rotation = { 0,0,0,0 };
					selectItem.width = systemItem.radius * 4;
					selectItem.height = systemItem.radius * 4;
					selectItem.length = systemItem.radius * 4;
					selectItem.color = selectionBoxColor;
					gameState->renderData.push_back(selectItem);
				}
			}

			return true;
			});




	}
};

static middle::SystemRegistrar<EditorRenderSetupSystem> reg("EditorRenderSetupSystem");
