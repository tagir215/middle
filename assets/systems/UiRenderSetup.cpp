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

class UiRenderSetup : public middle::MiddleGameplaySystem {
public:
	UiRenderSetup() {
		systemModeType = middle::SystemModeType::ENGINE;
	}

	void drawRect(middle::GameState* gameState, const std::vector<Vector3>& vertices, const Color& color) {
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

	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {
			auto rectangle = middle::getComponent<components::Rectangle>(shape);
			auto text = middle::getComponent<components::Text>(shape);
			auto circle = middle::getComponent<components::Circle>(shape);
			auto uiComponent = middle::getComponent<components::UiComponent>(shape);
			auto inputVariable = middle::getComponent<components::InputVariable>(shape);
			auto outputVariable = middle::getComponent<components::OutputVariable>(shape);
			auto procedure = middle::getComponent<components::ProcedureContainer>(shape);

			if (!rectangle && !circle && !uiComponent && !inputVariable && !outputVariable && !procedure)
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

				drawRect(gameState, vertices, color);
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

			if (procedure) {
				if (procedure->activeScope.index != middle::UNASSIGNED) {
					auto& activeScope = middle::getShape(gameState, procedure->activeScope.index);
					auto scope = middle::getComponent<components::ScopeComponent>(activeScope);
					std::vector<middle::Id>children;
					middle::getChildren(gameState, activeScope.id, children);
					auto& activeBlock = middle::getShape(gameState, children[scope->currentIndex].index);
					auto rectBlock = middle::getComponent<components::Rectangle>(activeBlock);
					Vector3 pos = middle::getShapePosition(gameState, activeBlock.id.index);
					middle::RenderItem rect;
					std::vector<Vector3>vertices = middle::getRectVertices(gameState, activeBlock.id);
					drawRect(gameState, vertices, GREEN);
				}
			}

			return true;
			});
	}
};

static middle::SystemRegistrar<UiRenderSetup> reg("UiRenderSetup");
