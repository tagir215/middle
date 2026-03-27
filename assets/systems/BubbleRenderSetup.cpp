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
#include "Circle.h"
#include "Cuboid.h"
#include "BubbleEqualsComponent.h"
#include "BubbleVariable.h"
#include "BubbleRootComponent.h" 


class BubbleRenderSetup : public middle::MiddleGameplaySystem {
public:

	BubbleRenderSetup() {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::RENDERING;
	}

	components::CompCache* bubbleCache;
	components::CompCache* mulCache;
	components::CompCache* fractionCache;
	components::CompCache* unitCache;
	components::CompCache* variableCache;
	components::CompCache* cuboidCache;
	components::CompCache* equalsCache;
	components::CompCache* rootCache;

	void init(middle::GameState* gameState) {
		bubbleCache = middle::newCompCache(gameState);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::Circle>();
		mulCache = middle::newCompCache(gameState);
		mulCache->addType<components::BubbleMultiplyComponent>();
		mulCache->addType<components::LoopSociety>();
		fractionCache = middle::newCompCache(gameState);
		fractionCache->addType<components::FractionalComponent>();
		fractionCache->addType<components::LoopSociety>();
		unitCache = middle::newCompCache(gameState);
		unitCache->addType<components::BubbleUnit>();
		unitCache->addType<components::Circle>();
		unitCache->addType<components::Position>();
		unitCache->addType<components::BubbleVariable>(components::NOTINTERESTED);
		variableCache = middle::newCompCache(gameState);
		variableCache->addType<components::BubbleUnit>();
		variableCache->addType<components::BubbleVariable>();
		cuboidCache = middle::newCompCache(gameState);
		cuboidCache->addType<components::Cuboid>();
		cuboidCache->addType<components::Position>();
		equalsCache = middle::newCompCache(gameState);
		equalsCache->addType<components::BubbleEqualsComponent>();
		equalsCache->addType<components::LoopSociety>();
		rootCache = middle::newCompCache(gameState);
		rootCache->addType<components::BubbleRootComponent>();
		rootCache->addType<components::Circle>();
		rootCache->addType<components::Position>();
	}
	bool debugRendering = false;

	void update(middle::GameState* gameState) override {


		// render bubbbles
		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		auto bubbleCircleIt = bubbleCache->begin<components::Circle>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto bubble = *bubbleIt;
			auto circle = *bubbleCircleIt;
			auto& shape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);

			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			bool intersecting = intersectable && intersectable->intersectingTop;
			Color color = intersecting ? WHITE : Color{ 200,200,200,200 };
			middle::RenderItem circleItem;
			circleItem.type = middle::RenderItemType::CIRCLE;
			circleItem.color = color;
			circleItem.radius = circle->radius;
			circleItem.center = middle::getShapePosition(gameState, shape.id.index);
			gameState->renderData.push_back(circleItem);

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
		auto unitCircleIt = unitCache->begin<components::Circle>();
		auto unitPosIt = unitCache->begin<components::Position>();
		for (int i = 0; i < unitCache->getSize(); ++i) {
			auto unit = *unitIt;
			auto circle = *unitCircleIt;
			auto pos = *unitPosIt;
			auto& shape = middle::getShape(gameState, unitCache->relevantIdVector[i].index);

			middle::RenderItem particle;
			particle.center = { pos->posX, pos->posY, pos->posZ };
			particle.length = 0.1f;
			particle.ringRadius = circle->radius;
			particle.radius = circle->radius;
			if (unit->value == 1) {
				particle.type = middle::RenderItemType::CYLINDER;
				particle.color = BLACK;
			}
			if (unit->value == 0) {
				particle.type = middle::RenderItemType::CIRCLE;
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

		// render variables
		auto variableIt = variableCache->begin<components::BubbleVariable>();
		auto variableUnitIt = variableCache->begin<components::BubbleUnit>();
		for (int i = 0; i < variableCache->getSize(); ++i) {
			auto variable = *variableIt;
			auto unit = *variableUnitIt;
			auto& shape = middle::getShape(gameState, variableCache->relevantIdVector[i].index);

			auto pos = middle::getComponent<components::Position>(shape);
			middle::RenderItem variableRing;
			variableRing.center = { pos->posX, pos->posY, pos->posZ };
			variableRing.length = 0.1f;
			const float variableRadius = 4;
			variableRing.ringRadius = variableRadius;
			variableRing.radius = variableRadius;
			variableRing.type = middle::RenderItemType::CIRCLE;
			variableRing.color = BLACK;
			if (unit->value == -1) {
				variableRing.color = { 0,255,255,255 };
			}
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			if (intersectable->intersectingTop) {
				variableRing.color = WHITE;
			}
			gameState->renderData.push_back(variableRing);

			middle::RenderItem variableText;
			variableText.type = middle::RenderItemType::TEXT;
			variableText.center = variableRing.center;
			variableText.color = GREEN;
			variableText.text = variable->label;
			variableText.fontSize = 15;
			gameState->renderData.push_back(variableText);
		}


		// render muls
		auto mulIt = mulCache->begin<components::BubbleMultiplyComponent>();
		auto mulLoopIt = mulCache->begin<components::LoopSociety>();
		for (int i = 0; i < mulCache->getSize(); ++i) {
			auto multiplyComponent = *mulIt;
			auto loop = *mulLoopIt;
			for (int x = 1; x < loop->loopMemberIds.size(); ++x) {
				auto& shapeA = middle::getShape(gameState, loop->loopMemberIds[x - 1].index);
				auto& shapeB = middle::getShape(gameState, loop->loopMemberIds[x].index);
				auto positionA = middle::getComponent<components::Position>(shapeA);
				auto positionB = middle::getComponent<components::Position>(shapeB);
				auto circleA = middle::getComponent<components::Circle>(shapeA);
				auto circleB = middle::getComponent<components::Circle>(shapeB);
				if (!circleA || !circleB)
					continue;
				Vector3 posA = { positionA->posX, positionA->posY, positionA->posZ };
				Vector3 posB = { positionB->posX, positionB->posY, positionB->posZ };
				Vector3 axis = Vector3Normalize(Vector3Subtract(posB, posA));
				middle::RenderItem line;
				line.type = middle::RenderItemType::LINE;
				line.linePointA = posA + Vector3Scale(axis, circleA->radius);
				line.linePointB = posB + Vector3Scale(axis, -circleB->radius);
				line.color = RED;
				gameState->renderData.push_back(line);
			}
		}


		auto fractionLoopIt = fractionCache->begin<components::LoopSociety>();
		for (int i = 0; i < fractionCache->getSize(); ++i) {
			auto loop = *fractionLoopIt;
			for (int x = 1; x < loop->loopMemberIds.size(); ++x) {
				auto& shapeA = middle::getShape(gameState, loop->loopMemberIds[x - 1].index);
				auto& shapeB = middle::getShape(gameState, loop->loopMemberIds[x].index);
				auto positionA = middle::getComponent<components::Position>(shapeA);
				auto positionB = middle::getComponent<components::Position>(shapeB);
				auto circleA = middle::getComponent<components::Circle>(shapeA);
				auto circleB = middle::getComponent<components::Circle>(shapeB);
				Vector3 posA = { positionA->posX, positionA->posY, positionA->posZ };
				Vector3 posB = { positionB->posX, positionB->posY, positionB->posZ };
				middle::RenderItem line;
				line.type = middle::RenderItemType::LINE;
				line.color = WHITE;
				Vector3 pointA = posA;
				Vector3 pointB = posB;
				Vector3 axis = Vector3Normalize(Vector3Subtract(posB, posA));
				if (circleA) {
					pointA += Vector3Scale(axis, circleA->radius);
				}
				if (circleB) {
					pointB += Vector3Scale(axis, -circleB->radius);
				}

				line.linePointA = pointA;
				line.linePointB = pointB;
				gameState->renderData.push_back(line);
			}
		}


		auto cuboidIt = cuboidCache->begin<components::Cuboid>();
		auto cuboidPosIt = cuboidCache->begin<components::Position>();
		for (int i = 0; i < cuboidCache->getSize(); ++i) {
			auto cuboid = *cuboidIt;
			auto pos = *cuboidPosIt;
			middle::RenderItem cuboidItem;
			cuboidItem.type = middle::RenderItemType::CUBOID;
			cuboidItem.width = cuboid->width;
			cuboidItem.height = cuboid->height;
			cuboidItem.length = cuboid->length;
			cuboidItem.color = WHITE;
			// TODO
			cuboidItem.color.a = 30;
			cuboidItem.center = { pos->posX, pos->posY, pos->posZ };
			gameState->renderData.push_back(cuboidItem);
		}

		auto equalsIt = equalsCache->begin<components::LoopSociety>();
		for (int i = 0; i < equalsCache->getSize(); ++i) {
			auto equalsLoop = *equalsIt;
			assert(equalsLoop->loopMemberIds.size() == 2);
			middle::Id& idA = equalsLoop->loopMemberIds[0];
			middle::Id& idB = equalsLoop->loopMemberIds[1];
			middle::Shape& shapeA = middle::getShape(gameState, idA.index);
			middle::Shape& shapeB = middle::getShape(gameState, idB.index);
			Vector3 posA = middle::getShapePosition(gameState, idA.index);
			Vector3 posB = middle::getShapePosition(gameState, idB.index);
			auto circleA = middle::getComponent<components::Circle>(shapeA);
			auto circleB = middle::getComponent<components::Circle>(shapeB);
			Vector3 axis = Vector3Normalize(Vector3Subtract(posB, posA));
			Vector3 linePointA = posA + Vector3Scale(axis, circleA->radius);
			Vector3 linePointB = posB + Vector3Scale(axis, -circleB->radius);
			middle::RenderItem equalsLine;
			equalsLine.type = middle::RenderItemType::LINE;
			equalsLine.linePointA = linePointA;
			equalsLine.linePointB = linePointB;
			equalsLine.color = BLUE;
			gameState->renderData.push_back(equalsLine);
		}


		auto rootIt = rootCache->begin<components::BubbleRootComponent>();
		auto rootCircleIt = rootCache->begin<components::Circle>();
		auto rootPositionIt = rootCache->begin<components::Position>();
		for (int i = 0; i < rootCache->getSize(); ++i) {
			auto root = *rootIt;
			auto bubbleCircle = *rootCircleIt;
			auto position = *rootPositionIt;
			Vector3 bubblePos = { position->posX, position->posY, position->posZ };


			const float powerRatioToBubble = 0.333f;
			const float arrowGap = 6;
			float positionRatioToPower = 0.333f;

			bool isInverse = root->isInverse;
			if (isInverse) {
				positionRatioToPower *= -1;
			}

			int powerIterations = root->power;
			bool isNegative = root->power < 0;
			if (isNegative) {
				powerIterations = -root->power;
			}

			Color powerColor = isNegative ? BLUE : RED;

			for (int power = 0; power < powerIterations; ++power) {
				float ra = bubbleCircle->radius;
				float rb = bubbleCircle->radius * powerRatioToBubble + arrowGap * power;
				float bz = ra + rb * positionRatioToPower;
				float intersectOffsetZ = (ra * ra - rb * rb + bz * bz) / (bz + bz);
				float smallZ = intersectOffsetZ - bz;
				float intersectOffsetX = std::sqrt(rb * rb - smallZ * smallZ);

				Vector3 powerPos = bubblePos + Vector3{ 0,0,bz };
				Vector3 intersectOffset = Vector3{ intersectOffsetX,0, intersectOffsetZ };
				Vector3 intersectPos = bubblePos + intersectOffset;


				Vector3 refAngle = { 1,0,0 };

				Vector3 toStartPoint = Vector3Subtract(intersectPos, powerPos);
				Vector3 endPoint = intersectPos + Vector3{-intersectOffsetX * 2, 0, 0};
				Vector3 toEndPoint = Vector3Subtract(endPoint, powerPos);

				if (isInverse) {
					toStartPoint.x *= -1;
					toEndPoint.x *= -1;
				}

				float startingAngle = Vector3Angle(refAngle, toStartPoint);
				if (toStartPoint.z < refAngle.z) {
					startingAngle *= -1;
				}

				float endAngle = startingAngle + PI;

				Vector3 endPointVecNonComplete = Vector3RotateByAxisAngle(refAngle, { 0,-1,0 }, endAngle);
				float remainingAngle = Vector3Angle(endPointVecNonComplete, toEndPoint);
				endAngle += remainingAngle;


				middle::RenderItem powerCircle;
				powerCircle.type = middle::RenderItemType::CIRCLE_SECTOR;
				powerCircle.center = powerPos;
				powerCircle.radius = rb;
				powerCircle.startAngle = startingAngle;
				powerCircle.endAngle = endAngle;
				powerCircle.color = powerColor;
				powerCircle.ringRadius = 0.2f;
				powerCircle.segments = 20;
				gameState->renderData.push_back(powerCircle);

				const float helperAngleOffset = PI * 0.1f;

				Vector3 conePos;
				Vector3 toNextSegment;
				Vector3 coneDir;

				if (!isNegative) {
					conePos = powerPos + toStartPoint;
					toNextSegment = Vector3RotateByAxisAngle(toStartPoint, { 0,-1,0 }, helperAngleOffset);
					coneDir = Vector3Normalize(Vector3Subtract(conePos, powerPos + toNextSegment));
				}
				else {
					conePos = powerPos + toEndPoint;
					toNextSegment = Vector3RotateByAxisAngle(toEndPoint, { 0,-1,0 }, -helperAngleOffset);
					coneDir = Vector3Normalize(Vector3Subtract(conePos, powerPos + toNextSegment));
				}

				middle::RenderItem cone;
				cone.type = middle::RenderItemType::CYLINDER;
				const float arrowLengthProportionToRadius = 0.41f;
				const float arrowWidthProportionToLength = 0.75f;
				float arrowLength = arrowLengthProportionToRadius * rb;
				float arrowRadius = arrowLength * arrowWidthProportionToLength;
				const Vector3 coneScale = { 1,1,0.001f };
				cone.radius = arrowRadius;
				cone.ringRadius = 0;
				cone.length = arrowLength;
				cone.transform.rotation = QuaternionFromVector3ToVector3({ 0,-1,0 }, coneDir);
				cone.transform.translation = conePos;
				cone.transform.scale = coneScale;
				cone.center = { 0,0,0 };
				cone.color = powerColor;
				gameState->renderData.push_back(cone);

			}
		}
	}
};

static middle::SystemRegistrar<BubbleRenderSetup> reg("BubbleRenderSetup");
