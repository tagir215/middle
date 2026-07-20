#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "PuzzleTextUnit.h"
#include "Rectangle.h"
#include "component_utils.h"
#include "Position.h"
#include "GrabbedTag.h"

class TextGrabbingSystem : public middle::MiddleGameplaySystem {
	components::CompCache* puzzleTextUnitCache;
	components::CompCache* grabbedCache;

	void init(middle::GameState* gameState) override {
		puzzleTextUnitCache = middle::newCompCache(gameState, systemName);
		puzzleTextUnitCache->addType<components::Rectangle>();
		puzzleTextUnitCache->addType<components::Position>();
		puzzleTextUnitCache->addType<components::PuzzleTextUnit>();
		puzzleTextUnitCache->addType<components::GrabbedTag>(components::NOTINTERESTED);

		grabbedCache = middle::newCompCache(gameState, systemName);
		grabbedCache->addType<components::GrabbedTag>();
		grabbedCache->addType<components::PuzzleTextUnit>();
	}
	void update(middle::GameState* gameState) override {
		auto posIt = puzzleTextUnitCache->begin<components::Position>();
		auto rectIt = puzzleTextUnitCache->begin<components::Rectangle>();

		for (middle::Id& id : puzzleTextUnitCache->relevantIdVector) {
			auto pos = *posIt;
			auto rect = *rectIt;

			// check intersection with mouse
			float axisX = rect->width * 0.5f;
			float axisZ = rect->height * 0.5f;
			Vector3& mousePos = gameState->input.mouseXZ_PlanePos;
			bool intersecting = mousePos.x > pos->posX - axisX && mousePos.x < pos->posX + axisX
				&& mousePos.z > pos->posZ - axisZ && mousePos.z < pos->posZ + axisZ;


			// draw debug rects
			middle::RenderItem item;
			item.type = middle::RenderItemType::RECTANGLE;
			item.center = { pos->posX, pos->posY, pos->posZ };
			item.width = rect->width;
			item.height = rect->height;
			item.color = intersecting ? RED : WHITE;
			gameState->renderData.push_back(item);

			if (intersecting && gameState->input.mouseClicked) {
				middle::Id copyId = middle::deepCopyShape(gameState, id.index);
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
