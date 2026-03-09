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
#include "bubble_utils.h"
#include "BubbleRef.h"

class BubbleRenderSetup : public middle::MiddleGameplaySystem {

public:
	components::CompCache* bubbleCache;
	components::CompCache* mulCache;
	components::CompCache* fractionCache;
	components::CompCache* unitCache;

	void init(middle::GameState* gameState) {
		bubbleCache = middle::newCompCache(gameState);
		bubbleCache->addType<components::BubbleComponent>();
		mulCache = middle::newCompCache(gameState);
		mulCache->addType<components::BubbleMultiplyComponent>();
		fractionCache = middle::newCompCache(gameState);
		fractionCache->addType<components::FractionalComponent>();
		unitCache = middle::newCompCache(gameState);
		unitCache->addType<components::BubbleUnit>();
	}
	bool debugRendering = false;

	void update(middle::GameState* gameState) override {

		if (gameState->applicationMode == middle::ApplicationMode::GAME_MODE) {

			// render bubbbles
			auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
			for (int i = 0; i < bubbleCache->getSize(); ++i) {
				auto bubble = *bubbleIt;
				auto& shape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);

				auto ref = middle::getComponent<components::BubbleRef>(shape);
				if (!ref || ref->idRef.index == middle::UNASSIGNED) {
					continue;
				}

				if (!middle::isShapeAlive(gameState, ref->idRef.index)) {
					continue;
				}

				auto& bubbleContainer = middle::getShape(gameState, ref->idRef.index);
				auto containerLoop = middle::getComponent<components::LoopSociety>(bubbleContainer);

				std::vector<middle::Id>outlineNodes = bubble::getNodes(gameState, containerLoop);

				for (int index = 0; index < outlineNodes.size(); ++index) {
					int indexA = index - 1;
					int indexB = index;
					if (index == 0) {
						indexA = outlineNodes.size() - 1;
					}
					middle::Id nodeIdA = outlineNodes[indexA];
					middle::Id nodeIdB = outlineNodes[indexB];
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

				if (debugRendering) {

					Vector3 axis = { bubble->axisX, bubble->axisY, bubble->axisZ };
					Vector3 center = { bubble->centerX, bubble->centerY, bubble->centerZ };
					float l = bubble->length;
					float w = bubble->width;

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
					d.center = { bubble->aX, bubble->aY, bubble->aZ };
					d.radius = 3;
					d.type = middle::RenderItemType::SPHERE;
					d.color = ORANGE;
					gameState->renderData.push_back(d);

					middle::RenderItem e;
					e.center = { bubble->bX, bubble->bY, bubble->bZ };
					e.radius = 3;
					e.type = middle::RenderItemType::SPHERE;
					e.color = ORANGE;
					gameState->renderData.push_back(e);

					middle::RenderItem nodeCountText;
					nodeCountText.center = center;
					nodeCountText.type = middle::RenderItemType::TEXT;
					//nodeCountText.text = std::to_string(bubble->outline.size());
					nodeCountText.text = std::to_string(bubble->nodeCountTarget);
					gameState->renderData.push_back(nodeCountText);

				}

			}


			// render units
			auto unitIt = unitCache->begin<components::BubbleUnit>();
			for (int i = 0; i < unitCache->getSize(); ++i) {
				auto unit = *unitIt;
				auto& shape = middle::getShape(gameState, unitCache->relevantIdVector[i].index);

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

			// render muls
			auto mulIt = mulCache->begin<components::BubbleMultiplyComponent>();
			for (int i = 0; i < mulCache->getSize(); ++i) {
				auto multiplyComponent = *mulIt;
				auto& shape = middle::getShape(gameState, mulCache->relevantIdVector[i].index);

				auto pos = middle::getComponent<components::Position>(shape);
				middle::RenderItem multiplyItem;
				multiplyItem.center = { pos->posX, pos->posY, pos->posZ };
				multiplyItem.text = "X";
				multiplyItem.fontSize = 20;
				multiplyItem.type = middle::RenderItemType::TEXT;
				gameState->renderData.push_back(multiplyItem);
			}


			// render fractions
			auto fractionIt = fractionCache->begin<components::FractionalComponent>();
			for (int i = 0; i < fractionCache->getSize(); ++i) {
				auto& shape = middle::getShape(gameState, fractionCache->relevantIdVector[i].index);

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

	}
};

static middle::SystemRegistrar<BubbleRenderSetup> reg("BubbleRenderSetup");
