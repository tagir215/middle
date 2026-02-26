#pragma once
#include "raylib.h"
#include "input.h"
#include <array>
#include <list>
#include "descart_physics.h"
#include <raymath.h>
#include "middle_gameplay_script.h"
#include "middle_gameplay_script_map.h"
#include "entity.h"
#include <string>
#include <memory>
#include <functional>

using namespace descart;

namespace middle {

	struct GameState;

	enum class CreationMode {
		SELECT_MODE,
		SPHERE_MODE,
		CONSTRAINT_MODE,
		CAMERA_MODE,
		LOOP_MODE,
	};

	enum class ApplicationMode {
		EDITOR_MODE,
		GAME_MODE,
	};


	struct CollisionData {
		Vector3 normal;
		Vector3 collisionPoint;
		int indexA;
		int indexB;
		float toi;
		bool swept;
		bool collided;
	};

	class EditorActionContainer {
	public:
		virtual ~EditorActionContainer() = default;
		virtual void execute(GameState* gameState) = 0;
		virtual void undo(GameState* gameState) = 0;
	};


	struct EditorState {
		CreationMode creationMode;
		Camera3D camera;
		bool initialized = false;
		bool doOneStep = false;
		bool showAllInfo = false;
		int stepDir = 1;
		int gridSize = 4;
		int intersectCount = 0;
		int selectCount = 0;
		int selectChangeCountAfterClick = 0;
		Color backgroundColor = BACKGROUND_COLOR;
		std::vector<std::unique_ptr<EditorActionContainer>> editorActions;
		int previousSize = 0;
		int actionPointer = 0;
		bool grabbing = false;
	};

	enum RenderItemType {
		SPHERE,
		LINE,
		RECTANGLE,
		CIRCLE,
		TEXT,
	};

	struct RenderItem {
		RenderItemType type;
		Color color;
		Vector3 center;
		Vector3 linePointA;
		Vector3 linePointB;
		Transform transform;
		float radius;
		float length;
		float widht;
		float height;
		int fontSize;
		std::string text = "";
	};

	struct BubbleAlgebraState {
		middle::Id grabbedId;
		bool intersectingUI = false;
		middle::Id idReplacementNews;
	};


	struct GameState {
	public:
		float screenWidth;
		float screenHeight;
		float aspectRatio;
		float frameTime;
		float frameTimeAccumulator = 0;
		const double nearPlaneDistance = 0.05;
		const double farPlaneDistance = 5000;
		bool systemsRegistered = false;
		ApplicationMode applicationMode = ApplicationMode::EDITOR_MODE;
		EditorState editorState;
		Camera activeCamera;
		// shapes
		std::array<Id, MAX_SHAPE_COUNT>ids;
		std::array<Shape, MAX_SHAPE_COUNT>shapes;
		// systems
		std::unordered_map<std::string, std::unique_ptr<MiddleGameplaySystem>> gameplaySystems;
		std::vector<std::unique_ptr<MiddleGameplaySystem>> engineSystemsFrameStart;
		std::vector<std::unique_ptr<MiddleGameplaySystem>> engineSystemsFrameEnd;
		std::vector<std::unique_ptr<MiddleGameplaySystem>> engineRendererSystems;

		std::array<Vector3, MAX_VERTEX_COUNT> vertexArray;
		Matrix worldM;
		Vector2 mouseDragPos;
		Matrix oldWorldM;
		Matrix screenOrientorM;
		int activeScene = 0;
		int activeCameraIndex = UNASSIGNED;
		int vertexIndex = 0;
		int loopIndex = 0;
		int uniqueComponentCount = 0;
		std::vector<std::string>sceneNames;
		std::vector<std::string>shapeNames;
		std::vector<std::string>systemNames;
		std::vector<std::string>componentNames;
		EditorInput input;
		GameInput gameInput;
		std::set<InputBlockers> inputBlockers;
		bool paused = false;
		bool closeGame = false;
		bool startGame = false;
		bool reload = true;
		bool reset = false;
		bool quit = false;
		std::vector<RenderItem>renderData;
		std::vector<PhysicsBody>physicsBodies;
		std::vector<std::function<void()>>uiSetups;
		std::vector<middle::FieldInfo>fields;
		BubbleAlgebraState bubbleAlgebraState;
	};

}
