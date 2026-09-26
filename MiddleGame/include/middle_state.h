#pragma once
#include <unordered_map>
#include <string>
#include "middle_primitives.h"
#include <functional>
#include "input.h"
#include <set>
#include "asset_enums.h"

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
		middleAssets::MODEL model;
		middleAssets::TEXTURE texture;
		middleAssets::SHADER shader;


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

	struct MiddleInputState {
		bool closeGame = false;
		bool releaseBuild = false;
		float aspectRatio;
		float screenWidth;
		float screenHeight;
		float frameTime;
		float frameTimeAccumulator = 0;
		EditorInput editorInput;
		GameInput gameInput;
		EqulabInput equlabInput;
	};

	struct MiddleOutputState {
		std::vector<middle::RenderItem> renderData;
		std::vector<std::function<void()>>uiSetups;
		midPrimitive::Color backgroundColor = { 188, 144, 181, 255 };
		midPrimitive::Camera activeCamera;
		std::set<InputBlockers> inputBlockers;
		float frameTimeAccumulator = 0;
		double nearPlaneDistance = 10;
		double farPlaneDistance = 4000;
		bool closeGame;
	};

}
