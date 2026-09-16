#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "Sphere.h"
#include "GlobalTransform.h"
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
#include "EditorText.h"
#include "HiddenTag.h"
#include "ConfigComponent.h"
#include "EditorConfigs.h"
#include "middle_math.h"
#include "Position.h"
#include "component_utils.h"
#include "IntersectingTag.h"

class EditorRenderSetupSystem : public middle::MiddleGameplaySystem {
public:
	EditorRenderSetupSystem() {
		systemUpdateType = middle::SystemUpdateType::RENDERING;
		systemModeType = middle::SystemModeType::EDITOR;
	}

	components::CompCache* gridCache;
	components::CompCache* configCache;
	components::CompCache* importRefCache;
	components::CompCache* nodeCache;
	components::CompCache* constraintCache;
	components::CompCache* loopTagCache;
	components::CompCache* hierarchyCache;
	components::CompCache* systemRefCache;
	components::CompCache* selectableSphereCache;
	components::CompCache* selectableLineCache;
	components::CompCache* positionCache;
	components::CompCache* oPosCache;
	components::CompCache* textCache;

	void init(middle::GameState* gameState) {
		gridCache = middle::newCompCache(gameState, systemName);
		gridCache->addType<components::EditorConfigs>();
		gridCache->addType<components::HiddenTag>(components::NOTINTERESTED);
		configCache = middle::newCompCache(gameState, systemName);
		configCache->addType<components::ConfigComponent>();
		configCache->addType<components::GlobalTransform>();
		configCache->addType<components::HiddenTag>(components::NOTINTERESTED);
		importRefCache = middle::newCompCache(gameState, systemName);
		importRefCache->addType<components::Reference>();
		importRefCache->addType<components::GlobalTransform>();
		importRefCache->addType<components::HiddenTag>(components::NOTINTERESTED);
		nodeCache = middle::newCompCache(gameState, systemName);
		nodeCache->addType<components::Sphere>();
		nodeCache->addType<components::GlobalTransform>();
		nodeCache->addType<components::HiddenTag>(components::NOTINTERESTED);
		constraintCache = middle::newCompCache(gameState, systemName);
		constraintCache->addType<components::Constraint>();
		constraintCache->addType<components::HiddenTag>(components::NOTINTERESTED);
		loopTagCache = middle::newCompCache(gameState, systemName);
		loopTagCache->addType<components::LoopSociety>();
		loopTagCache->addType<components::LoopTag>();
		loopTagCache->addType<components::GlobalTransform>();
		loopTagCache->addType<components::HiddenTag>(components::NOTINTERESTED);
		loopTagCache->addType<components::Reference>(components::NOTINTERESTED);
		hierarchyCache = middle::newCompCache(gameState, systemName);
		hierarchyCache->addType<components::LoopSociety>();
		hierarchyCache->addType<components::IntersectingTag>();
		systemRefCache = middle::newCompCache(gameState, systemName);
		systemRefCache->addType<components::LoopSociety>();
		systemRefCache->addType<components::SystemReference>();
		systemRefCache->addType<components::GlobalTransform>();
		systemRefCache->addType<components::HiddenTag>(components::NOTINTERESTED);
		selectableSphereCache = middle::newCompCache(gameState, systemName);
		selectableSphereCache->addType<components::MouseSelectable>();
		selectableSphereCache->addType<components::Sphere>();
		selectableSphereCache->addType<components::GlobalTransform>();
		selectableSphereCache->addType<components::HiddenTag>(components::NOTINTERESTED);
		selectableLineCache = middle::newCompCache(gameState, systemName);
		selectableLineCache->addType<components::MouseSelectable>();
		selectableLineCache->addType<components::Constraint>();
		selectableLineCache->addType<components::HiddenTag>(components::NOTINTERESTED);
		positionCache = middle::newCompCache(gameState, systemName);
		positionCache->addType<components::GlobalTransform>();
		textCache = middle::newCompCache(gameState, systemName);
		textCache->addType<components::EditorText>();
		textCache->addType<components::GlobalTransform>();

		oPosCache = middle::newCompCache(gameState, systemName);
		oPosCache->addType<components::Position>();
	}

	void replaceMan(middle::GameState* gameState) {
		auto ps = oPosCache->begin<components::Position>();
		for (middle::Id& id : oPosCache->relevantIdVector) {
			middle::queueComponentDeletion<components::Position>(gameState, id);
		}
	}

	void update(middle::GameState* gameState) override {

		replaceMan(gameState);

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

		auto transformIt = positionCache->begin<components::GlobalTransform>();
		for (int i = 0; i < positionCache->getSize(); ++i) {
			auto transform = *transformIt;
			middle::RenderItem sphereItem;
			sphereItem.type = middle::RenderItemType::SPHERE;
			sphereItem.radius = 2;
			sphereItem.center = transform->pos;
			sphereItem.color = { 150,150,150,255 };
			sphereItem.disableDepthTest = true;
			gameState->renderData.push_back(sphereItem);
		}

		// drawing grid
		auto gridIt = gridCache->begin<components::EditorConfigs>();
		for (int i = 0; i < gridCache->getSize(); ++i) {
			auto editorConfigs = *gridIt;

			if (editorConfigs->gridSize == 0)
				continue;
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


		auto configIt = configCache->begin<components::ConfigComponent>();
		for (int i = 0; i < configCache->getSize(); ++i) {
			auto config = *configIt;
			auto& shape = middle::getShape(gameState, configCache->relevantIdVector[i].index);

			middle::RenderItem configSphere;
			configSphere.type = middle::RenderItemType::SPHERE;
			configSphere.center = middle::getGlobalPosition(gameState, shape.id.index);
			configSphere.radius = middle::DEF_RADIUS_SYSTEM;
			configSphere.color = configColor;
			auto intersecting = middle::getComponent<components::IntersectingTag>(shape);
			if (intersecting) {
				configSphere.color = hoveredColor;
			}
			gameState->renderData.push_back(configSphere);
		}


		auto nodeIt = nodeCache->begin<components::Sphere>();
		for (int i = 0; i < nodeCache->getSize(); ++i) {
			auto sphere = *nodeIt;
			auto& shape = middle::getShape(gameState, nodeCache->relevantIdVector[i].index);
			middle::RenderItem sphereItem;
			sphereItem.type = middle::RenderItemType::SPHERE;
			sphereItem.radius = sphere->radius;
			sphereItem.center = middle::getGlobalPosition(gameState, shape.id.index);
			sphereItem.color = jointColor;
			auto intersecting = middle::getComponent<components::IntersectingTag>(shape);
			if (intersecting) {
				sphereItem.color = hoveredColor;
			}
			gameState->renderData.push_back(sphereItem);
		}


		auto constraintIt = constraintCache->begin<components::Constraint>();
		for (int i = 0; i < constraintCache->getSize(); ++i) {
			auto constraint = *constraintIt;
			auto& shape = middle::getShape(gameState, constraintCache->relevantIdVector[i].index);
			middle::RenderItem lineItem;
			lineItem.type = middle::RenderItemType::LINE;
			lineItem.linePointA = getGlobalPosition(gameState, constraint->idA.index);
			lineItem.linePointB = getGlobalPosition(gameState, constraint->idB.index);
			lineItem.color = constraintColor;
			auto intersecting = middle::getComponent<components::IntersectingTag>(shape);
			if (intersecting) {
				lineItem.color = hoveredColor;
			}
			gameState->renderData.push_back(lineItem);
		}

		if (gameState->editorState.creationMode == middle::CreationMode::LOOP_MODE) {
			auto hierarchyIntersectableIt = hierarchyCache->begin<components::IntersectingTag>();
			for (int i = 0; i < hierarchyCache->getSize(); ++i) {
				auto intersectable = *hierarchyIntersectableIt;
				std::vector<middle::Id>children;
				middle::getChildren(gameState, hierarchyCache->relevantIdVector[i], children);
				middle::Id parentId = middle::getParent(gameState, hierarchyCache->relevantIdVector[i]);
				for (middle::Id& id : children) {
					Vector3 childPos = middle::getGlobalPosition(gameState, id.index);
					middle::RenderItem childItem;
					childItem.type = middle::RenderItemType::TEXT;
					childItem.color = loopItemColor;
					childItem.center = childPos;
					childItem.text = "child";
					gameState->renderData.push_back(childItem);

				}
				if (parentId.index != middle::UNASSIGNED) {
					Vector3 parentPos = middle::getGlobalPosition(gameState, parentId.index);
					middle::RenderItem parentItem;
					parentItem.type = middle::RenderItemType::TEXT;
					parentItem.color = loopItemColor;
					parentItem.center = parentPos;
					parentItem.text = "parent";
					gameState->renderData.push_back(parentItem);
				}
			}
		}

		auto importRefGlobalTransformIt = importRefCache->begin<components::GlobalTransform>();
		for (int i = 0; i < importRefCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, importRefCache->relevantIdVector[i].index);
			auto transform = *importRefGlobalTransformIt;

			auto selectable = middle::getComponent<components::MouseSelectable>(shape);
			middle::RenderItem refItem;
			refItem.type = middle::RenderItemType::SPHERE;
			refItem.color = referenceColor;
			refItem.center = transform->pos;
			refItem.radius = middle::DEF_RADIUS_REFERENCE_INDICATOR;
			auto intersecting = middle::getComponent<components::IntersectingTag>(shape);
			if (intersecting) {
				refItem.color = hoveredColor;
			}
			gameState->renderData.push_back(refItem);
		}

		auto systemGlobalTransformIt = systemRefCache->begin<components::GlobalTransform>();
		for (int i = 0; i < systemRefCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, systemRefCache->relevantIdVector[i].index);
			auto transform = *systemGlobalTransformIt;
			middle::RenderItem systemItem;
			systemItem.type = middle::RenderItemType::SPHERE;
			systemItem.center = transform->pos;
			systemItem.radius = middle::DEF_RADIUS_SYSTEM;
			systemItem.color = systemColor;
			auto intersecting = middle::getComponent<components::IntersectingTag>(shape);
			if (intersecting) {
				systemItem.color = hoveredColor;
			}
			gameState->renderData.push_back(systemItem);
		}

		for (int i = 0; i < loopTagCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, loopTagCache->relevantIdVector[i].index);
			middle::RenderItem loopItem;
			loopItem.type = middle::RenderItemType::SPHERE;
			loopItem.center = middle::getGlobalPosition(gameState, shape.id.index);
			loopItem.radius = middle::DEF_RADIUS_LOOP_INDICATOR;
			loopItem.color = loopColor;
			loopItem.disableDepthTest = true;
			loopItem.layer = 6;
			auto intersecting = middle::getComponent<components::IntersectingTag>(shape);
			if (intersecting) {
				loopItem.color = hoveredColor;
			}
			gameState->renderData.push_back(loopItem);
		}


		auto selectableSphereIt = selectableSphereCache->begin<components::MouseSelectable>();
		auto selectableSphere = selectableSphereCache->begin<components::Sphere>();
		auto selectableSphereGlobalTransform = selectableSphereCache->begin<components::GlobalTransform>();
		for (int i = 0; i < selectableSphereCache->getSize(); ++i) {
			auto selectable = *selectableSphereIt;
			auto sphere = *selectableSphere;
			auto transform = *selectableSphereGlobalTransform;
			if (!selectable->selected) {
				continue;
			}

			middle::RenderItem selectItem;
			selectItem.type = middle::RenderItemType::RECTANGLE;
			selectItem.center = { 0,0,0 };
			selectItem.transform.translation = transform->pos;
			selectItem.transform.scale = { 1,1,1 };
			selectItem.transform.rotation = { 0,0,0,0 };
			selectItem.width = sphere->radius * 4;
			selectItem.height = sphere->radius * 4;
			selectItem.length = sphere->radius * 4;
			selectItem.color = selectionBoxColor;
			selectItem.disableDepthTest = true;
			selectItem.layer = 6;
			gameState->renderData.push_back(selectItem);
		}

		auto selectableLineIt = selectableLineCache->begin<components::MouseSelectable>();
		auto selectableConstraintIt = selectableLineCache->begin<components::Constraint>();
		for (int i = 0; i < selectableLineCache->getSize(); ++i) {
			auto selectable = *selectableLineIt;
			auto constraint = *selectableConstraintIt;
			if (!selectable->selected) {
				continue;
			}

			Vector3 linePointA = middle::getGlobalPosition(gameState, constraint->idA.index);
			Vector3 linePointB = middle::getGlobalPosition(gameState, constraint->idB.index);

			middle::RenderItem selectItem;
			selectItem.type = middle::RenderItemType::RECTANGLE;
			selectItem.center = { 0,0,0 };
			float height = Vector3Distance(linePointA, linePointB);
			Vector3 lineDir = Vector3Normalize(linePointB - linePointA);
			selectItem.width = 1;
			selectItem.height = height;
			selectItem.length = 1;
			selectItem.color = selectionBoxColor;
			selectItem.transform.scale = { 1,1,1 };
			selectItem.transform.rotation = QuaternionFromVector3ToVector3({ 0,0,1 }, lineDir);
			selectItem.transform.translation = Vector3Scale(linePointA + linePointB, 0.5f);
			gameState->renderData.push_back(selectItem);
		}

		const float editorTextSize = 10;
		const Color editorTextColor = WHITE;

		auto textIt = textCache->begin<components::EditorText>();
		auto textGlobalTransformIt = textCache->begin<components::GlobalTransform>();
		for (int i = 0; i < textCache->getSize(); ++i) {
			auto text = *textIt;
			auto transform = *textGlobalTransformIt;
			middle::RenderItem textItem;
			textItem.type = middle::RenderItemType::TEXT;
			textItem.center = { 0,0,0 };
			textItem.transform.translation = transform->pos;
			textItem.transform.scale = transform->scale;
			textItem.transform.rotation = transform->rotation;
			textItem.text = text->text;
			textItem.fontSize = editorTextSize;
			textItem.color = editorTextColor;
			gameState->renderData.push_back(textItem);
		}

	}
};

static middle::SystemRegistrar<EditorRenderSetupSystem> reg("EditorRenderSetupSystem");
