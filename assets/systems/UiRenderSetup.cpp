#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "Rectangle.h"
#include "Text.h"
#include "MouseIntersectable.h"
#include "Offset.h"
#include "Circle.h"
#include "UiComponent.h"
#include "InputVariable.h"
#include "OutputVariable.h"
#include "Triangle.h"
#include "ProcedureContainer.h"
#include "ScopeComponent.h"
#include "bubble_colors.h"
#include "Scale.h"

class UiRenderSetup : public middle::MiddleGameplaySystem {
public:
	UiRenderSetup() {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::RENDERING;
	}

	components::CompCache* rectangleCache;
	components::CompCache* circleCache;
	components::CompCache* textCache;

	void init(middle::GameState* gameState) {
		rectangleCache = middle::newCompCache(gameState);
		rectangleCache->addType<components::Rectangle>();
		rectangleCache->addType<components::UiComponent>();
		circleCache = middle::newCompCache(gameState);
		circleCache->addType<components::Circle>();
		circleCache->addType<components::UiComponent>();
		textCache = middle::newCompCache(gameState);
		textCache->addType<components::Text>();
		textCache->addType<components::UiComponent>();
	}


	void update(middle::GameState* gameState) override {

		auto rectangleIt = rectangleCache->begin<components::Rectangle>();
		for (int i = 0; i < rectangleCache->getSize(); ++i) {
			auto rectangle = *rectangleIt;
			auto& shape = middle::getShape(gameState, rectangleCache->relevantIdVector[i].index);
			Vector3 position = middle::getShapePosition(gameState, shape.id.index);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			Color color = intersectable && intersectable->intersectingTop ? bubbleColors::HOVERED_ITEM : bubbleColors::UI_BUTTON;
			auto rect = middle::getComponent<components::Rectangle>(shape);

			middle::RenderItem rectItem;
			rectItem.type = middle::RenderItemType::RECTANGLE;
			rectItem.color = color;
			rectItem.width = rect->width;
			rectItem.height = rect->height;
			rectItem.length = 0.01f;
			rectItem.center = position;
			rectItem.backgroundColor = bubbleColors::UI_BUTTON_BACKGROUND;
			rectItem.disableDepthTest = true;
			auto scale = middle::getComponent<components::Scale>(shape);
			if (scale) {
				rectItem.scale = scale->scale;
			}
			gameState->renderData.push_back(rectItem);
		}

		auto circleIt = circleCache->begin<components::Circle>();
		for (int i = 0; i < circleCache->getSize(); ++i){
			auto circle = *circleIt;
			auto& shape = middle::getShape(gameState, circleCache->relevantIdVector[i].index);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			bool intersecting = intersectable && intersectable->intersectingTop;
			Color color = intersecting ? bubbleColors::HOVERED_ITEM : bubbleColors::UI_BUTTON;
			middle::RenderItem circleItem;
			circleItem.type = middle::RenderItemType::CIRCLE;
			circleItem.color = color;
			circleItem.radius = circle->radius;
			circleItem.center = middle::getShapePosition(gameState, shape.id.index);
			circleItem.disableDepthTest = true;
			gameState->renderData.push_back(circleItem);
		}

		auto textIt = textCache->begin<components::Text>();
		for (int i = 0; i < textCache->getSize(); ++i) {
			auto text = *textIt;
			auto& shape = middle::getShape(gameState, textCache->relevantIdVector[i].index);
			auto inputVariable = middle::getComponent<components::InputVariable>(shape);
			auto outputVariable = middle::getComponent<components::OutputVariable>(shape);
			if (inputVariable) {
				text->text = inputVariable->label;
			}
			if (outputVariable) {
				text->text = outputVariable->label;
			}
			middle::RenderItem textItem;
			Vector3 pos = middle::getShapePosition(gameState, shape.id.index);
			textItem.type = middle::RenderItemType::TEXT;
			textItem.text = text->text;
			Vector3 offset = { text->offsetX, text->offsetY, text->offsetZ };
			textItem.center = pos + offset;
			textItem.color = bubbleColors::UI_TEXT;
			gameState->renderData.push_back(textItem);
		}
	}
};

static middle::SystemRegistrar<UiRenderSetup> reg("UiRenderSetup");
