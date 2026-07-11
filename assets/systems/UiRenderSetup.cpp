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
#include "Highlight.h"

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
	components::CompCache* nonUiTextCache;
	components::CompCache* procedureContainerCache;
	components::CompCache* circleHighlightCache;
	components::CompCache* rectHighlightCache;

	void init(middle::GameState* gameState) {
		rectangleCache = middle::newCompCache(gameState, systemName);
		rectangleCache->addType<components::Rectangle>();
		rectangleCache->addType<components::UiComponent>();
		rectangleCache->addType<components::Layer>();
		rectangleCache->addType<components::TextureComponent>(components::NOTINTERESTED);
		rectangleCache->addType<components::Inventory>(components::NOTINTERESTED);
		rectangleCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		circleCache = middle::newCompCache(gameState, systemName);
		circleCache->addType<components::Circle>();
		circleCache->addType<components::UiComponent>();
		circleCache->addType<components::Layer>();
		circleCache->addType<components::TextureComponent>(components::NOTINTERESTED);
		circleCache->addType<components::BubbleComponent>(components::NOTINTERESTED);
		circleCache->addType<components::BubbleUnit>(components::NOTINTERESTED);
		circleCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		textCache = middle::newCompCache(gameState, systemName);
		textCache->addType<components::Text>();
		textCache->addType<components::UiComponent>();
		textCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		nonUiRectangleCache = middle::newCompCache(gameState, systemName);
		nonUiRectangleCache->addType<components::Rectangle>();
		nonUiRectangleCache->addType<components::Position>();
		nonUiRectangleCache->addType<components::UiComponent>(components::NOTINTERESTED);
		nonUiRectangleCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		nonUiRectangleCache->addType<components::TextureComponent>(components::NOTINTERESTED);
		nonUiCircleCache = middle::newCompCache(gameState, systemName);
		nonUiCircleCache->addType<components::Circle>();
		nonUiCircleCache->addType<components::Position>();
		nonUiCircleCache->addType<components::UiComponent>(components::NOTINTERESTED);
		nonUiCircleCache->addType<components::BubbleComponent>(components::NOTINTERESTED);
		nonUiCircleCache->addType<components::BubbleUnit>(components::NOTINTERESTED);
		nonUiCircleCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		nonUiCircleCache->addType<components::TextureComponent>(components::NOTINTERESTED);
		nonUiTextCache = middle::newCompCache(gameState, systemName);
		nonUiTextCache->addType<components::Text>();
		nonUiTextCache->addType<components::Button>();
		nonUiTextCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		nonUiTextCache->addType<components::UiComponent>(components::NOTINTERESTED);
		procedureContainerCache = middle::newCompCache(gameState, systemName);
		procedureContainerCache->addType<components::ProcedureContainer>();
		circleHighlightCache = middle::newCompCache(gameState, systemName);
		circleHighlightCache->addType<components::Highlight>();
		circleHighlightCache->addType<components::Circle>();
		circleHighlightCache->addType<components::Layer>();
		rectHighlightCache = middle::newCompCache(gameState, systemName);
		rectHighlightCache->addType<components::Highlight>();
		rectHighlightCache->addType<components::Rectangle>();
		rectHighlightCache->addType<components::Layer>();
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
			if (uiComp->type == UiElementTypes::PROCEDURE_RECT) {
				//color = bubbleColors::PROCEDURE_RECT;
				//backgroundColor = bubbleColors::PROCEDURE_RECT;
				continue;
			}
			if (uiComp->type == UiElementTypes::PROCEDURE_BACKGROUND) {
				//backgroundColor = bubbleColors::PROCEDURE_BACKGROUND;
				continue;
			}
			if (uiComp->type == UiElementTypes::PROCEDURE_SCOPE) {
				//backgroundColor = bubbleColors::PROCEDURE_SCOPE;
				color = bubbleColors::PROCEDURE_SCOPE;
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
			if (uiComp->type == UiElementTypes::PROCEDURE_INPUT) {
				color = bubbleColors::PROCEDURE_RECT;
			}
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
			if (outputVariable) {
				text->text = outputVariable->label;
			}
			middle::RenderItem textItem;
			Vector3 pos = middle::getShapePosition(gameState, shape.id.index);
			textItem.type = middle::RenderItemType::TEXT;
			textItem.text = text->text;
			Vector3 offset = { text->offsetX, text->offsetY, text->offsetZ };
			textItem.center = pos + offset;
			textItem.color.r = text->fontColorR;
			textItem.color.g = text->fontColorG;
			textItem.color.b = text->fontColorB;
			textItem.color.a = text->fontColorA;
			textItem.fontSize = text->fontSize;
			textItem.disableDepthTest = true;
			gameState->renderData.push_back(textItem);
		}

		auto nonUiTextIt = nonUiTextCache->begin<components::Text>();
		for (int i = 0; i < nonUiTextCache->getSize(); ++i) {
			auto text = *nonUiTextIt;
			auto& shape = middle::getShape(gameState, nonUiTextCache->relevantIdVector[i].index);
			middle::RenderItem textItem;
			Vector3 pos = middle::getShapePosition(gameState, shape.id.index);
			textItem.type = middle::RenderItemType::TEXT;
			textItem.text = text->text;
			Vector3 offset = { text->offsetX, text->offsetY, text->offsetZ };
			textItem.center = pos + offset;
			textItem.color.r = text->fontColorR;
			textItem.color.g = text->fontColorG;
			textItem.color.b = text->fontColorB;
			textItem.color.a = text->fontColorA;
			textItem.fontSize = text->fontSize;
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



		// PROCEDURE STUFF


		if (procedureContainerCache->getSize() > 0) {
			// execution iterator rendering
			auto procIt = procedureContainerCache->begin<components::ProcedureContainer>();
			auto procContainer = *procIt;

			bool procedureInAction = procContainer && procContainer->procedureTransitionStack.size() > 0;
			if (procContainer && procContainer->activeBlock.index != middle::UNASSIGNED && procedureInAction) {
				auto& activeBlockShape = middle::getShape(gameState, procContainer->activeBlock.index);
				Vector3 position = middle::getShapePosition(gameState, activeBlockShape.id.index);
				auto rect = middle::getComponent<components::Rectangle>(activeBlockShape);
				middle::RenderItem activeBlockItem;
				activeBlockItem.type = middle::RenderItemType::RECTANGLE;
				activeBlockItem.backgroundColor = bubbleColors::HIGHLIGHT_COLOR_2;
				activeBlockItem.width = rect->width;
				activeBlockItem.height = rect->height;
				activeBlockItem.length = 0.2f;
				activeBlockItem.center = position;
				activeBlockItem.layer = 3;
				activeBlockItem.disableDepthTest = true;
				gameState->renderData.push_back(activeBlockItem);
			}
		}


		auto circleHightlightIt = circleHighlightCache->begin<components::Circle>();
		auto circleHightlightLayerIt = circleHighlightCache->begin<components::Layer>();
		for (int i = 0; i < circleHighlightCache->getSize(); ++i) {
			auto circle = *circleHightlightIt;
			auto layer = *circleHightlightLayerIt;
			middle::RenderItem highlight;
			Vector3 pos = middle::getShapePosition(gameState, circleHighlightCache->relevantIdVector[i].index);
			highlight.type = middle::RenderItemType::CYLINDER;
			highlight.center = pos;
			highlight.color = bubbleColors::HIGHLIGHT_COLOR;
			highlight.radius = circle->radius;
			highlight.ringRadius = circle->radius;
			highlight.length = 0.1f;
			highlight.layer = layer->layer;
			highlight.disableDepthTest = true;
			gameState->renderData.push_back(highlight);
		}

		auto rectHighlightIt = rectHighlightCache->begin<components::Rectangle>();
		auto rectHighlightLayerIt = rectHighlightCache->begin<components::Layer>();
		for (int i = 0; i < rectHighlightCache->getSize(); ++i) {
			auto rect = *rectHighlightIt;
			auto layer = *rectHighlightLayerIt;
			middle::RenderItem highlight;
			Vector3 pos = middle::getShapePosition(gameState, rectHighlightCache->relevantIdVector[i].index);
			highlight.type = middle::RenderItemType::RECTANGLE;
			highlight.transform.translation = pos;
			highlight.color = bubbleColors::HIGHLIGHT_COLOR;
			highlight.width = rect->width;
			highlight.height = rect->height;
			highlight.disableDepthTest = true;
			highlight.layer = layer->layer;
			gameState->renderData.push_back(highlight);
		}


	}
};

static middle::SystemRegistrar<UiRenderSetup> reg("UiRenderSetup");
