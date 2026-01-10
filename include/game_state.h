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

using namespace descart;

namespace middle {

	struct GameState;

	enum class CreationMode {
		SELECT_MODE,
		SPHERE_MODE,
		CONSTRAINT_MODE,
		CAMERA_MODE,
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


	enum class EditorAction {
		NONE, 
		NEW_SPHERE,
		NEW_CONSTRAINT,
		NEW_CAMERA,
		NEW_SCENE,
		NEW_SYSTEM,
		NEW_COMPONENT,
		SET_ACTIVE_CAMERA,
		MOVE_SPHERES,
		DELETE_SHAPES,
		SAVE_SCENE,
		BUILD,
		CREATE_LOOPS,
		LOAD_SCENE,
		IMPORT_SCENE,
		IMPORT_SYSTEM,
		IMPORT_COMPONENT,
		OPEN_SYSTEM,
		OPEN_COMPONENT,
	};

	class EditorActionContainer {
	public:
		// todo: this is probably horrible
		struct Params {
		public:
			std::string stringValue = "";
			int intValue = UNASSIGNED; 
		};
		Params params;

		EditorAction editorAction;
		virtual ~EditorActionContainer() = default;
		virtual void execute(GameState* gameState) = 0;
	};

	struct EditorState {
		CreationMode creationMode;
		Camera3D initCamera;
		bool initialized = false;
		bool doOneStep = false;
		bool showAllInfo = false;
		int stepDir = 1;
		EditorAction nextEditorAction = EditorAction::NONE;
		EditorActionContainer::Params nextEditorActionParams;
	};

	enum RenderItemType {
		SPHERE,
		LINE
	};

	struct RenderItem {
		RenderItemType type;
		Color color;
		Vector3 center;
		Vector3 linePointA;
		Vector3 linePointB;
		float radius;
	};

	struct GameState {
	public:
		float screenWidth;
		float screenHeight;
		float aspectRatio;
		float frameTime;
		float frameTimeAccumulator = 0;
		int gridSize = 4;
		int intersectCount = 0;
		int selectCount = 0;
		const double nearPlaneDistance = 0.05;
		const double farPlaneDistance = 5000;
		ApplicationMode applicationMode = ApplicationMode::EDITOR_MODE;
		EditorState editorState;
		std::array<Id, MAX_SHAPE_COUNT>ids;
		std::array<Shape, MAX_SHAPE_COUNT>shapes;
		std::array<Vector3, MAX_VERTEX_COUNT> vertexArray;
		std::array<int, MAX_LOOP_MEMBER_COUNT> loopMembers;
		std::unordered_map<std::string, std::unique_ptr<MiddleGameplaySystem>> gameplayScripts;
		Matrix worldM;
		Vector2 mouseDragPos;
		Matrix oldWorldM;
		Matrix screenOrientorM;
		int activeScene = 0;
		int activeCameraIndex = 0;
		int vertexIndex = 0;
		int loopIndex = 0;
		int uniqueComponentCount = 0;
		std::vector<std::string>sceneNames;
		std::vector<std::string>scriptNames;
		std::vector<std::string>componentNames;
		EditorInput input;
		std::set<InputBlockers> inputBlockers;
		bool paused = false;
		bool closeGame = false;
		bool startGame = false;
		bool reload = true;
		bool reset = false;
		bool quit = false;
		std::vector<RenderItem>renderData;
	};

}
