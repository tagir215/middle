#pragma once
#include <unordered_map>
#include <string>
#include "middle_primitives.h"
#include <functional>
#include "input.h"

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
		bool paused = false;
		bool closeGame = false;
		bool startGame = false;
		bool reload = true;
		bool reset = false;
		bool loaded = false;
		bool quit = false;
		bool releaseBuild = false;

		float aspectRatio;
		float screenWidth;
		float screenHeight;
		float frameTime;
		float frameTimeAccumulator = 0;
		float nearPlaneAxisX = 0;
		float nearPlaneAxisY = 0;
		const double nearPlaneDistance = 10;
		const double farPlaneDistance = 4000;

		std::vector<middle::RenderItem> renderData;
		std::vector<std::function<void()>>uiSetups;
		midPrimitive::Color backgroundColor = { 188, 144, 181, 255 };
		midPrimitive::Camera activeCamera;
		ApplicationMode applicationMode;

		EditorInput input;
		GameInput gameInput;
		EqulabInput equlabInput;
		std::set<InputBlockers> inputBlockers;
	};

}
