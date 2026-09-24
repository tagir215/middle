#pragma once
#include <unordered_map>
#include <string>
#include "middle_primitives.h"
#include <functional>

namespace middle {


	enum RenderItemType {
		SPHERE,
		LINE,
		RECTANGLE,
		CIRCLE,
		TEXT,
		MODEL,
		VECTOR,
		CIRCLE_SECTOR,
		CONE,
		RING,
		CYLINDER,
		CUBOID,
		BILLBOARD,
		BACKGROUND,
	};

	struct RenderItem {
		RenderItemType type;
		midPrimitive::Color color;
		midPrimitive::Color backgroundColor = { 0,0,0,0 };
		midMath::Vector3 center = { 0,0,0 };
		midMath::Vector3 scale = { 1,1,1 };
		midMath::Vector3 linePointA;
		midMath::Vector3 linePointB;
		midMath::Vector3 textOffset = { 0,0,0 };
		midPrimitive::Transform transform;
		int layer = 0;
		int slices = 20;
		float radius;
		float ringRadius;
		float startAngle;
		float endAngle;
		int segments;
		float length = 0;
		float width = 0;
		float height = 0;
		float textureScale = 10;
		int fontSize = 10;
		bool disableDepthTest = false;
		std::string text = "";
		midPrimitive::Model* model = nullptr;
		midPrimitive::Texture2D* texture = nullptr;
		midPrimitive::Shader* shader = nullptr;

		RenderItem() {
			transform.translation = { 0,0,0 };
			transform.rotation = { 0,0,0 };
			transform.scale = { 1,1,1 };
		}
	};

	enum class ApplicationMode {
		EDITOR_MODE,
		GAME_MODE,
	};

	struct MiddleState {
		float aspectRatio;
		std::vector<middle::RenderItem> renderData;
		std::vector<std::function<void()>>uiSetups;
		midPrimitive::Color backgroundColor = { 188, 144, 181, 255 };
		midPrimitive::Camera activeCamera;
		const double nearPlaneDistance = 10;
		const double farPlaneDistance = 4000;
		ApplicationMode applicationMode;
	};

}
