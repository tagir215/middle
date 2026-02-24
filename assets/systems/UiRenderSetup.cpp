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

class UiRenderSetup : public middle::MiddleGameplaySystem {
public:
	UiRenderSetup() {
		systemModeType = middle::SystemModeType::ENGINE;
	}

	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {
			auto rectangle = middle::getComponent<components::Rectangle>(shape);
			auto text = middle::getComponent<components::Text>(shape);
			auto circle = middle::getComponent<components::Circle>(shape);
			auto uiComponent = middle::getComponent<components::UiComponent>(shape);
			auto inputVariable = middle::getComponent<components::InputVariable>(shape);
			auto outputVariable = middle::getComponent<components::OutputVariable>(shape);

			if (!rectangle && !circle && !uiComponent && !inputVariable && !outputVariable)
				return true;

			if (inputVariable) {
				text->text = inputVariable->label;
			}
			if (outputVariable) {
				text->text = outputVariable->label;
			}

			if (rectangle) {
				Vector3 position = middle::getShapePosition(gameState, shape.id.index);

				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				Color color = intersectable && intersectable->intersectingTop ? WHITE : Color{ 200,200,200,200 };

				std::vector<Vector3>vertices = middle::getRectVertices(gameState, shape.id);

				middle::RenderItem line1;
				line1.type = middle::RenderItemType::LINE;
				line1.linePointA = vertices[0];
				line1.linePointB = vertices[1];
				line1.color = color;
				gameState->renderData.push_back(line1);
				middle::RenderItem line2;
				line2.type = middle::RenderItemType::LINE;
				line2.linePointA = vertices[1];
				line2.linePointB = vertices[2];
				line2.color = color;
				gameState->renderData.push_back(line2);
				middle::RenderItem line3;
				line3.type = middle::RenderItemType::LINE;
				line3.linePointA = vertices[2];
				line3.linePointB = vertices[3];
				line3.color = color;
				gameState->renderData.push_back(line3);
				middle::RenderItem line4;
				line4.type = middle::RenderItemType::LINE;
				line4.linePointA = vertices[3];
				line4.linePointB = vertices[0];
				line4.color = color;
				gameState->renderData.push_back(line4);
			}

			if (text) {
				middle::RenderItem textItem;
				Vector3 pos = middle::getShapePosition(gameState, shape.id.index);
				textItem.type = middle::RenderItemType::TEXT;
				textItem.text = text->text;
				Vector3 offset = { text->offsetX, text->offsetY, text->offsetZ };
				textItem.center = pos + offset;
				gameState->renderData.push_back(textItem);
			}

			if (circle) {
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				bool intersecting = intersectable && intersectable->intersectingTop;

				Color color = intersecting ? WHITE : Color{ 200,200,200,200 };

				middle::RenderItem circleItem;
				circleItem.type = middle::RenderItemType::CIRCLE;
				circleItem.color = color;
				circleItem.radius = circle->radius;
				circleItem.center = middle::getShapePosition(gameState, shape.id.index);
				gameState->renderData.push_back(circleItem);
			}

			return true;
			});
	}
};

static middle::SystemRegistrar<UiRenderSetup> reg("UiRenderSetup");
