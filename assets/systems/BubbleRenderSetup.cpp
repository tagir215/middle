#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "middle_component_table.h"
#include "BubbleComponent.h"

class BubbleRenderSetup : public middle::MiddleGameplaySystem {

	bool debugRendering = false;

	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {

			auto bubbleComponent = middle::getComponent<components::BubbleComponent>(shape);
			if (!bubbleComponent)
				return;



			if (debugRendering) {

				Vector3 axis = { bubbleComponent->axisX, bubbleComponent->axisY, bubbleComponent->axisZ };
				Vector3 center = { bubbleComponent->centerX, bubbleComponent->centerY, bubbleComponent->centerZ };
				float l = bubbleComponent->length;
				float w = bubbleComponent->width;



				middle::RenderItem p1;
				p1.linePointA = center + axis * l * 0.5f;
				p1.linePointB = center - axis * l * 0.5f;
				p1.type = middle::RenderItemType::LINE;
				p1.color = PINK;
				gameState->renderData.push_back(p1);

				middle::RenderItem c;
				c.center = center;
				c.radius = 10;
				c.type = middle::RenderItemType::SPHERE;
				c.color = PINK;
				gameState->renderData.push_back(c);

				middle::RenderItem d;
				d.center = { bubbleComponent->aX, bubbleComponent->aY, bubbleComponent->aZ };
				d.radius = 3;
				d.type = middle::RenderItemType::SPHERE;
				d.color = ORANGE;
				gameState->renderData.push_back(d);

				middle::RenderItem e;
				e.center = { bubbleComponent->bX, bubbleComponent->bY, bubbleComponent->bZ };
				e.radius = 3;
				e.type = middle::RenderItemType::SPHERE;
				e.color = ORANGE;
				gameState->renderData.push_back(e);

				middle::RenderItem nodeCountText;
				nodeCountText.center = center;
				nodeCountText.type = middle::RenderItemType::TEXT;
				//nodeCountText.text = std::to_string(bubbleComponent->outline.size());
				nodeCountText.text = std::to_string(bubbleComponent->nodeCountTarget);
				gameState->renderData.push_back(nodeCountText);

			}
			});
	}
};

static middle::SystemRegistrar<BubbleRenderSetup> reg("BubbleRenderSetup");
