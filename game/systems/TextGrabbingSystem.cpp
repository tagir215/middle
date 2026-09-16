#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "PuzzleTextUnit.h"
#include "Rectangle.h"
#include "component_utils.h"
#include "GlobalTransform.h"
#include "GrabbedTag.h"

class TextGrabbingSystem : public middle::MiddleGameplaySystem {
	components::CompCache* puzzleTextUnitCache;
	components::CompCache* grabbedCache;

	void init(middle::GameState* gameState) override {
		puzzleTextUnitCache = middle::newCompCache(gameState, systemName);
		puzzleTextUnitCache->addType<components::Rectangle>();
		puzzleTextUnitCache->addType<components::GlobalTransform>();
		puzzleTextUnitCache->addType<components::PuzzleTextUnit>();
		puzzleTextUnitCache->addType<components::GrabbedTag>(components::NOTINTERESTED);

		grabbedCache = middle::newCompCache(gameState, systemName);
		grabbedCache->addType<components::GrabbedTag>();
		grabbedCache->addType<components::PuzzleTextUnit>();
	}
	void update(middle::GameState* gameState) override {
		auto transformIt = puzzleTextUnitCache->begin<components::GlobalTransform>();
		auto rectIt = puzzleTextUnitCache->begin<components::Rectangle>();

		for (middle::Id& id : puzzleTextUnitCache->relevantIdVector) {
			auto transform = *transformIt;
			auto rect = *rectIt;

			// check intersection with mouse
			float axisX = rect->width * 0.5f;
			float axisZ = rect->height * 0.5f;
			Vector3& mousePos = gameState->input.mouseXZ_PlanePos;
			bool intersecting = mousePos.x > transform->pos.x - axisX && mousePos.x < transform->pos.x + axisX
				&& mousePos.z > transform->pos.z - axisZ && mousePos.z < transform->pos.z + axisZ;


			// draw debug rects
			middle::RenderItem item;
			item.type = middle::RenderItemType::RECTANGLE;
			item.center = transform->pos;
			item.width = rect->width;
			item.height = rect->height;
			item.color = intersecting ? RED : WHITE;
			gameState->renderData.push_back(item);

			if (intersecting && gameState->input.mouseClicked) {
				middle::Id copyId = middle::deepCopyShapeGlobalCoordinates(gameState, id);
				middle::EditorActionRemoveFromLoop(copyId.index).execute(gameState);
				middle::attachComponent<components::GrabbedTag>(gameState, copyId);
			}
		}

		for (middle::Id& id : grabbedCache->relevantIdVector) {
			if (gameState->input.mouseReleased) {
				auto del = std::make_shared<middle::EditorActionDeleteSingle>(id);
				middle::queueAction(gameState, del);
				continue;
			}

			middle::moveShape(gameState, id.index, gameState->input.mouseXZ_PlaneVelocity * gameState->frameTime);
		}
	}
};

static middle::SystemRegistrar<TextGrabbingSystem> reg("TextGrabbingSystem");
