#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "middle_component_table.h"
#include "BubbleComponent.h"
#include "BubbleMultiplyComponent.h"
#include "Position.h"
#include "Sphere.h"
#include "BubbleUnit.h"
#include "FractionalComponent.h"
#include "LoopSociety.h"
#include "MouseIntersectable.h"

class BubbleRenderSetup : public middle::MiddleGameplaySystem {

	bool debugRendering = false;

	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {

			auto bubbleComponent = middle::getComponent<components::BubbleComponent>(shape);
			auto multiplyComponent = middle::getComponent<components::BubbleMultiplyComponent>(shape);
			auto unit = middle::getComponent<components::BubbleUnit>(shape);
			auto fraction = middle::getComponent<components::FractionalComponent>(shape);
			if (!bubbleComponent && !multiplyComponent && !unit && !fraction)
				return true;

			if (bubbleComponent && bubbleComponent->hidden) {
				return true;
			}
			if (unit && unit->hidden) {
				return true;
			}

			if (gameState->applicationMode == middle::ApplicationMode::GAME_MODE) {

				if (unit) {
					auto pos = middle::getComponent<components::Position>(shape);
					middle::RenderItem particle;
					particle.center = { pos->posX, pos->posY, pos->posZ };
					particle.type = middle::RenderItemType::SPHERE;
					const float unitRadius = 2;
					particle.radius = unitRadius;
					if (unit->value == 1) {
						particle.color = middle::UGLY_PINK;
					}
					if (unit->value == 0) {
						particle.color = { 255,255,255, 60 };
					}
					if (unit->value == -1) {
						particle.color = { 0,255,255,255 };
					}

					auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
					if (intersectable->intersectingTop) {
						particle.color = WHITE;
					}
					gameState->renderData.push_back(particle);
				}

				if (multiplyComponent) {
					auto pos = middle::getComponent<components::Position>(shape);
					middle::RenderItem multiplyItem;
					multiplyItem.center = { pos->posX, pos->posY, pos->posZ };
					multiplyItem.text = "X";
					multiplyItem.fontSize = 20;
					multiplyItem.type = middle::RenderItemType::TEXT;
					gameState->renderData.push_back(multiplyItem);
				}

				if (bubbleComponent) {
					for (int index = 0; index < bubbleComponent->outlineNodes.size(); ++index) {
						int indexA = index - 1;
						int indexB = index;
						if (index == 0) {
							indexA = bubbleComponent->outlineNodes.size() - 1;
						}
						middle::Id nodeIdA = bubbleComponent->outlineNodes[indexA];
						middle::Id nodeIdB = bubbleComponent->outlineNodes[indexB];
						Vector3 posA = middle::getShapePosition(gameState, nodeIdA.index);
						Vector3 posB = middle::getShapePosition(gameState, nodeIdB.index);

						middle::RenderItem outlineItem;
						outlineItem.type = middle::RenderItemType::LINE;
						outlineItem.linePointA = posA;
						outlineItem.linePointB = posB;

						auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
						if (intersectable->intersectingTop) {
							outlineItem.color = WHITE;
						}
						else {
							outlineItem.color = middle::UGLY_PINK;
						}
						gameState->renderData.push_back(outlineItem);
					}
				}

				if (fraction) {
					auto loop = middle::getComponent<components::LoopSociety>(shape);
					int size = loop->loopMemberIds.size();
					for (int x = 0; x < size; ++x) {
						for (int y = x + 1; y < size; ++y) {
							middle::RenderItem fractionLine;
							fractionLine.type = middle::RenderItemType::LINE;
							fractionLine.linePointA = middle::getShapePosition(gameState, loop->loopMemberIds[x].index);
							fractionLine.linePointB = middle::getShapePosition(gameState, loop->loopMemberIds[y].index);
							fractionLine.color = { 255,255,255,50 };
							gameState->renderData.push_back(fractionLine);
						}
					}
				}
			}


			if (bubbleComponent && debugRendering) {

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
			return true;
			});
	}
};

static middle::SystemRegistrar<BubbleRenderSetup> reg("BubbleRenderSetup");
