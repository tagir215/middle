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
#include "Layer.h"
#include "RuntimeHiddenTag.h"
#include "Button.h"
#include "TextureComponent.h"
#include "Inventory.h"

class UiRenderSetup : public middle::MiddleGameplaySystem {
public:
	UiRenderSetup() {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::RENDERING;
	}

	components::CompCache* rectangleCache;
	components::CompCache* circleCache;
	components::CompCache* textCache;
	components::CompCache* nonUiRectangleCache;
	components::CompCache* nonUiCircleCache;

	void init(middle::GameState* gameState) {
		rectangleCache = middle::newCompCache(gameState);
		rectangleCache->addType<components::Rectangle>();
		rectangleCache->addType<components::UiComponent>();
		rectangleCache->addType<components::Layer>();
		rectangleCache->addType<components::TextureComponent>(components::NOTINTERESTED);
		rectangleCache->addType<components::Inventory>(components::NOTINTERESTED);
		circleCache = middle::newCompCache(gameState);
		circleCache->addType<components::Circle>();
		circleCache->addType<components::UiComponent>();
		circleCache->addType<components::Layer>();
		circleCache->addType<components::TextureComponent>(components::NOTINTERESTED);
		circleCache->addType<components::BubbleComponent>(components::NOTINTERESTED);
		circleCache->addType<components::BubbleUnit>(components::NOTINTERESTED);
		textCache = middle::newCompCache(gameState);
		textCache->addType<components::Text>();
		textCache->addType<components::Button>();
		nonUiRectangleCache = middle::newCompCache(gameState);
		nonUiRectangleCache->addType<components::Rectangle>();
		nonUiRectangleCache->addType<components::Position>();
		nonUiRectangleCache->addType<components::UiComponent>(components::NOTINTERESTED);
		nonUiRectangleCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		nonUiRectangleCache->addType<components::TextureComponent>(components::NOTINTERESTED);
		nonUiCircleCache = middle::newCompCache(gameState);
		nonUiCircleCache->addType<components::Circle>();
		nonUiCircleCache->addType<components::Position>();
		nonUiCircleCache->addType<components::UiComponent>(components::NOTINTERESTED);
		nonUiCircleCache->addType<components::BubbleComponent>(components::NOTINTERESTED);
		nonUiCircleCache->addType<components::BubbleUnit>(components::NOTINTERESTED);
		nonUiCircleCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		nonUiCircleCache->addType<components::TextureComponent>(components::NOTINTERESTED);
	}


	void update(middle::GameState* gameState) override {

		auto rectangleIt = rectangleCache->begin<components::Rectangle>();
		auto uiCompIt = rectangleCache->begin<components::UiComponent>();
		auto rectangleLayerIt = rectangleCache->begin<components::Layer>();
		for (int i = 0; i < rectangleCache->getSize(); ++i) {
			auto rectangle = *rectangleIt;
			auto uiComp = *uiCompIt;
			auto layer = *rectangleLayerIt;
			auto& shape = middle::getShape(gameState, rectangleCache->relevantIdVector[i].index);
			Vector3 position = middle::getShapePosition(gameState, shape.id.index);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			Color color = intersectable && intersectable->intersectingTop ? bubbleColors::HOVERED_ITEM : bubbleColors::UI_BUTTON;
			auto rect = middle::getComponent<components::Rectangle>(shape);
			Color backgroundColor = bubbleColors::UI_BUTTON_BACKGROUND;

			if (uiComp->type == UiElementTypes::UI_BACKGROUND) {
				backgroundColor = bubbleColors::UI_BACKGROUND;
			}

			middle::RenderItem rectItem;
			rectItem.type = middle::RenderItemType::RECTANGLE;
			rectItem.color = color;
			rectItem.width = rect->width;
			rectItem.height = rect->height;
			rectItem.length = 0.01f;
			rectItem.center = position;
			rectItem.backgroundColor = backgroundColor;
			rectItem.disableDepthTest = true;
			rectItem.layer = layer->layer;
			auto scale = middle::getComponent<components::Scale>(shape);
			if (scale) {
				rectItem.scale = scale->scale;
			}
			gameState->renderData.push_back(rectItem);
		}

		auto circleIt = circleCache->begin<components::Circle>();
		auto circleLayerIt = circleCache->begin<components::Layer>();
		auto circleUiCompIt = circleCache->begin<components::UiComponent>();
		for (int i = 0; i < circleCache->getSize(); ++i){
			auto circle = *circleIt;
			auto layer = *circleLayerIt;
			auto uiComp = *circleUiCompIt;
			auto& shape = middle::getShape(gameState, circleCache->relevantIdVector[i].index);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			bool intersecting = intersectable && intersectable->intersectingTop;
			Color color = intersecting ? bubbleColors::HOVERED_ITEM : bubbleColors::UI_BUTTON;
			middle::RenderItem circleItem;
			circleItem.type = middle::RenderItemType::CIRCLE;
			circleItem.color = color;
			circleItem.backgroundColor = bubbleColors::UI_BUTTON_BACKGROUND;
			circleItem.radius = circle->radius;
			circleItem.center = middle::getShapePosition(gameState, shape.id.index);
			circleItem.disableDepthTest = true;
			circleItem.layer = layer->layer;
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



		
		// depth testable ui

		auto nonUiRectangleIt = nonUiRectangleCache->begin<components::Rectangle>();
		for (int i = 0; i < nonUiRectangleCache->getSize(); ++i) {
			auto rectangle = *nonUiRectangleIt;
			middle::Id id = nonUiRectangleCache->relevantIdVector[i];

			middle::RenderItem rectangleItem;
			rectangleItem.type = middle::RenderItemType::RECTANGLE;
			rectangleItem.center = middle::getShapePosition(gameState, id.index);
			rectangleItem.color = bubbleColors::UI_BUTTON;
			rectangleItem.width = rectangle->width;
			rectangleItem.height = rectangle->height;
			rectangleItem.length = 0.001f;
			gameState->renderData.push_back(rectangleItem);
		}


		auto nonUiCircleIt = nonUiCircleCache->begin<components::Circle>();
		auto nonUiCirclePositionIt = nonUiCircleCache->begin<components::Position>();
		for (int i = 0; i < nonUiCircleCache->getSize(); ++i){
			auto circle = *nonUiCircleIt;
			auto position = *nonUiCirclePositionIt;
			auto& shape = middle::getShape(gameState, nonUiCircleCache->relevantIdVector[i].index);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			bool intersecting = intersectable && intersectable->intersectingTop;
			Color color = intersecting ? bubbleColors::HOVERED_ITEM : bubbleColors::UI_BUTTON;
			middle::RenderItem circleItem;
			circleItem.type = middle::RenderItemType::CIRCLE;
			circleItem.color = color;
			circleItem.radius = circle->radius;
			circleItem.center = middle::getShapePosition(gameState, shape.id.index);
			gameState->renderData.push_back(circleItem);
		}

	}
};

static middle::SystemRegistrar<UiRenderSetup> reg("UiRenderSetup");
