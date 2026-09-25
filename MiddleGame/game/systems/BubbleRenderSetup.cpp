#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "middle_component_table.h"
#include "MidComp/BubbleComponent.h"
#include "MidComp/BubbleMultiplyComponent.h"
#include "MidComp/Sphere.h"
#include "MidComp/BubbleUnit.h"
#include "MidComp/FractionalComponent.h"
#include "MidComp/LoopSociety.h"
#include "MidComp/IntersectingTag.h"
#include "bubble_utils.h"
#include "MidComp/BubbleRef.h"
#include "MidComp/Circle.h"
#include "MidComp/Cuboid.h"
#include "MidComp/BubbleEqualsComponent.h"
#include "MidComp/BubbleVariable.h"
#include "bubble_colors.h"
#include "MidComp/Layer.h"
#include "MidComp/Rectangle.h"
#include "MidComp/UiComponent.h"
#include "MidComp/TextureComponent.h"
#include "MidComp/RuntimeHiddenTag.h"
#include "MidComp/ActiveCheckBoxTag.h"
#include "MidComp/InputVariable.h"
#include "MidComp/ProcedureInputVariable.h"
#include "MidComp/ProcedureContainer.h"
#include "MidComp/CodeBlock.h"
#include "MidComp/IdRef.h"
#include "MidComp/UnIntersectableWindowComponent.h"
#include "MidComp/ActiveSceneEditableTag.h"
#include "MidComp/GlobalTransform.h"
#include "component_utils.h"
#include "MidComp/LocalPosition.h"
#include "MidComp/LocalScale.h"
#include "MidComp/BubblePowerComponent.h"
#include "MidComp/BubbleInequaltyComponent.h"
#include "MidComp/BubbleFunctionComponent.h"
#include "MidComp/GlobalRect.h"
#include "MidComp/BubbleSummationComponent.h"
#include "bubble_paths.h"
#include "imgui.h"
#include "MidComp/BubbleTextComponent.h"
#include "MidComp/BubbleSwapComponent.h"
#include "MidComp/BubbleLogicComponent.h"
#include "MidComp/BubbleGateComponent.h"
#include "MidComp/InViewTag.h"
#include "MidComp/BubbleLockedComponent.h"
#include "MidComp/BubbleManipulatable.h"


class BubbleRenderSetup : public middle::MiddleGameplaySystem {
public:

	BubbleRenderSetup() {
		systemModeType = middle::SystemModeType::ENGINE;
		systemUpdateType = middle::SystemUpdateType::RENDERING;
	}

	components::CompCache* bubbleCache;
	components::CompCache* mulCache;
	components::CompCache* variableCache;
	components::CompCache* equalsCache;
	components::CompCache* nonManipulatableEqualsCache;
	components::CompCache* inequCache;
	components::CompCache* unitCache;
	components::CompCache* activeBubbleCache;
	components::CompCache* powerCache;
	components::CompCache* functionCache;
	components::CompCache* summationCache;
	components::CompCache* textCache;
	components::CompCache* swapCache;
	components::CompCache* logicCache;
	components::CompCache* gateCache;

			const float scaleCorrection = 10.2f;

	void init(middle::GameState* gameState) {
		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::InViewTag>();
		bubbleCache->addType<components::Rectangle>();
		bubbleCache->addType<components::Layer>();
		bubbleCache->addType<components::LoopSociety>();
		bubbleCache->addType<components::GlobalTransform>();
		bubbleCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubblePowerComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleSummationComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleFunctionComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleMultiplyComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleInequaltyComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleLogicComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleTextComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleVariable>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleSwapComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleUnit>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleEqualsComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleGateComponent>(components::NOTINTERESTED);
		bubbleCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		bubbleCache->addType<components::BubbleLockedComponent>(components::NOTINTERESTED);
		unitCache = middle::newCompCache(gameState, systemName);
		unitCache->addType<components::BubbleUnit>();
		unitCache->addType<components::InViewTag>();
		unitCache->addType<components::Layer>();
		unitCache->addType<components::GlobalTransform>();
		unitCache->addType<components::Rectangle>();
		unitCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		unitCache->addType<components::BubbleLockedComponent>(components::NOTINTERESTED);
		mulCache = middle::newCompCache(gameState, systemName);
		mulCache->addType<components::BubbleMultiplyComponent>();
		mulCache->addType<components::InViewTag>();
		mulCache->addType<components::LoopSociety>();
		mulCache->addType<components::GlobalTransform>();
		mulCache->addType<components::Rectangle>();
		mulCache->addType<components::Layer>();
		mulCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		mulCache->addType<components::BubbleLockedComponent>(components::NOTINTERESTED);
		variableCache = middle::newCompCache(gameState, systemName);
		variableCache->addType<components::BubbleComponent>();
		variableCache->addType<components::InViewTag>();
		variableCache->addType<components::Layer>();
		variableCache->addType<components::BubbleVariable>();
		variableCache->addType<components::Rectangle>();
		variableCache->addType<components::GlobalTransform>();
		variableCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		variableCache->addType<components::BubbleLockedComponent>(components::NOTINTERESTED);
		nonManipulatableEqualsCache = middle::newCompCache(gameState, systemName);
		nonManipulatableEqualsCache->addType<components::BubbleEqualsComponent>();
		nonManipulatableEqualsCache->addType<components::InViewTag>();
		nonManipulatableEqualsCache->addType<components::Layer>();
		nonManipulatableEqualsCache->addType<components::Rectangle>();
		nonManipulatableEqualsCache->addType<components::GlobalTransform>();
		nonManipulatableEqualsCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		nonManipulatableEqualsCache->addType<components::BubbleManipulatable>(components::NOTINTERESTED);
		nonManipulatableEqualsCache->addType<components::BubbleLockedComponent>(components::NOTINTERESTED);
		equalsCache = middle::newCompCache(gameState, systemName);
		equalsCache->addType<components::BubbleEqualsComponent>();
		equalsCache->addType<components::InViewTag>();
		equalsCache->addType<components::Layer>();
		equalsCache->addType<components::Rectangle>();
		equalsCache->addType<components::GlobalTransform>();
		equalsCache->addType<components::BubbleManipulatable>();
		equalsCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		equalsCache->addType<components::BubbleLockedComponent>(components::NOTINTERESTED);

		inequCache = middle::newCompCache(gameState, systemName);
		inequCache->addType<components::BubbleInequaltyComponent>();
		inequCache->addType<components::InViewTag>();
		inequCache->addType<components::Layer>();
		inequCache->addType<components::Rectangle>();
		inequCache->addType<components::GlobalTransform>();
		inequCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		inequCache->addType<components::BubbleLockedComponent>(components::NOTINTERESTED);
		activeBubbleCache = middle::newCompCache(gameState, systemName);
		activeBubbleCache->addType<components::ActiveSceneSelectableTag>();
		activeBubbleCache->addType<components::InViewTag>();
		activeBubbleCache->addType<components::GlobalTransform>();
		activeBubbleCache->addType<components::GlobalRect>();
		activeBubbleCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		activeBubbleCache->addType<components::BubbleLockedComponent>(components::NOTINTERESTED);
		powerCache = middle::newCompCache(gameState, systemName);
		powerCache->addType<components::BubblePowerComponent>();
		powerCache->addType<components::InViewTag>();
		powerCache->addType<components::Rectangle>();
		powerCache->addType<components::LoopSociety>();
		powerCache->addType<components::GlobalTransform>();
		powerCache->addType<components::Layer>();
		powerCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		powerCache->addType<components::BubbleLockedComponent>(components::NOTINTERESTED);
		functionCache = middle::newCompCache(gameState, systemName);
		functionCache->addType<components::BubbleFunctionComponent>();
		functionCache->addType<components::InViewTag>();
		functionCache->addType<components::GlobalTransform>();
		functionCache->addType<components::Layer>();
		functionCache->addType<components::Rectangle>();
		functionCache->addType<components::GlobalRect>();
		functionCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		functionCache->addType<components::BubbleLockedComponent>(components::NOTINTERESTED);
		summationCache = middle::newCompCache(gameState, systemName);
		summationCache->addType<components::BubbleSummationComponent>();
		summationCache->addType<components::InViewTag>();
		summationCache->addType<components::Layer>();
		summationCache->addType<components::Rectangle>();
		summationCache->addType<components::GlobalTransform>();
		summationCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		summationCache->addType<components::BubbleLockedComponent>(components::NOTINTERESTED);
		textCache = middle::newCompCache(gameState, systemName);
		textCache->addType<components::BubbleTextComponent>();
		textCache->addType<components::InViewTag>();
		textCache->addType<components::GlobalTransform>();
		textCache->addType<components::Rectangle>();
		textCache->addType<components::Layer>();
		textCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		textCache->addType<components::BubbleLockedComponent>(components::NOTINTERESTED);
		logicCache = middle::newCompCache(gameState, systemName);
		logicCache->addType<components::BubbleLogicComponent>();
		logicCache->addType<components::InViewTag>();
		logicCache->addType<components::GlobalTransform>();
		logicCache->addType<components::Rectangle>();
		logicCache->addType<components::Layer>();
		logicCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		logicCache->addType<components::BubbleLockedComponent>(components::NOTINTERESTED);
		gateCache = middle::newCompCache(gameState, systemName);
		gateCache->addType<components::BubbleGateComponent>();
		gateCache->addType<components::InViewTag>();
		gateCache->addType<components::GlobalTransform>();
		gateCache->addType<components::Rectangle>();
		gateCache->addType<components::Layer>();
		gateCache->addType<components::RuntimeHiddenTag>(components::NOTINTERESTED);
		gateCache->addType<components::BubbleLockedComponent>(components::NOTINTERESTED);
	}
	bool debugRendering = false;


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

	enum class IconPos {
		CENTER,
		TOP
	};

	int getLayer(middle::GameState* gameState, components::Layer* layer) const{
		return layer->layer - gameState->bubbleAlgebraState.traversePath.size();
	}

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
			text.transform.translation = transform->pos + midMath::Vector3{ -axis + offset,0, axis - offset };
		else if (pos == LabelPos::CENTER)
			text.transform.translation = transform->pos + midMath::Vector3{ 0,0, axis - offset };
		else if (pos == LabelPos::RIGHT)
			text.transform.translation = transform->pos + midMath::Vector3{ axis + offset, axis - offset };
		text.transform.scale = transform->scale;
		text.transform.rotation = transform->rotation;
		middle::queueForRender(gameState, text);
	}

	void renderBubbleIcon(middle::GameState* gameState, components::GlobalTransform* transform, float height, 
		const std::string& textureName, int layer, IconPos pos) {

		middle::RenderItem icon;
		icon.type = middle::RenderItemType::BILLBOARD;
		icon.shader = &gameState->shaderMap[bubbleShaderNames::BUBBLE_SHADER].shader;
		icon.texture = &gameState->textureMap[textureName].texture;
		icon.layer = layer;
		setTransform(icon, transform);
		icon.transform.scale.x *= scaleCorrection;
		icon.transform.scale.y *= scaleCorrection;
		icon.transform.scale.z *= scaleCorrection;

		const float offsetFactor = 0.1f;
		float offset = height * offsetFactor * transform->scale.z;
		float axis = height * 0.5f * transform->scale.z;


		icon.transform.scale = transform->scale;
		if (pos == IconPos::TOP)
			icon.transform.translation = transform->pos + midMath::Vector3{ 0,0, axis - offset };
		else if (pos == IconPos::CENTER) {
			icon.transform.translation = transform->pos + midMath::Vector3{ 0,0,0 };
			const float centerScaleMultiplier = 4;
			icon.transform.scale = Vector3Scale(transform->scale, centerScaleMultiplier);
		}
		icon.transform.rotation = transform->rotation;
		middle::queueForRender(gameState, icon);
	}


	Color calculateFadedColor(middle::GameState* gameState, const Color& color, components::GlobalTransform* transform, int layer) {
		const Color background = bubbleColors::BACKGROUND;

		const float oneChildScaleRatio = 0.758;
		const float stepScale = 1.0f / oneChildScaleRatio;
		float layerOffset = 0;

		float camDist = gameState->middleState.activeCamera.position.y;
		// todo... is cosntant
		float axisY = gameState->middleState.nearPlaneAxisY / gameState->middleState.nearPlaneDistance * -camDist;

		const float firstStepScale = axisY / bubble::bubbleAxis;

		float maxScale = firstStepScale * 0.5f;
		float minScale = 0.0001f;

		float scaleRatio = transform->scale.x / maxScale;
		if (scaleRatio > 1) {
			scaleRatio = 1;
		}

		float s = scaleRatio;

		s = std::powf(s, 0.20f);

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
		middle::queueForRender(gameState, texture);
	}


	void update(middle::GameState* gameState) override {

		gameState->editorState.backgroundColor = bubbleColors::BACKGROUND;

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

			Color backgroundColor = calculateFadedColor(gameState, bubbleColors::BUBBLE, transform, getLayer(gameState, layer));

			//middle::RenderItem debugRect;
			//debugRect.type = middle::RenderItemType::RECTANGLE;
			//setTransform(debugRect, transform);
			//debugRect.color = BLUE;
			//debugRect.layer = getLayer(gameState, layer);
			//debugRect.width = bubble::bubbleAxis * 2;
			//debugRect.height = bubble::bubbleAxis * 2;
			//debugRect.length = 0;
			//gameState->renderData.push_back(debugRect);

			renderBubble(gameState, getLayer(gameState, layer), backgroundColor, transform);

			renderBubbleLabel(gameState, transform, rect->height, "+",
				getLayer(gameState, layer), LabelPos::CENTER, bubbleColors::MULTIPLICATION_TEXT);
		}

		auto logicLayerIt = logicCache->begin<components::Layer>();
		auto logicTransformIt = logicCache->begin<components::GlobalTransform>();
		auto logicRectIt = logicCache->begin<components::Rectangle>();
		for (middle::Id id : logicCache->relevantIdVector) {
			auto layer = *logicLayerIt;
			auto transform = *logicTransformIt;
			auto rect = *logicRectIt;
			Color color = calculateFadedColor(gameState, bubbleColors::LOGIC, transform, getLayer(gameState, layer));
			renderBubble(gameState, getLayer(gameState, layer), color, transform);
			renderBubbleIcon(gameState, transform, rect->height, bubbleTextureNames::TEXTURE_AND_GATE, getLayer(gameState, layer) + 1, IconPos::TOP);
		}

		auto gateLayerIt = gateCache->begin<components::Layer>();
		auto gateTransformIt = gateCache->begin<components::GlobalTransform>();
		auto gateRectIt = gateCache->begin<components::Rectangle>();
		auto gateIt = gateCache->begin<components::BubbleGateComponent>();
		for (middle::Id id : gateCache->relevantIdVector) {
			auto layer = *gateLayerIt;
			auto transform = *gateTransformIt;
			auto rect = *gateRectIt;
			auto gate = *gateIt;
			Color color;
			if (gate->status == components::BubbleGateStatus::CLOSED)
				color = bubbleColors::CLOSED_GATE;
			else if (gate->status == components::BubbleGateStatus::DUMMY)
				color = bubbleColors::DUMMY_GATE;
			else
				color = bubbleColors::OPEN_GATE;
			Color fadedColor = calculateFadedColor(gameState, color, transform, layer->layer);
			renderBubble(gameState, getLayer(gameState, layer), fadedColor, transform);

			if (gate->status != components::BubbleGateStatus::OPEN) {
				renderBubbleIcon(gameState, transform, rect->height, bubbleTextureNames::TEXTURE_CLOSED_GATE, getLayer(gameState, layer) + 1, IconPos::CENTER);
			}
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
				unitItem.text = "1";
				textColor = bubbleColors::UNIT_TEXT_POSITIVE;
				backgroundColor = calculateFadedColor(gameState, bubbleColors::POSITIVE_UNIT, transform, getLayer(gameState, layer));
			}
			else {
				unitItem.text = "-1";
				textColor = bubbleColors::UNIT_TEXT_NEGATIVE;
				backgroundColor = calculateFadedColor(gameState, bubbleColors::NEGATIVE_UNIT, transform, getLayer(gameState, layer));
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
			middle::queueForRender(gameState, unitItem);

			renderBubble(gameState, getLayer(gameState, layer), backgroundColor, transform);
		}

		auto textIt = textCache->begin<components::BubbleTextComponent>();
		auto textTransformIt = textCache->begin<components::GlobalTransform>();
		auto textLayerIt = textCache->begin<components::Layer>();
		for (middle::Id id : textCache->relevantIdVector){
			auto text = *textIt;
			auto layer = *textLayerIt;
			auto transform = *textTransformIt;

			middle::RenderItem textItem;
			textItem.type = middle::RenderItemType::TEXT;
			textItem.layer = getLayer(gameState, layer);
			setTransform(textItem, transform);
			textItem.text = text->text;
			textItem.fontSize = text->fontSize;
			textItem.color = bubbleColors::UNIT_TEXT_POSITIVE;
			middle::queueForRender(gameState, textItem);

			Color color = calculateFadedColor(gameState, bubbleColors::BUBBLE, transform, getLayer(gameState, layer));
			renderBubble(gameState, getLayer(gameState, layer), color, transform);
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
				colorBackground = calculateFadedColor(gameState, bubbleColors::POSITIVE_UNIT, transform, getLayer(gameState, layer));
			}
			else {
				varText = "-";
				colorText = bubbleColors::UNIT_TEXT_NEGATIVE;
				colorBackground = calculateFadedColor(gameState, bubbleColors::NEGATIVE_UNIT, transform, getLayer(gameState, layer));
			}
			varText += variable->label;

			renderBubble(gameState, getLayer(gameState, layer), colorBackground, transform);

			middle::RenderItem variableText;
			variableText.type = middle::RenderItemType::TEXT;
			setTransform(variableText, transform);
			variableText.text = varText;
			variableText.color = colorText;
			variableText.textOffset.x = -rect->width * 0.42f;
			variableText.textOffset.z = rect->height * 1.5f;
			variableText.fontSize = bubble::bubbleFontSize;
			middle::queueForRender(gameState, variableText);

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

			renderBubble(gameState, getLayer(gameState, layer), calculateFadedColor(gameState, bubbleColors::MULTIPLICATION, transform, getLayer(gameState, layer)), transform);

			renderBubbleLabel(gameState, transform, rect->height, u8"\u00D7",
				getLayer(gameState, layer), LabelPos::CENTER, bubbleColors::MULTIPLICATION_TEXT);
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

			renderBubble(gameState, getLayer(gameState, layer), calculateFadedColor(gameState, bubbleColors::POWER, transform, getLayer(gameState, layer)), transform);

			renderBubbleLabel(gameState, transform, rect->height, "^",
				getLayer(gameState, layer), LabelPos::CENTER, bubbleColors::POWER_TEXT);
		}

		auto equTransformIt = equalsCache->begin<components::GlobalTransform>();
		auto equCircleIt = equalsCache->begin<components::Rectangle>();
		auto equLayerIt = equalsCache->begin<components::Layer>();
		for (middle::Id id : equalsCache->relevantIdVector) {
			auto transform = *equTransformIt;
			auto rect = *equCircleIt;;
			auto layer = *equLayerIt;
			renderBubble(gameState, getLayer(gameState, layer), calculateFadedColor(gameState, bubbleColors::EQUALS, transform, getLayer(gameState, layer)), transform);
			renderBubbleLabel(gameState, transform, rect->height, "=",
				getLayer(gameState, layer), LabelPos::CENTER, bubbleColors::EQUALS_TEXT);
		}

		// dimmer equals that cant be modified...
		auto nonEquTransformIt = nonManipulatableEqualsCache->begin<components::GlobalTransform>();
		auto nonEquCircleIt = nonManipulatableEqualsCache->begin<components::Rectangle>();
		auto nonEquLayerIt = nonManipulatableEqualsCache->begin<components::Layer>();
		for (middle::Id id : nonManipulatableEqualsCache->relevantIdVector) {
			auto transform = *nonEquTransformIt;
			auto rect = *nonEquCircleIt;;
			auto layer = *nonEquLayerIt;
			renderBubble(gameState, getLayer(gameState, layer), calculateFadedColor(gameState, bubbleColors::DUMMY_GATE, transform, getLayer(gameState, layer)), transform);
			renderBubbleLabel(gameState, transform, rect->height, "=",
				getLayer(gameState, layer), LabelPos::CENTER, bubbleColors::EQUALS_TEXT);
		}

		auto inequTransformIt = inequCache->begin<components::GlobalTransform>();
		auto inequRectIt = inequCache->begin<components::Rectangle>();
		auto inequLayerIt = inequCache->begin<components::Layer>();
		for (middle::Id id : inequCache->relevantIdVector) {
			auto transform = *inequTransformIt;
			auto rect = *inequRectIt;
			auto layer = *inequLayerIt;

			renderBubbleLabel(gameState, transform, rect->height, ">",
				getLayer(gameState, layer), LabelPos::CENTER, bubbleColors::EQUALS_TEXT);
			renderBubble(gameState, getLayer(gameState, layer), calculateFadedColor(gameState, bubbleColors::INEQUALS, transform, getLayer(gameState, layer)), transform);
		}

		// cross hair or something
		middle::RenderItem cameraTarget;
		cameraTarget.type = middle::RenderItemType::CIRCLE;
		cameraTarget.radius = 1;
		cameraTarget.center = gameState->middleState.activeCamera.position + midMath::Vector3{ 0,100,0 };
		cameraTarget.color = bubbleColors::WHITE;
		middle::queueForRender(gameState, cameraTarget);

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
			boundingRect.color = bubbleColors::WHITE;
			middle::queueForRender(gameState, boundingRect);
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
				getLayer(gameState, layer), LabelPos::CENTER, bubbleColors::FUNCTION_TEXT);

			renderBubble(gameState, getLayer(gameState, layer), calculateFadedColor(gameState, bubbleColors::FUNCTION, transform, getLayer(gameState, layer)), transform);

			// render indexes
			std::vector<middle::Id>children;
			middle::getChildren(gameState, id, children);
			int index = 1;
			for (middle::Id childId : children) {
				auto transform = middle::getComp<components::GlobalTransform>(gameState, childId);
				auto globalRChild = middle::getComp<components::GlobalRect>(gameState, childId);
				auto layer = middle::getComp<components::Layer>(gameState, childId);
				renderBubbleLabel(gameState, transform, rect->width, std::to_string(index++),
					getLayer(gameState, layer), LabelPos::LEFT, bubbleColors::FUNCTION_TEXT);
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

			renderBubble(gameState, getLayer(gameState, layer), calculateFadedColor(gameState, bubbleColors::SUMMATION, transform, getLayer(gameState, layer)), transform);

			renderBubbleLabel(gameState, transform, rect->width, u8"\u2211",
				getLayer(gameState, layer), LabelPos::CENTER, bubbleColors::SUMMATION_TEXT);
		}

	}

};

static middle::SystemRegistrar<BubbleRenderSetup> reg("BubbleRenderSetup");
