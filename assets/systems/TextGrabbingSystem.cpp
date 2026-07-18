#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "PuzzleTextUnit.h"
#include "Rectangle.h"
#include "component_utils.h"
#include "Position.h"

class TextGrabbingSystem : public middle::MiddleGameplaySystem {
	components::CompCache* puzzleTextUnitCache;

	void init(middle::GameState* gameState) override {
		puzzleTextUnitCache = middle::newCompCache(gameState, systemName);
		puzzleTextUnitCache->addType<components::Rectangle>();
		puzzleTextUnitCache->addType<components::Position>();
	}
	void update(middle::GameState* gameState) override {
		auto posIt = puzzleTextUnitCache->begin<components::Position>();
		auto rectIt = puzzleTextUnitCache->begin<components::Rectangle>();

		// draw debug rects
		for (middle::Id& id : puzzleTextUnitCache->relevantIdVector) {
			auto pos = *posIt;
			auto rect = *rectIt;
			middle::RenderItem item;
			item.type = middle::RenderItemType::RECTANGLE;
			item.center = { pos->posX, pos->posY, pos->posZ };
			item.width = rect->width;
			item.height = rect->height;
			gameState->renderData.push_back(item);
		}
	}
};

static middle::SystemRegistrar<TextGrabbingSystem> reg("TextGrabbingSystem");
