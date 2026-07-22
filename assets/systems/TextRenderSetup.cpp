#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "Text.h"
#include "GlobalTransform.h"
#include "HiddenTag.h"

class TextRenderSetup : public middle::MiddleGameplaySystem {
	components::CompCache* textCache;

	void init(middle::GameState* gameState) override {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::RENDERING;

		textCache = middle::newCompCache(gameState, systemName);
		textCache->addType<components::Text>();
		textCache->addType<components::GlobalTransform>();
		textCache->addType<components::HiddenTag>(components::NOTINTERESTED);
	}
	void update(middle::GameState* gameState) override {

		auto textIt = textCache->begin<components::Text>();
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
			textItem.fontSize = text->fontSize;
			textItem.color.r = text->fontColorR;
			textItem.color.g = text->fontColorG;
			textItem.color.b = text->fontColorB;
			textItem.color.a = text->fontColorA;
			gameState->renderData.push_back(textItem);
		}
	}
};

static middle::SystemRegistrar<TextRenderSetup> reg("TextRenderSetup");
