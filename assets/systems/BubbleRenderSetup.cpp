#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "middle_component_table.h"
#include "BubbleComponent.h"
#include "BubbleMultiplyComponent.h"
#include "Sphere.h"
#include "BubbleUnit.h"
#include "FractionalComponent.h"
#include "LoopSociety.h"
#include "IntersectingTag.h"
#include "bubble_utils.h"
#include "BubbleRef.h"
#include "Circle.h"
#include "Cuboid.h"
#include "BubbleEqualsComponent.h"
#include "BubbleVariable.h"
#include "bubble_colors.h"
#include "Layer.h"
#include "Rectangle.h"
#include "UiComponent.h"
#include "TextureComponent.h"
#include "RuntimeHiddenTag.h"
#include "ActiveCheckBoxTag.h"
#include "InputVariable.h"
#include "ProcedureInputVariable.h"
#include "ProcedureContainer.h"
#include "CodeBlock.h"
#include "IdRef.h"
#include "UnIntersectableWindowComponent.h"
#include "ActiveSceneEditableTag.h"
#include "GlobalTransform.h"
#include "component_utils.h"
#include "LocalPosition.h"
#include "LocalScale.h"
#include "BubblePowerComponent.h"
#include "BubbleInequaltyComponent.h"


class BubbleRenderSetup : public middle::MiddleGameplaySystem {
public:

	BubbleRenderSetup() {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::RENDERING;
	}

	components::CompCache* bubbleCache;
	components::CompCache* mulCache;
	components::CompCache* fractionCache;
	components::CompCache* variableCache;
	components::CompCache* cuboidCache;
	components::CompCache* equalsCache;
	components::CompCache* inequCache;
	components::CompCache* textureCache;
	components::CompCache* editThisCache;
	components::CompCache* inputCache;
	components::CompCache* procContainerCache;
	components::CompCache* unitCache;
	components::CompCache* activeBubbleCache;
	components::CompCache* oPosCache;
	components::CompCache* powerCache;

	void init(middle::GameState* gameState) {
		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::Circle>();
		bubbleCache->addType<components::Layer>();
		bubbleCache->addType<components::LoopSociety>();
		bubbleCache->addType<components::GlobalTransform>();
		bubbleCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleVariable>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleUnit>(components::NOTINTERESTED);
		unitCache = middle::newCompCache(gameState, systemName);
		unitCache->addType<components::BubbleUnit>();
		unitCache->addType<components::Layer>();
		unitCache->addType<components::GlobalTransform>();
		unitCache->addType<components::Circle>();
		mulCache = middle::newCompCache(gameState, systemName);
		mulCache->addType<components::BubbleMultiplyComponent>();
		mulCache->addType<components::LoopSociety>();
		mulCache->addType<components::GlobalTransform>();
		mulCache->addType<components::Circle>();
		mulCache->addType<components::Layer>();
		fractionCache = middle::newCompCache(gameState, systemName);
		fractionCache->addType<components::FractionalComponent>();
		fractionCache->addType<components::LoopSociety>();
		fractionCache->addType<components::GlobalTransform>();
		variableCache = middle::newCompCache(gameState, systemName);
		variableCache->addType<components::BubbleComponent>();
		variableCache->addType<components::Layer>();
		variableCache->addType<components::BubbleVariable>();
		variableCache->addType<components::Circle>();
		variableCache->addType<components::GlobalTransform>();
		variableCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		cuboidCache = middle::newCompCache(gameState, systemName);
		cuboidCache->addType<components::Cuboid>();
		cuboidCache->addType<components::GlobalTransform>();
		cuboidCache->addType<components::TextureComponent>(components::NOTINTERESTED);
		cuboidCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		equalsCache = middle::newCompCache(gameState, systemName);
		equalsCache->addType<components::BubbleEqualsComponent>();
		equalsCache->addType<components::Layer>();
		equalsCache->addType<components::Circle>();
		equalsCache->addType<components::GlobalTransform>();
		inequCache = middle::newCompCache(gameState, systemName);
		inequCache->addType<components::BubbleInequaltyComponent>();
		inequCache->addType<components::Layer>();
		inequCache->addType<components::Circle>();
		inequCache->addType<components::GlobalTransform>();
		textureCache = middle::newCompCache(gameState, systemName);
		textureCache->addType<components::TextureComponent>();
		textureCache->addType<components::GlobalTransform>();
		textureCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		editThisCache = middle::newCompCache(gameState, systemName);
		editThisCache->addType<components::TextureComponent>();
		inputCache = middle::newCompCache(gameState, systemName);
		inputCache->addType<components::InputVariable>();
		inputCache->addType<components::IntersectingTag>();
		inputCache->addType<components::ProcedureInputVariable>(components::NOTINTERESTED);
		procContainerCache = middle::newCompCache(gameState, systemName);
		procContainerCache->addType<components::ProcedureContainer>();
		activeBubbleCache = middle::newCompCache(gameState, systemName);
		activeBubbleCache->addType<components::ActiveSceneSelectableTag>();
		activeBubbleCache->addType<components::GlobalTransform>();
		powerCache = middle::newCompCache(gameState, systemName);
		powerCache->addType<components::BubblePowerComponent>();
		powerCache->addType<components::Circle>();
		powerCache->addType<components::LoopSociety>();
		powerCache->addType<components::GlobalTransform>();
		powerCache->addType<components::Layer>();

		oPosCache = middle::newCompCache(gameState, systemName);
		oPosCache->addType<components::Position>(components::NOTINTERESTED);
	}
	bool debugRendering = false;


	void replaceMan(middle::GameState* gameState) {
		auto ps = oPosCache->begin<components::Position>();
		for (middle::Id& id : oPosCache->relevantIdVector) {
			middle::queueComponentDeletion<components::Position>(gameState, id);
		}
	}

	void setTransform(middle::RenderItem& item, components::GlobalTransform* transform) {
		item.transform.translation = transform->pos;
		item.transform.scale = transform->scale;
		item.transform.rotation = transform->rotation;
	}


	Color getBubbleColor(middle::GameState* gameState, middle::Id id, components::BubbleComponent* bubble) {
		Color color;
		int depth = bubble::findBubbleDepth(gameState, id);
		bool isEven = depth % 2 == 0;

		{
			color = isEven ? bubbleColors::BUBBLE_BACKGROUND_EVEN : bubbleColors::BUBBLE_BACKGROUND_UNEVEN;
		}
		return color;
	}

	void update(middle::GameState* gameState) override {

		gameState->editorState.backgroundColor = bubbleColors::BACKGROUND;

		// render bubbbles
		components::TextureComponent* editThisComp = nullptr;
		if (editThisCache->getSize() == 1) {
			auto it = editThisCache->begin<components::TextureComponent>();
			editThisComp = *it;
		}
		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		auto bubbleCircleIt = bubbleCache->begin<components::Circle>();
		auto bubbleLayerIt = bubbleCache->begin<components::Layer>();
		auto bubbleTransform = bubbleCache->begin<components::GlobalTransform>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto bubble = *bubbleIt;
			auto circle = *bubbleCircleIt;
			auto layer = *bubbleLayerIt;
			auto transform = *bubbleTransform;

			auto& shape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);
			bool isUiItem = middle::getComponent<components::UiComponent>(shape);
			bool isHighlighted = middle::getComponent<components::UnIntersectableWindowComponent>(shape);

			auto intersectable = middle::getComponent<components::IntersectingTag>(shape);
			bool intersecting = intersectable && intersectable->intersectingTop;


			float radius = intersecting ? circle->radius * 1.05f : circle->radius;

			Color backgroundColor = getBubbleColor(gameState, shape.id, bubble);
			if (isHighlighted) {
				backgroundColor = bubbleColors::HIGHLIGHT_COLOR;
			}
			//Color backgroundColor = bubble->inverse ? bubbleColors::BUBBLE_BACKGROUND_INVERSE : bubbleColors::BUBBLE_BACKGROUND;
			middle::RenderItem circleItem;
			circleItem.type = middle::RenderItemType::CIRCLE;
			circleItem.color = bubbleColors::BUBBLE_OUTLINE;
			circleItem.layer = layer->layer;
			circleItem.backgroundColor = backgroundColor;
			circleItem.radius = radius;
			circleItem.center = { 0,0,0 };
			circleItem.disableDepthTest = isUiItem;
			setTransform(circleItem, transform);
			gameState->renderData.push_back(circleItem);
		}


		// renderunits
		auto unitIt = unitCache->begin<components::BubbleUnit>();
		auto unitLayerIt = unitCache->begin<components::Layer>();
		auto unitTransformIt = unitCache->begin<components::GlobalTransform>();
		auto unitCircleIt = unitCache->begin<components::Circle>();
		for (int i = 0; i < unitCache->getSize(); ++i) {
			auto unit = *unitIt;
			auto layer = *unitLayerIt;
			auto transform = *unitTransformIt;
			auto circle = *unitCircleIt;
			middle::RenderItem unitItem;
			unitItem.type = middle::RenderItemType::TEXT;
			if (unit->value > 0) {
				unitItem.text = "+";
			}
			else {
				unitItem.text = "-";
			}
			middle::Id id = unitCache->relevantIdVector[i];
			setTransform(unitItem, transform);

			auto& shape = middle::getShape(gameState, unitCache->relevantIdVector[i].index);
			auto intersectable = middle::getComponent<components::IntersectingTag>(shape);
			float fontSize = 50;
			if (intersectable && intersectable->intersectingTop) {
				fontSize *= 1.2f;
			}

			unitItem.center = { 0,0,0 };
			unitItem.layer = layer->layer;
			unitItem.fontSize = fontSize;
			unitItem.textOffset.x = -circle->radius * 0.5f;
			unitItem.textOffset.z = circle->radius * 1.1f;
			unitItem.color = unit->value < 0 ? bubbleColors::NEGATIVE_UNIT : bubbleColors::POSITIVE_UNIT;
			gameState->renderData.push_back(unitItem);
		}

		// render variables
		auto variableIt = variableCache->begin<components::BubbleVariable>();
		auto variableBubbleIt = variableCache->begin<components::BubbleComponent>();
		auto variableCircleIt = variableCache->begin<components::Circle>();
		auto layerIt = variableCache->begin<components::Layer>();
		auto varTransformIt = variableCache->begin<components::GlobalTransform>();
		for (int i = 0; i < variableCache->getSize(); ++i) {
			auto variable = *variableIt;
			auto bubble = *variableBubbleIt;
			auto layer = *layerIt;
			auto circle = *variableCircleIt;
			auto transform = *varTransformIt;
			auto& shape = middle::getShape(gameState, variableCache->relevantIdVector[i].index);
			bool isUiItem = middle::getComponent<components::UiComponent>(shape);

			// todo temp
			if (circle->radius < bubble::variableRadius) {
				circle->radius = bubble::variableRadius;
			}

			auto intersectable = middle::getComponent<components::IntersectingTag>(shape);

			float radius = circle->radius;
			float fontSize = bubble::variableTextFontSize;
			if (intersectable && intersectable->intersectingTop) {
				fontSize *= 1.2f;
				radius *= 1.2f;
			}


			middle::RenderItem variableText;
			variableText.type = middle::RenderItemType::TEXT;
			setTransform(variableText, transform);
			variableText.color = bubbleColors::POSITIVE_UNIT;
			variableText.text = variable->label;
			if (variable->isNegative) {
				variableText.color = bubbleColors::NEGATIVE_UNIT;
				variableText.text = "-" + variable->label;
			}
			variableText.textOffset.x = -circle->radius * 0.42f;
			variableText.textOffset.z = circle->radius * 1.5f;
			Color backgroundColor = getBubbleColor(gameState, shape.id, bubble);
			variableText.fontSize = fontSize;
			variableText.disableDepthTest = isUiItem;
			gameState->renderData.push_back(variableText);
		}


		// render muls
		auto mulIt = mulCache->begin<components::BubbleMultiplyComponent>();
		auto mulCircleIt = mulCache->begin<components::Circle>();
		auto mulTransformIt = mulCache->begin<components::GlobalTransform>();
		auto mulLayerIt = mulCache->begin<components::Layer>();
		for (int i = 0; i < mulCache->getSize(); ++i) {
			auto multiplyComponent = *mulIt;
			auto circle = *mulCircleIt;
			auto transform = *mulTransformIt;
			auto layer = *mulLayerIt;

			middle::RenderItem mulCircle;
			mulCircle.type = middle::RenderItemType::CIRCLE;
			mulCircle.transform.translation = transform->pos;
			mulCircle.transform.scale = transform->scale;
			mulCircle.transform.rotation = transform->rotation;
			mulCircle.radius = circle->radius + 4;
			mulCircle.color = GREEN;
			mulCircle.layer = layer->layer + 2;
			gameState->renderData.push_back(mulCircle);

		}

		// renderPowers
		auto powerLoopIt = powerCache->begin<components::LoopSociety>();
		auto powerCircleIt = powerCache->begin<components::Circle>();
		auto powerTransformIt = powerCache->begin<components::GlobalTransform>();
		auto powerLayerIt = powerCache->begin<components::Layer>();
		for (middle::Id powerId : powerCache->relevantIdVector) {
			auto loop = *powerLoopIt;
			auto circle = *powerCircleIt;
			auto transform = *powerTransformIt;
			auto layer = *powerLayerIt;

			middle::RenderItem circleItemA;
			circleItemA.type = middle::RenderItemType::CIRCLE;
			circleItemA.transform.translation = transform->pos;
			circleItemA.transform.scale = transform->scale;
			circleItemA.transform.rotation = transform->rotation;
			circleItemA.radius = circle->radius;
			circleItemA.color = RED;
			circleItemA.layer = layer->layer + 1;
			gameState->renderData.push_back(circleItemA);

			assert(loop->loopMemberIds.size() == 2);
			middle::Id exponentId = loop->loopMemberIds[components::PowerRole::POWER_ROLE_EXPONENT];
			auto& childShape = middle::getShape(gameState, exponentId.index);
			auto exponentTransform = middle::getComponent<components::GlobalTransform>(childShape);
			auto exponentCircle = middle::getComponent<components::Circle>(childShape);
			if (exponentCircle) {
				middle::RenderItem circleItemB;
				circleItemB.type = middle::RenderItemType::CIRCLE;
				circleItemB.transform.translation = exponentTransform->pos;
				circleItemB.transform.scale = exponentTransform->scale;
				circleItemB.transform.rotation = exponentTransform->rotation;
				circleItemB.radius = exponentCircle->radius - 3;
				circleItemB.color = ORANGE;
				circleItemB.layer = layer->layer + 2;
				gameState->renderData.push_back(circleItemB);
			}
		}

		auto cuboidIt = cuboidCache->begin<components::Cuboid>();
		auto cuboidTransformIt = cuboidCache->begin<components::GlobalTransform>();
		for (int i = 0; i < cuboidCache->getSize(); ++i) {
			auto cuboid = *cuboidIt;
			auto transform = *cuboidTransformIt;
			middle::RenderItem cuboidItem;
			cuboidItem.type = middle::RenderItemType::CUBOID;
			cuboidItem.width = cuboid->width;
			cuboidItem.height = cuboid->height;
			cuboidItem.length = cuboid->length;
			cuboidItem.color = bubbleColors::BACKGROUND;
			// TODO
			cuboidItem.color.a = 30;
			cuboidItem.center = transform->pos;
			gameState->renderData.push_back(cuboidItem);
		}

		auto equTransformIt = equalsCache->begin<components::GlobalTransform>();
		auto equCircleIt = equalsCache->begin<components::Circle>();
		auto equLayerIt = equalsCache->begin<components::Layer>();
		for (middle::Id id : equalsCache->relevantIdVector){
			auto transform = *equTransformIt;
			auto circle = *equCircleIt;;
			auto layer = *equLayerIt;

			middle::RenderItem equCirc;
			equCirc.type = middle::RenderItemType::CIRCLE;
			equCirc.radius = circle->radius + 8;
			setTransform(equCirc, transform);
			equCirc.layer = layer->layer;
			equCirc.color = bubbleColors::EQUALS_CONNECTION;
			gameState->renderData.push_back(equCirc);
		}

		auto inequTransformIt = inequCache->begin<components::GlobalTransform>();
		auto inequCircleIt = inequCache->begin<components::Circle>();
		auto inequLayerIt = inequCache->begin<components::Layer>();
		for (middle::Id id : inequCache->relevantIdVector){
			auto transform = *inequTransformIt;
			auto circle = *inequCircleIt;;
			auto layer = *inequLayerIt;

			middle::RenderItem equCirc;
			equCirc.type = middle::RenderItemType::CIRCLE;
			equCirc.radius = circle->radius + 4;
			setTransform(equCirc, transform);
			equCirc.layer = layer->layer + 1;
			equCirc.color = bubbleColors::INEQUALS_COLOR;
			gameState->renderData.push_back(equCirc);

			middle::Id lesser, greater;
			bubble::getInequaltyLesserAndGreater(gameState, id, lesser, greater);

			middle::RenderItem greaterCirc;
			greaterCirc.type = middle::RenderItemType::CIRCLE;
			auto greaterTransform = middle::getComp<components::GlobalTransform>(gameState, greater);
			setTransform(greaterCirc, greaterTransform);
			auto greaterCircleComp = middle::getComp<components::Circle>(gameState, greater);
			greaterCirc.radius = greaterCircleComp->radius;
			greaterCirc.layer = layer->layer + 2;
			greaterCirc.color = bubbleColors::INEQUALS_COLOR;
			gameState->renderData.push_back(greaterCirc);
		}




		auto textureIt = textureCache->begin<components::TextureComponent>();
		auto textureTransformIt = textureCache->begin<components::GlobalTransform>();
		for (int i = 0; i < textureCache->getSize(); ++i) {
			auto texture = *textureIt;
			auto transform = *textureTransformIt;
			auto& shape = middle::getShape(gameState, textureCache->relevantIdVector[i].index);
			bool isUiItem = middle::getComponent<components::UiComponent>(shape);
			auto layer = middle::getComponent<components::Layer>(shape);

			if (texture->textureType == middleTextureType::BILLBOARD) {
				middle::RenderItem textureItem;
				textureItem.type = middle::RenderItemType::BILLBOARD;
				textureItem.texture = &texture->texture;
				textureItem.transform.translation = transform->pos;
				textureItem.color = WHITE;
				textureItem.textureScale = texture->scale;
				textureItem.disableDepthTest = isUiItem;
				if (layer) {
					textureItem.layer = layer->layer;
				}
				gameState->renderData.push_back(textureItem);
			}

			else if (texture->textureType == middleTextureType::BACKGROUND) {
				middle::RenderItem textureItem;
				textureItem.type = middle::RenderItemType::BACKGROUND;
				textureItem.texture = &texture->texture;
				textureItem.transform.translation = transform->pos;
				textureItem.color = WHITE;
				textureItem.textureScale = texture->scale;
				textureItem.disableDepthTest = false;
				textureItem.width = 10000;
				textureItem.height = 10000;
				if (layer) {
					textureItem.layer = layer->layer;
				}
				gameState->renderData.push_back(textureItem);
			}
		}

		// cross hair or something
		middle::RenderItem cameraTarget;
		cameraTarget.type = middle::RenderItemType::CIRCLE;
		cameraTarget.radius = 1;
		cameraTarget.center = gameState->activeCamera.position + Vector3{ 0,100,0 };
		cameraTarget.color = WHITE;
		cameraTarget.disableDepthTest = true;
		gameState->renderData.push_back(cameraTarget);

		// render activity bounding box
		for (middle::Id& id : activeBubbleCache->relevantIdVector) {
			float left, right, top, bottom;
			bubble::bubbleRectBoundingBox(gameState, id, &left, &right, &bottom, &top);
			middle::RenderItem boundingRect;
			boundingRect.type = middle::RenderItemType::RECTANGLE;
			boundingRect.width = right - left;
			boundingRect.height = top - bottom;
			boundingRect.center = { (left + right) * 0.5f, 0, (bottom + top) * 0.5f };
			boundingRect.color = WHITE;
			gameState->renderData.push_back(boundingRect);
		}


		if (procContainerCache->getSize() > 0) {
			auto procContainerIt = procContainerCache->begin<components::ProcedureContainer>();
			auto procContainer = *procContainerIt;

			auto inputIt = inputCache->begin<components::InputVariable>();
			auto intersectabeInputIt = inputCache->begin<components::IntersectingTag>();
			for (int i = 0; i < inputCache->getSize(); ++i) {
				auto input = *inputIt;
				auto intersectable = *intersectabeInputIt;

				// render hover effect
				if (intersectable->intersectingTop) {
					middle::RenderItem hovering;
					hovering.type = middle::RenderItemType::CIRCLE;
					hovering.color = bubbleColors::INPUT_HOVER_COLOR;
					const float hoveringInputIndicatorRadius = 3;
					hovering.radius = hoveringInputIndicatorRadius;
					hovering.center = middle::getGlobalPosition(gameState, inputCache->relevantIdVector[i].index);
					hovering.disableDepthTest = true;
					hovering.layer = 3;
					gameState->renderData.push_back(hovering);
				}


				// render line from input to bubble
				Vector3 p1, p2;
				bool renderLine = false;

				// todo generation checks should be always...
				if (input->unitRef.index != middle::UNASSIGNED
					&& input->unitRef.generation == gameState->ids[input->unitRef.index].generation
					&& middle::isValidId(gameState, input->unitRef)) {
					p1 = middle::getGlobalPosition(gameState, inputCache->relevantIdVector[i].index);
					p2 = middle::getGlobalPosition(gameState, input->unitRef.index);
					renderLine = true;
				}

				middle::Id grabbedId = gameState->bubbleAlgebraState.grabbedId;
				if (!renderLine && grabbedId.index != middle::UNASSIGNED) {
					auto& shape = middle::getShape(gameState, grabbedId.index);
					auto idRef = middle::getComponent<components::IdRef>(shape);
					if (!idRef) {
						continue;
					}
					auto& ogShape = middle::getShape(gameState, idRef->idRef.index);
					if (ogShape.id == inputCache->relevantIdVector[i]) {
						renderLine = true;
						p1 = middle::getGlobalPosition(gameState, ogShape.id.index);
						p2 = gameState->input.mouseXZ_PlanePos;
					}
				}

				if (renderLine) {
					middle::RenderItem line;
					line.type = middle::RenderItemType::LINE;
					line.linePointA = p1;
					line.linePointB = p2;
					line.color = bubbleColors::HIGHLIGHT_COLOR;
					line.disableDepthTest = true;
					line.layer = 4;
					gameState->renderData.push_back(line);
				}
			}
		}

		}


};

static middle::SystemRegistrar<BubbleRenderSetup> reg("BubbleRenderSetup");
