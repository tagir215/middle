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
#include "MouseIntersectable.h"
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
#include "BubbleAlgebraProblem.h"
#include "HelperBubbleEquation.h"
#include "EditThisTag.h"
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
#include "BubbleExponentTag.h"


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
		unitCache = middle::newCompCache(gameState, systemName);
		unitCache->addType<components::BubbleUnit>();
		unitCache->addType<components::Layer>();
		unitCache->addType<components::GlobalTransform>();
		mulCache = middle::newCompCache(gameState, systemName);
		mulCache->addType<components::BubbleMultiplyComponent>();
		mulCache->addType<components::LoopSociety>();
		mulCache->addType<components::GlobalTransform>();
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
		equalsCache->addType<components::LoopSociety>();
		equalsCache->addType<components::GlobalTransform>();
		textureCache = middle::newCompCache(gameState, systemName);
		textureCache->addType<components::TextureComponent>();
		textureCache->addType<components::GlobalTransform>();
		textureCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		editThisCache = middle::newCompCache(gameState, systemName);
		editThisCache->addType<components::EditThisTag>();
		editThisCache->addType<components::TextureComponent>();
		inputCache = middle::newCompCache(gameState, systemName);
		inputCache->addType<components::InputVariable>();
		inputCache->addType<components::MouseIntersectable>();
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

			// todo maybe replace with tag
			bool parentIsEditableEquals = false;
			middle::Id parentId = middle::getParent(gameState, shape.id);
			if (parentId.index != middle::UNASSIGNED) {
				auto& parentShape = middle::getShape(gameState, parentId.index);
				bool equals = middle::getComponent<components::BubbleEqualsComponent>(parentShape) != nullptr;
				auto problem = middle::getComponent<components::BubbleAlgebraProblem>(parentShape);
				auto helper = middle::getComponent<components::HelperBubbleEquation>(parentShape);
				parentIsEditableEquals = equals && ((problem && problem->editable) || helper);
			}

			// check if should add editable indicator
			auto problem = middle::getComponent<components::BubbleAlgebraProblem>(shape);
			auto helper = middle::getComponent<components::HelperBubbleEquation>(shape);
			bool addEditableTag = (problem && problem->editable) || helper || parentIsEditableEquals;

			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
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

			if (addEditableTag && editThisComp) {
				middle::RenderItem editThisSign;
				editThisSign.type = middle::RenderItemType::BILLBOARD;
				editThisSign.color = WHITE;
				const float editThisZOffset = 20;
				editThisSign.textureScale = editThisComp->scale;
				editThisSign.disableDepthTest = isUiItem;
				Vector3 pos = circleItem.transform.translation;
				editThisSign.transform.translation = { pos.x, pos.y, pos.z + circle->radius + editThisZOffset };
				editThisSign.texture = &editThisComp->texture;
				editThisSign.layer = -1;
				editThisSign.disableDepthTest = true;
				gameState->renderData.push_back(editThisSign);
			}
		}

		// renderunits
		auto unitIt = unitCache->begin<components::BubbleUnit>();
		auto unitLayerIt = unitCache->begin<components::Layer>();
		auto unitTransformIt = unitCache->begin<components::GlobalTransform>();
		for (int i = 0; i < unitCache->getSize(); ++i) {
			auto unit = *unitIt;
			auto layer = *unitLayerIt;
			auto transform = *unitTransformIt;
			middle::RenderItem unitItem;
			unitItem.type = middle::RenderItemType::TEXT;
			unitItem.text = std::to_string(unit->value);
			middle::Id id = unitCache->relevantIdVector[i];
			setTransform(unitItem, transform);
			unitItem.center = { 0,0,0 };
			unitItem.layer = layer->layer;
			unitItem.fontSize = 25;
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

			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);

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
			Color backgroundColor = getBubbleColor(gameState, shape.id, bubble);
			variableText.fontSize = fontSize;
			variableText.disableDepthTest = isUiItem;
			gameState->renderData.push_back(variableText);
		}


		// render muls
		auto mulIt = mulCache->begin<components::BubbleMultiplyComponent>();
		for (int i = 0; i < mulCache->getSize(); ++i) {
			auto multiplyComponent = *mulIt;
			std::vector<middle::Id>children;
			middle::getChildren(gameState, mulCache->relevantIdVector[i], children);
			for (int x = 1; x < children.size(); ++x) {
				auto& shapeA = middle::getShape(gameState, children[x - 1].index);
				auto& shapeB = middle::getShape(gameState, children[x].index);
				auto transformA = middle::getComponent<components::GlobalTransform>(shapeA);
				auto transformB = middle::getComponent<components::GlobalTransform>(shapeB);
				auto circleA = middle::getComponent<components::Circle>(shapeA);
				auto circleB = middle::getComponent<components::Circle>(shapeB);
				auto layer = middle::getComponent<components::Layer>(shapeA);
				bool isUiItem = middle::getComponent<components::UiComponent>(shapeA);
				float radiusA = circleA->radius * transformA->scale.x;
				float radiusB = circleB->radius * transformB->scale.x;

				if (!circleA || !circleB)
					continue;
				Vector3 posA = transformA->pos;
				Vector3 posB = transformB->pos;
				Vector3 axis = Vector3Normalize(Vector3Subtract(posB, posA));
				middle::RenderItem line;
				line.type = middle::RenderItemType::LINE;
				line.linePointA = posA + Vector3Scale(axis, radiusA);
				line.linePointB = posB + Vector3Scale(axis, -radiusB);
				line.color = bubbleColors::MULTIPLICATION_CONNECTION;
				line.layer = layer->layer;
				line.disableDepthTest = isUiItem;

				gameState->renderData.push_back(line);
			}
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
			for (middle::Id childId : loop->loopMemberIds) {
				auto& childShape = middle::getShape(gameState, childId.index);
				if (!middle::getComponent<components::BubbleExponentTag>(childShape)) {
					continue;
				}
				auto exponentTransform = middle::getComponent<components::GlobalTransform>(childShape);
				auto exponentCircle = middle::getComponent<components::Circle>(childShape);
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

		for (int i = 0; i < equalsCache->getSize(); ++i) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, equalsCache->relevantIdVector[i], children);

			assert(children.size() == 2);
			middle::Id& idA = children[0];
			middle::Id& idB = children[1];
			middle::Shape& shapeA = middle::getShape(gameState, idA.index);
			middle::Shape& shapeB = middle::getShape(gameState, idB.index);
			Vector3 posA = middle::getGlobalPosition(gameState, idA.index);
			Vector3 posB = middle::getGlobalPosition(gameState, idB.index);
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
			auto intersectabeInputIt = inputCache->begin<components::MouseIntersectable>();
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
