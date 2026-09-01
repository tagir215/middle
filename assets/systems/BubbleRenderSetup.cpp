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
#include "BubbleFunctionComponent.h"
#include "GlobalRect.h"
#include "BubbleSummationComponent.h"
#include "bubble_paths.h"
#include "imgui.h"


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
	components::CompCache* functionCache;
	components::CompCache* summationCache;

			const float scaleCorrection = 10.2f;

	void init(middle::GameState* gameState) {
		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::Rectangle>();
		bubbleCache->addType<components::Layer>();
		bubbleCache->addType<components::LoopSociety>();
		bubbleCache->addType<components::GlobalTransform>();
		bubbleCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubblePowerComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleSummationComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleFunctionComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleMultiplyComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleVariable>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleUnit>(components::NOTINTERESTED);
		bubbleCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		unitCache = middle::newCompCache(gameState, systemName);
		unitCache->addType<components::BubbleUnit>();
		unitCache->addType<components::Layer>();
		unitCache->addType<components::GlobalTransform>();
		unitCache->addType<components::Rectangle>();
		mulCache = middle::newCompCache(gameState, systemName);
		mulCache->addType<components::BubbleMultiplyComponent>();
		mulCache->addType<components::LoopSociety>();
		mulCache->addType<components::GlobalTransform>();
		mulCache->addType<components::Rectangle>();
		mulCache->addType<components::Layer>();
		fractionCache = middle::newCompCache(gameState, systemName);
		fractionCache->addType<components::FractionalComponent>();
		fractionCache->addType<components::LoopSociety>();
		fractionCache->addType<components::GlobalTransform>();
		variableCache = middle::newCompCache(gameState, systemName);
		variableCache->addType<components::BubbleComponent>();
		variableCache->addType<components::Layer>();
		variableCache->addType<components::BubbleVariable>();
		variableCache->addType<components::Rectangle>();
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
		equalsCache->addType<components::Rectangle>();
		equalsCache->addType<components::GlobalTransform>();
		inequCache = middle::newCompCache(gameState, systemName);
		inequCache->addType<components::BubbleInequaltyComponent>();
		inequCache->addType<components::Layer>();
		inequCache->addType<components::Rectangle>();
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
		activeBubbleCache->addType<components::GlobalRect>();
		powerCache = middle::newCompCache(gameState, systemName);
		powerCache->addType<components::BubblePowerComponent>();
		powerCache->addType<components::Rectangle>();
		powerCache->addType<components::LoopSociety>();
		powerCache->addType<components::GlobalTransform>();
		powerCache->addType<components::Layer>();
		functionCache = middle::newCompCache(gameState, systemName);
		functionCache->addType<components::BubbleFunctionComponent>();
		functionCache->addType<components::GlobalTransform>();
		functionCache->addType<components::Layer>();
		functionCache->addType<components::Rectangle>();
		functionCache->addType<components::GlobalRect>();
		summationCache = middle::newCompCache(gameState, systemName);
		summationCache->addType<components::BubbleSummationComponent>();
		summationCache->addType<components::Layer>();
		summationCache->addType<components::Rectangle>();
		summationCache->addType<components::GlobalTransform>();

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

	int getCircleSlices(components::GlobalTransform* transform) {
		int slices = (int)(transform->scale.x * 30);
		const int maxSlices = 200;
		const int minSlices = 10;
		slices = slices > minSlices ? slices : minSlices;
		slices = slices < maxSlices ? slices : maxSlices;
		return slices;
	}

	void setTransform(middle::RenderItem& item, components::GlobalTransform* transform) {
		item.transform.translation = transform->pos;
		item.transform.scale = transform->scale;
		item.transform.rotation = transform->rotation;
	}

	enum class LabelPos {
		LEFT,
		CENTER,
		RIGHT
	};

	void renderBubbleLabel(middle::GameState* gameState, components::GlobalTransform* transform, float height, 
		const std::string& label, int layer, LabelPos pos, const Color& color) {
		middle::RenderItem text;
		text.type = middle::RenderItemType::TEXT;
		text.text = label;
		text.color = color;
		const float labelFontSize = 20;
		const float offsetFactor = 0.1f;
		float offset = height * offsetFactor * transform->scale.z;
		float axis = height * 0.5f * transform->scale.z;
		text.fontSize = labelFontSize;
		text.layer = layer + 1;
		if(pos == LabelPos::LEFT)
			text.transform.translation = transform->pos + Vector3{ -axis + offset,0, axis - offset };
		else if (pos == LabelPos::CENTER)
			text.transform.translation = transform->pos + Vector3{ 0,0, axis - offset };
		else if (pos == LabelPos::RIGHT)
			text.transform.translation = transform->pos + Vector3{ axis + offset, axis - offset };
		text.transform.scale = transform->scale;
		text.transform.rotation = transform->rotation;
		gameState->renderData.push_back(text);
	}


	Color calculateFadedColor(middle::GameState* gameState, const Color& color, components::GlobalTransform* transform, int layer) {
		const Color background = bubbleColors::BACKGROUND;

		const float oneChildScaleRatio = 0.758;
		const float stepScale = 1.0f / oneChildScaleRatio;
		float layerOffset = 0;

		float camDist = gameState->activeCamera.position.y;
		// todo... is cosntant
		float axisY = gameState->nearPlaneAxisY / gameState->nearPlaneDistance * -camDist;

		const float firstStepScale = axisY / bubble::bubbleAxis;

		float maxScale = firstStepScale;
		float minScale = 0.0001f;

		float scaleRatio = transform->scale.x / maxScale;
		if (scaleRatio > 1) {
			scaleRatio = 1;
		}

		float s = scaleRatio;

		s = std::powf(s, 0.17f);

		Color result;
		result.r = color.r * s + background.r * (1 - s);
		result.g = color.g * s + background.g * (1 - s);
		result.b = color.b * s + background.b * (1 - s);
		result.a = 255;

		return result;


		//if (gameState->bubbleAlgebraState.worldScale * bubble::bubbleAxis > axisY) {
		//	float overFlowScale = gameState->bubbleAlgebraState.worldScale / firstStepScale;
		//	float layersPassed = std::log(overFlowScale) / std::log(stepScale);

		//	layerOffset = layersPassed;

		//	auto ui = [gameState, layersPassed]() {
		//		ImGui::Begin("xxx");
		//		ImGui::Text(std::to_string(layersPassed).c_str());
		//		ImGui::End();
		//		};
		//	gameState->uiSetups.push_back(ui);
		//}

		//float layerY = layer - layerOffset;

		//if (layerY > 0) {
		//	s = 1.0f / (layerY + 1);

		//	s = std::powf(s, 0.1f);

		//	Color result = color;
		//	result.r = color.r * s + background.r * (1 - s);
		//	result.g = color.g * s + background.g * (1 - s);
		//	result.b = color.b * s + background.b * (1 - s);
		//	result.a = 255;

		//	return result;
		//}
		//else {
		//	s = 1.0f / (-layerY + 1);

		//	s = std::powf(s, 0.2f);

		//	Color result;
		//	result.r = color.r * s + background.r * (1 - s);
		//	result.g = color.g * s + background.g * (1 - s);
		//	result.b = color.b * s + background.b * (1 - s);
		//	result.a = 255;
		//	return result;
		//}
	}

	void renderBubble(middle::GameState* gameState, int layer, Color color, components::GlobalTransform* transform) {

		//middle::RenderItem rect;
		//rect.type = middle::RenderItemType::RECTANGLE;
		//rect.color = BLACK;
		//setTransform(rect, transform);
		//rect.layer = layer;
		//rect.width = bubble::bubbleAxis * 2;
		//rect.height = bubble::bubbleAxis * 2;
		//rect.length = 0;
		//rect.backgroundColor = color;
		//gameState->renderData.push_back(rect);

		middle::RenderItem texture;
		texture.type = middle::RenderItemType::BILLBOARD;
		texture.shader = &gameState->shaderMap[bubbleShaderNames::BUBBLE_SHADER].shader;
		texture.texture = &gameState->textureMap[bubbleTextureNames::TEXTURE_BACKGROUND].texture;
		texture.layer = layer;
		setTransform(texture, transform);
		texture.transform.scale.x *= scaleCorrection;
		texture.transform.scale.y *= scaleCorrection;
		texture.transform.scale.z *= scaleCorrection;
		texture.color = color;
		gameState->renderData.push_back(texture);
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
		auto bubbleRectIt = bubbleCache->begin<components::Rectangle>();
		auto bubbleLayerIt = bubbleCache->begin<components::Layer>();
		auto bubbleTransform = bubbleCache->begin<components::GlobalTransform>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto bubble = *bubbleIt;
			auto rect = *bubbleRectIt;
			auto layer = *bubbleLayerIt;
			auto transform = *bubbleTransform;

			auto& shape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);
			bool isUiItem = middle::getComponent<components::UiComponent>(shape);
			bool isHighlighted = middle::getComponent<components::UnIntersectableWindowComponent>(shape);

			auto intersectable = middle::getComponent<components::IntersectingTag>(shape);
			bool intersecting = intersectable && intersectable->intersectingTop;

			Color backgroundColor = calculateFadedColor(gameState, bubbleColors::BUBBLE, transform, layer->layer);

			//middle::RenderItem debugRect;
			//debugRect.type = middle::RenderItemType::RECTANGLE;
			//setTransform(debugRect, transform);
			//debugRect.color = BLUE;
			//debugRect.layer = layer->layer;
			//debugRect.width = bubble::bubbleAxis * 2;
			//debugRect.height = bubble::bubbleAxis * 2;
			//debugRect.length = 0;
			//gameState->renderData.push_back(debugRect);

			renderBubble(gameState, layer->layer, backgroundColor, transform);
		}


		// renderunits
		auto unitIt = unitCache->begin<components::BubbleUnit>();
		auto unitLayerIt = unitCache->begin<components::Layer>();
		auto unitTransformIt = unitCache->begin<components::GlobalTransform>();
		auto unitRectIt = unitCache->begin<components::Rectangle>();
		for (int i = 0; i < unitCache->getSize(); ++i) {
			auto unit = *unitIt;
			auto layer = *unitLayerIt;
			auto transform = *unitTransformIt;
			auto rect = *unitRectIt;
			middle::RenderItem unitItem;
			unitItem.type = middle::RenderItemType::TEXT;
			Color textColor;
			Color backgroundColor;
			middle::Id id = unitCache->relevantIdVector[i];
			if (unit->value > 0) {
				unitItem.text = "+";
				textColor = bubbleColors::UNIT_TEXT_POSITIVE;
				backgroundColor = calculateFadedColor(gameState, bubbleColors::POSITIVE_UNIT, transform, layer->layer);
			}
			else {
				unitItem.text = "-";
				textColor = bubbleColors::UNIT_TEXT_NEGATIVE;
				backgroundColor = calculateFadedColor(gameState, bubbleColors::NEGATIVE_UNIT, transform, layer->layer);
			}
			setTransform(unitItem, transform);

			auto& shape = middle::getShape(gameState, unitCache->relevantIdVector[i].index);
			auto intersectable = middle::getComponent<components::IntersectingTag>(shape);
			float fontSize = 50;
			if (intersectable && intersectable->intersectingTop) {
				fontSize *= 1.2f;
			}

			unitItem.color = textColor;
			unitItem.textOffset.x = -rect->width * 0.42f;
			unitItem.textOffset.z = rect->height * 1.5f;
			unitItem.fontSize = bubble::bubbleFontSize;
			gameState->renderData.push_back(unitItem);

			renderBubble(gameState, layer->layer, backgroundColor, transform);
		}

		// render variables
		auto variableIt = variableCache->begin<components::BubbleVariable>();
		auto variableBubbleIt = variableCache->begin<components::BubbleComponent>();
		auto variableRectIt = variableCache->begin<components::Rectangle>();
		auto layerIt = variableCache->begin<components::Layer>();
		auto varTransformIt = variableCache->begin<components::GlobalTransform>();
		for (int i = 0; i < variableCache->getSize(); ++i) {
			auto variable = *variableIt;
			auto bubble = *variableBubbleIt;
			auto layer = *layerIt;
			auto rect = *variableRectIt;
			auto transform = *varTransformIt;
			auto id = variableCache->relevantIdVector[i];

			std::string varText;
			Color colorText;
			Color colorBackground;
			if (!variable->isNegative) {
				colorText = bubbleColors::UNIT_TEXT_POSITIVE;
				colorBackground = calculateFadedColor(gameState, bubbleColors::POSITIVE_UNIT, transform, layer->layer);
			}
			else {
				varText = "-";
				colorText = bubbleColors::UNIT_TEXT_NEGATIVE;
				colorBackground = calculateFadedColor(gameState, bubbleColors::NEGATIVE_UNIT, transform, layer->layer);
			}
			varText += variable->label;

			renderBubble(gameState, layer->layer, colorBackground, transform);

			middle::RenderItem variableText;
			variableText.type = middle::RenderItemType::TEXT;
			setTransform(variableText, transform);
			variableText.text = varText;
			variableText.color = colorText;
			variableText.textOffset.x = -rect->width * 0.42f;
			variableText.textOffset.z = rect->height * 1.5f;
			variableText.fontSize = bubble::bubbleFontSize;
			gameState->renderData.push_back(variableText);

		}


		// render muls
		auto mulIt = mulCache->begin<components::BubbleMultiplyComponent>();
		auto mulRectIt = mulCache->begin<components::Rectangle>();
		auto mulTransformIt = mulCache->begin<components::GlobalTransform>();
		auto mulLayerIt = mulCache->begin<components::Layer>();
		for (middle::Id id : mulCache->relevantIdVector) {
			auto multiplyComponent = *mulIt;
			auto rect = *mulRectIt;
			auto transform = *mulTransformIt;
			auto layer = *mulLayerIt;

			renderBubble(gameState, layer->layer, calculateFadedColor(gameState, bubbleColors::MULTIPLICATION, transform, layer->layer), transform);

			renderBubbleLabel(gameState, transform, rect->height, u8"\u00D7",
				layer->layer, LabelPos::CENTER, bubbleColors::MULTIPLICATION_TEXT);
		}

		// renderPowers
		auto powerLoopIt = powerCache->begin<components::LoopSociety>();
		auto powerRectIt = powerCache->begin<components::Rectangle>();
		auto powerTransformIt = powerCache->begin<components::GlobalTransform>();
		auto powerLayerIt = powerCache->begin<components::Layer>();
		for (middle::Id powerId : powerCache->relevantIdVector) {
			auto loop = *powerLoopIt;
			auto rect = *powerRectIt;
			auto transform = *powerTransformIt;
			auto layer = *powerLayerIt;

			renderBubble(gameState, layer->layer, calculateFadedColor(gameState, bubbleColors::POWER, transform, layer->layer), transform);

			renderBubbleLabel(gameState, transform, rect->height, "^",
				layer->layer, LabelPos::CENTER, bubbleColors::POWER_TEXT);
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
		auto equCircleIt = equalsCache->begin<components::Rectangle>();
		auto equLayerIt = equalsCache->begin<components::Layer>();
		for (middle::Id id : equalsCache->relevantIdVector) {
			auto transform = *equTransformIt;
			auto rect = *equCircleIt;;
			auto layer = *equLayerIt;

			renderBubble(gameState, layer->layer, calculateFadedColor(gameState, bubbleColors::EQUALS, transform, layer->layer), transform);

			renderBubbleLabel(gameState, transform, rect->height, "=",
				layer->layer, LabelPos::CENTER, bubbleColors::EQUALS_TEXT);
		}

		auto inequTransformIt = inequCache->begin<components::GlobalTransform>();
		auto inequRectIt = inequCache->begin<components::Rectangle>();
		auto inequLayerIt = inequCache->begin<components::Layer>();
		for (middle::Id id : inequCache->relevantIdVector) {
			auto transform = *inequTransformIt;
			auto rect = *inequRectIt;
			auto layer = *inequLayerIt;

			renderBubble(gameState, layer->layer, calculateFadedColor(gameState, bubbleColors::INEQUALS, transform, layer->layer), transform);
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
		gameState->renderData.push_back(cameraTarget);

		// render activity bounding box
		auto activeRIt = activeBubbleCache->begin<components::GlobalRect>();
		auto activeTransformIt = activeBubbleCache->begin<components::GlobalTransform>();
		for (middle::Id& id : activeBubbleCache->relevantIdVector) {
			auto globalR = *activeRIt;
			auto transform = *activeTransformIt;
			middle::RenderItem boundingRect;
			boundingRect.type = middle::RenderItemType::RECTANGLE;
			const float margin = 10 * transform->scale.x;
			boundingRect.width = globalR->width + margin;
			boundingRect.height = globalR->height + margin;
			boundingRect.center = transform->pos;
			boundingRect.color = WHITE;
			gameState->renderData.push_back(boundingRect);
		}



		auto functionTransformIt = functionCache->begin<components::GlobalTransform>();
		auto functionIt = functionCache->begin<components::BubbleFunctionComponent>();
		auto functionRectIt = functionCache->begin<components::Rectangle>();
		auto functionLayerIt = functionCache->begin<components::Layer>();
		for (middle::Id id : functionCache->relevantIdVector) {
			// render functionlabel
			auto transform = *functionTransformIt;
			auto rect = *functionRectIt;
			auto func = *functionIt;
			auto layer = *functionLayerIt;

			renderBubbleLabel(gameState, transform, rect->width, func->label + "()",
				layer->layer, LabelPos::CENTER, bubbleColors::FUNCTION_TEXT);

			renderBubble(gameState, layer->layer, calculateFadedColor(gameState, bubbleColors::FUNCTION, transform, layer->layer), transform);

			// render indexes
			std::vector<middle::Id>children;
			middle::getChildren(gameState, id, children);
			int index = 1;
			for (middle::Id childId : children) {
				auto transform = middle::getComp<components::GlobalTransform>(gameState, childId);
				auto globalRChild = middle::getComp<components::GlobalRect>(gameState, childId);
				auto layer = middle::getComp<components::Layer>(gameState, childId);
				renderBubbleLabel(gameState, transform, rect->width, std::to_string(index++),
					layer->layer, LabelPos::LEFT, bubbleColors::FUNCTION_TEXT);
			}
		}

		auto summationTransformIt = summationCache->begin<components::GlobalTransform>();
		auto summationRectIt = summationCache->begin<components::Rectangle>();
		auto summationLayerIt = summationCache->begin<components::Layer>();
		for (middle::Id id : summationCache->relevantIdVector) {
			// render functionlabel
			auto transform = *summationTransformIt;
			auto rect = *summationRectIt;
			auto layer = *summationLayerIt;

			renderBubble(gameState, layer->layer, calculateFadedColor(gameState, bubbleColors::SUMMATION, transform, layer->layer), transform);

			renderBubbleLabel(gameState, transform, rect->width, u8"\u2211",
				layer->layer, LabelPos::CENTER, bubbleColors::SUMMATION_TEXT);
		}
	}

};

static middle::SystemRegistrar<BubbleRenderSetup> reg("BubbleRenderSetup");
