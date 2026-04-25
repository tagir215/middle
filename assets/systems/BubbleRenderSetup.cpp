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
#include "bubble_colors.h"
#include "ExponentComponent.h"
#include "Layer.h"
#include "Rectangle.h"
#include "UiComponent.h"
#include "TextureComponent.h"
#include "RuntimeHiddenTag.h"
#include "ActiveCheckBoxTag.h"
#include "BubbleAlgebraProblem.h"


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
	components::CompCache* exponentCache;
	components::CompCache* textureCache;
	components::CompCache* activeCheckBoxCache;

	void init(middle::GameState* gameState) {
		bubbleCache = middle::newCompCache(gameState);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::Circle>();
		bubbleCache->addType<components::Layer>();
		bubbleCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleVariable>(components::NOTINTERESTED);
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
		unitCache->addType<components::Layer>();
		unitCache->addType<components::MouseIntersectable>();
		unitCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		variableCache = middle::newCompCache(gameState);
		variableCache->addType<components::BubbleComponent>();
		variableCache->addType<components::Layer>();
		variableCache->addType<components::BubbleVariable>();
		variableCache->addType<components::Circle>();
		variableCache->addType<components::Circle>();
		variableCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		cuboidCache = middle::newCompCache(gameState);
		cuboidCache->addType<components::Cuboid>();
		cuboidCache->addType<components::Position>();
		cuboidCache->addType<components::TextureComponent>(components::NOTINTERESTED);
		cuboidCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		equalsCache = middle::newCompCache(gameState);
		equalsCache->addType<components::BubbleEqualsComponent>();
		equalsCache->addType<components::LoopSociety>();
		exponentCache = middle::newCompCache(gameState);
		exponentCache->addType<components::ExponentComponent>();
		exponentCache->addType<components::Circle>();
		exponentCache->addType<components::Position>();
		exponentCache->addType<components::Layer>();
		exponentCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		textureCache = middle::newCompCache(gameState);
		textureCache->addType<components::TextureComponent>();
		textureCache->addType<components::Position>();
		textureCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		activeCheckBoxCache = middle::newCompCache(gameState);
		activeCheckBoxCache->addType<components::ActiveCheckBoxTag>();
		activeCheckBoxCache->addType<components::Position>();
		activeCheckBoxCache->addType<components::Layer>();

	}
	bool debugRendering = false;


	void calculateExponentVisualFactors(float radius, int power, float positionRatioToPower, float& intersectOffsetX, float& intersectOffsetZ, float& bz, float& rb) {
		const float powerRatioToBubble = 0.333f;
		const float arrowGap = 6;

		float ra = radius;
		rb = radius * powerRatioToBubble + arrowGap * power;
		bz = ra + rb * positionRatioToPower;
		intersectOffsetZ = (ra * ra - rb * rb + bz * bz) / (bz + bz);
		float smallZ = intersectOffsetZ - bz;
		intersectOffsetX = std::sqrt(rb * rb - smallZ * smallZ);
	}


	void update(middle::GameState* gameState) override {

		gameState->editorState.backgroundColor = bubbleColors::BACKGROUND;

		// render bubbbles
		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		auto bubbleCircleIt = bubbleCache->begin<components::Circle>();
		auto bubbleLayerIt = bubbleCache->begin<components::Layer>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto bubble = *bubbleIt;
			auto circle = *bubbleCircleIt;
			auto layer = *bubbleLayerIt;
			auto& shape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);
			bool isUiItem = middle::getComponent<components::UiComponent>(shape);

			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			bool intersecting = intersectable && intersectable->intersectingTop;
			Color color = intersecting ? bubbleColors::HOVERED_ITEM : bubbleColors::BUBBLE_OUTLINE;
			float radius = intersecting ? circle->radius * 1.05f : circle->radius;
			Color backgroundColor = bubble->inverse ? bubbleColors::BUBBLE_BACKGROUND_INVERSE : bubbleColors::BUBBLE_BACKGROUND;
			middle::RenderItem circleItem;
			circleItem.type = middle::RenderItemType::CIRCLE;
			circleItem.color = color;
			circleItem.layer = layer->layer;
			circleItem.backgroundColor = backgroundColor;
			circleItem.radius = radius;
			Vector3 pos = middle::getShapePosition(gameState, shape.id.index);
			circleItem.center = pos;
			circleItem.disableDepthTest = isUiItem;
			gameState->renderData.push_back(circleItem);

			if (bubble->inverse) {
				float offsetX, offsetZ, bz, rb;
				calculateExponentVisualFactors(circle->radius, 1, -0.33f, offsetX, offsetZ, bz, rb);
				middle::RenderItem inverseIndicator;
				inverseIndicator.type = middle::RenderItemType::CIRCLE;
				inverseIndicator.color = bubbleColors::POSITIVE_UNIT;
				inverseIndicator.backgroundColor = bubbleColors::POSITIVE_UNIT;
				inverseIndicator.layer = layer->layer + 2;
				inverseIndicator.radius = bubble::unitRadius;
				inverseIndicator.center = Vector3{ pos.x + offsetX, pos.y, pos.z + offsetZ };
				inverseIndicator.disableDepthTest = isUiItem;
				gameState->renderData.push_back(inverseIndicator);
			}
		}


		// render units
		auto unitIt = unitCache->begin<components::BubbleUnit>();
		auto unitCircleIt = unitCache->begin<components::Circle>();
		auto unitPosIt = unitCache->begin<components::Position>();
		auto unitLayerIt = unitCache->begin<components::Layer>();
		auto unitIntersectableIt = unitCache->begin<components::MouseIntersectable>();
		for (int i = 0; i < unitCache->getSize(); ++i) {
			auto unit = *unitIt;
			auto circle = *unitCircleIt;
			auto pos = *unitPosIt;
			auto layer = *unitLayerIt;
			auto intersectable = *unitIntersectableIt;
			auto& shape = middle::getShape(gameState, unitCache->relevantIdVector[i].index);
			bool isUiItem = middle::getComponent<components::UiComponent>(shape);

			float radius = circle->radius;
			if (intersectable->intersectingTop) {
				radius *= 1.5f;
			}

			middle::RenderItem particle;
			particle.center = { pos->posX, pos->posY, pos->posZ };
			particle.length = 0.1f;
			particle.ringRadius = radius;
			particle.radius = radius;
			particle.layer = layer->layer;
			particle.disableDepthTest = isUiItem;
			particle.type = middle::RenderItemType::CIRCLE;
			if (unit->value == 1) {
				particle.color = bubbleColors::POSITIVE_UNIT;
				particle.backgroundColor = bubbleColors::POSITIVE_UNIT;
			}
			if (unit->value == 0) {
				particle.color = bubbleColors::BUBBLE_OUTLINE;
			}
			if (unit->value == -1) {
				particle.color = bubbleColors::NEGATIVE_UNIT;
				particle.backgroundColor = bubbleColors::NEGATIVE_UNIT;
			}

			gameState->renderData.push_back(particle);
		}

		// render variables
		auto variableIt = variableCache->begin<components::BubbleVariable>();
		auto variableBubbleIt = variableCache->begin<components::BubbleComponent>();
		auto variableCircleIt = variableCache->begin<components::Circle>();
		auto layerIt = variableCache->begin<components::Layer>();
		for (int i = 0; i < variableCache->getSize(); ++i) {
			auto variable = *variableIt;
			auto bubble = *variableBubbleIt;
			auto layer = *layerIt;
			auto circle = *variableCircleIt;
			auto& shape = middle::getShape(gameState, variableCache->relevantIdVector[i].index);
			bool isUiItem = middle::getComponent<components::UiComponent>(shape);

			// todo temp
			if (circle->radius < bubble::variableRadius) {
				circle->radius = bubble::variableRadius;
			}


			auto pos = middle::getComponent<components::Position>(shape);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);

			float radius = circle->radius;
			float fontSize = bubble::variableTextFontSize;
			if (intersectable && intersectable->intersectingTop) {
				fontSize *= 1.2f;
				radius *= 1.2f;
			}
			Color color;
			color = bubbleColors::POSITIVE_UNIT;
			if (variable->isNegative) {
				color = bubbleColors::NEGATIVE_UNIT;
			}
			Color backgroundColor = bubbleColors::BUBBLE_BACKGROUND;
			if ((bubble->inverse)) {
				backgroundColor = bubbleColors::BUBBLE_BACKGROUND_INVERSE;
			}

			middle::RenderItem variableText;
			variableText.type = middle::RenderItemType::TEXT;
			variableText.center = { pos->posX, pos->posY, pos->posZ };
			variableText.color = color;
			variableText.text = variable->label;
			variableText.fontSize = fontSize;
			variableText.disableDepthTest = isUiItem;
			gameState->renderData.push_back(variableText);

			middle::RenderItem variableCircle;
			variableCircle.type = middle::RenderItemType::CIRCLE;
			variableCircle.center = variableText.center;
			variableCircle.radius = radius;
			variableCircle.backgroundColor = backgroundColor;
			variableCircle.color = bubbleColors::VARIABLE_OUTLINE;
			variableCircle.disableDepthTest = isUiItem;
			variableCircle.layer = layer->layer + 1;
			gameState->renderData.push_back(variableCircle);

			if (bubble->inverse) {
				float offsetX, offsetZ, bz, rb;
				calculateExponentVisualFactors(circle->radius, 1, -0.33f, offsetX, offsetZ, bz, rb);
				middle::RenderItem inverseIndicator;
				inverseIndicator.type = middle::RenderItemType::CIRCLE;
				inverseIndicator.color = bubbleColors::POSITIVE_UNIT;
				inverseIndicator.backgroundColor = bubbleColors::POSITIVE_UNIT;
				inverseIndicator.layer = layer->layer + 2;
				inverseIndicator.radius = bubble::unitRadius;
				Vector3 pos = variableText.center;
				inverseIndicator.center = Vector3{ pos.x + offsetX, pos.y, pos.z + offsetZ };
				inverseIndicator.disableDepthTest = isUiItem;
				gameState->renderData.push_back(inverseIndicator);
			}
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
				auto layer = middle::getComponent<components::Layer>(shapeA);
				bool isUiItem = middle::getComponent<components::UiComponent>(shapeA);

				if (!circleA || !circleB)
					continue;
				Vector3 posA = { positionA->posX, positionA->posY, positionA->posZ };
				Vector3 posB = { positionB->posX, positionB->posY, positionB->posZ };
				Vector3 axis = Vector3Normalize(Vector3Subtract(posB, posA));
				middle::RenderItem line;
				line.type = middle::RenderItemType::LINE;
				line.linePointA = posA + Vector3Scale(axis, circleA->radius);
				line.linePointB = posB + Vector3Scale(axis, -circleB->radius);
				line.color = bubbleColors::MULTIPLICATION_CONNECTION;
				line.layer = layer->layer;
				line.disableDepthTest = isUiItem;

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
			cuboidItem.color = bubbleColors::BACKGROUND;
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
			auto layer = middle::getComponent<components::Layer>(shapeA);
			middle::RenderItem equalsLine;
			equalsLine.type = middle::RenderItemType::LINE;
			equalsLine.linePointA = linePointA;
			equalsLine.linePointB = linePointB;
			equalsLine.color = bubbleColors::EQUALS_CONNECTION;
			equalsLine.layer = layer->layer;
			gameState->renderData.push_back(equalsLine);
		}


		auto exponentIt = exponentCache->begin<components::ExponentComponent>();
		auto exponentCircleIt = exponentCache->begin<components::Circle>();
		auto exponentPositionIt = exponentCache->begin<components::Position>();
		auto exponentLayerIt = exponentCache->begin<components::Layer>();
		for (int i = 0; i < exponentCache->getSize(); ++i) {
			auto root = *exponentIt;
			auto bubbleCircle = *exponentCircleIt;
			auto position = *exponentPositionIt;
			auto layer = *exponentLayerIt;
			Vector3 bubblePos = { position->posX, position->posY, position->posZ };

			auto& shape = middle::getShape(gameState, exponentCache->relevantIdVector[i].index);
			bool isUiItem = middle::getComponent<components::UiComponent>(shape);

			float positionRatioToPower = 0.333f;

			bool isInverse = root->isInverse;
			if (isInverse) {
				positionRatioToPower *= -1;
			}

			int powerIterations = root->power;
			bool isPowerNegative = root->power < 0;
			if (isPowerNegative) {
				powerIterations = -root->power;
			}

			Color exponentColor = isPowerNegative ? bubbleColors::NEGATIVE_POWER : bubbleColors::POSITIVE_POWER;

			for (int power = 0; power < powerIterations; ++power) {

				float intersectOffsetX, intersectOffsetZ, bz, rb;
				calculateExponentVisualFactors(bubbleCircle->radius, power, positionRatioToPower, intersectOffsetX, intersectOffsetZ, bz, rb);

				Vector3 powerPos = bubblePos + Vector3{ 0,0,bz };
				Vector3 intersectOffset = Vector3{ intersectOffsetX,0, intersectOffsetZ };
				Vector3 intersectPos = bubblePos + intersectOffset;

				Vector3 refAngle = { 1,0,0 };

				Vector3 toStartPoint = Vector3Subtract(intersectPos, powerPos);
				Vector3 endPoint = intersectPos + Vector3{ -intersectOffsetX * 2, 0, 0 };
				Vector3 toEndPoint = Vector3Subtract(endPoint, powerPos);

				if (power == 0) {
					Color unitColor = root->isNegative ? bubbleColors::NEGATIVE_UNIT : bubbleColors::POSITIVE_UNIT;
					middle::RenderItem unitIndicator;
					unitIndicator.type = middle::RenderItemType::CIRCLE;
					Vector3 pos = root->isInverse || isPowerNegative ? intersectPos : endPoint;
					unitIndicator.center = pos;
					unitIndicator.backgroundColor = unitColor;
					unitIndicator.color = unitColor;
					unitIndicator.radius = bubble::unitRadius * 1.5f;
					unitIndicator.layer = layer->layer + 2;
					gameState->renderData.push_back(unitIndicator);
				}

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
				powerCircle.color = exponentColor;
				powerCircle.ringRadius = 0.2f;
				powerCircle.segments = 20;
				powerCircle.layer = layer->layer + 1;
				powerCircle.disableDepthTest = isUiItem;
				gameState->renderData.push_back(powerCircle);

				const float helperAngleOffset = PI * 0.1f;

				Vector3 conePos;
				Vector3 toNextSegment;
				Vector3 coneDir;

				if (!isPowerNegative) {
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
				cone.color = exponentColor;
				cone.layer = layer->layer + 1;
				cone.disableDepthTest = isUiItem;
				gameState->renderData.push_back(cone);
			}
		}


		auto textureIt = textureCache->begin<components::TextureComponent>();
		auto texturePosIt = textureCache->begin<components::Position>();
		for (int i = 0; i < textureCache->getSize(); ++i) {
			auto texture = *textureIt;
			auto pos = *texturePosIt;
			auto& shape = middle::getShape(gameState, textureCache->relevantIdVector[i].index);
			bool isUiItem = middle::getComponent<components::UiComponent>(shape);

			if (texture->textureType == middleTextureType::BILLBOARD) {
				middle::RenderItem textureItem;
				textureItem.type = middle::RenderItemType::BILLBOARD;
				textureItem.texture = &texture->texture;
				textureItem.transform.translation = { pos->posX, pos->posY, pos->posZ };
				textureItem.color = WHITE;
				textureItem.textureScale = texture->scale;
				textureItem.disableDepthTest = isUiItem;
				gameState->renderData.push_back(textureItem);
			}

			else if (texture->textureType == middleTextureType::BACKGROUND) {
				middle::RenderItem textureItem;
				textureItem.type = middle::RenderItemType::BACKGROUND;
				textureItem.texture = &texture->texture;
				textureItem.transform.translation = { pos->posX, pos->posY, pos->posZ };
				textureItem.color = WHITE;
				textureItem.textureScale = texture->scale;
				textureItem.disableDepthTest = false;
				textureItem.width = 10000;
				textureItem.height = 10000;
				gameState->renderData.push_back(textureItem);
			}
		}

		auto checkBoxPositionIt = activeCheckBoxCache->begin<components::Position>();
		auto checkBoxLayerIt = activeCheckBoxCache->begin<components::Layer>();
		for (int i = 0; i < activeCheckBoxCache->getSize(); ++i) {
			auto pos = *checkBoxPositionIt;
			auto layer = *checkBoxLayerIt;

			middle::RenderItem powerCircle;
			powerCircle.type = middle::RenderItemType::CYLINDER;
			powerCircle.center = { pos->posX, pos->posY, pos->posZ };
			powerCircle.radius = 3;
			powerCircle.ringRadius = 3;
			powerCircle.length = 0.001f;
			powerCircle.color = RED;
			powerCircle.layer = layer->layer + 1;
			powerCircle.disableDepthTest = true;
			gameState->renderData.push_back(powerCircle);
		}
	}


};

static middle::SystemRegistrar<BubbleRenderSetup> reg("BubbleRenderSetup");
