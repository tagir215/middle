#pragma once
#include "raylib.h"
#include "input.h"
#include <array>
#include <list>
#include "game_colors.h"
#include "physics_body.h"
#include "descart_physics.h"
#include "descart_constraints.h"
#include <raymath.h>
#include <string>
#include <set>
#include "middle_constants.h"
#include <memory>
#include "middle_gameplay_script.h"
#include <unordered_map>
#include "middle_gameplay_script_map.h"

using namespace descart;

namespace middle {
	static float DEF_RADIUS = 2;
	static float DEF_RADIUS_LOOP_INDICATOR = 1;
	static float DEF_RADIUS_REFERENCE_INDICATOR = 3;
	static float DEF_RADIUS_CAMERA = 5;
	static float DEF_RADIUS_SCRIPT = 8;
	static Color DEF_COLOR = UGLY_PINK;
	static float DEF_LIFETIME = INFINITY;
	static float DEF_GRAVITY = 0;
	static float DEF_DAMPING = 0.8f;
	static int DEF_HISTORY_MEMEORY_LENGTH = 10;
	static ConstraintType DEF_CONSTRAINT_TYPE = ConstraintType::distance;
	static float DEF_STIFFNESS = 0.8f;
	static float DEF_MASS = 1;
	static float DEF_INERTIA = 1;
	static float DEF_LINE_PADDING_H = -3.0f;
	static float DEF_LINE_PADDING_V = 2.2f;

	struct GameState;

	enum class ShapeType {
		NONE,
		SPHERE,
		CONSTRAINT,
		LOOP,
		REFERENCE,
		CAMERA,
		SCRIPT,
	};

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

	struct Id {
		int generation = -1;
		bool operator==(const Id& other) {
			return other.generation == generation;
		}

		bool operator!=(const Id& other) {
			return !(*this == other);
		}
	};

	struct Shape {
	public:
		Id id;
		ShapeType type;
		Matrix initTransform;
		Color color;
		std::string name;
		float thickness;
		float radius;
		float maxLifetime;
		float linearDamping;
		float mass;
		float inertia;
		Vector3 linearVelocity;
		Vector3 linearAcceleration;
		Vector3 angularVelocity;
		Vector3 angularAcceleration;
		Vector3 position;
		bool physicalShape;
		bool hasPerspective;
		bool renderMemory;
		bool initialized;
		int historyMemoryLength;
		int loopArrayOffset;
		int loopSize;
		int parentLoopIndex;
		Constraint constraint;

		Shape() {
			// shape defaults
			type = ShapeType::NONE;
			initTransform = MatrixIdentity();
			color = DEF_COLOR;
			maxLifetime = DEF_LIFETIME;
			radius = DEF_RADIUS;
			linearVelocity = { 0,0,0 };
			angularVelocity = { 0,0,0 };
			linearAcceleration = { 0,0,DEF_GRAVITY };
			angularAcceleration = { 0,0,0 };
			position = { 0,0,0 };
			linearDamping = DEF_DAMPING;
			mass = DEF_MASS;
			inertia = DEF_INERTIA;
			physicalShape = true;
			hasPerspective = true;
			historyMemoryLength = DEF_HISTORY_MEMEORY_LENGTH;
			renderMemory = false;
			parentLoopIndex = UNASSIGNED;
			loopArrayOffset = 0;
			loopSize = 0;
			name = "";

			// constraint defaults
			constraint.type = DEF_CONSTRAINT_TYPE;
			constraint.stiffness = DEF_STIFFNESS;
		}

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

	struct ShapeInstance {
		Id id;
		bool isShapeAlive = false;
		Shape shape;
		PhysicsBody pData;
		SweepResult sweepData;
		std::list<PhysicsBody> history;
		float lifeTime = 0;
		bool mouseIntersects = false;
		bool selected = false;
		bool grabDown = false;
		bool grabReleased = false;
		bool infoVisible = false;
		int vertexListOffset;
		int vertexCount = 0;
	};

	enum class EditorAction {
		NONE, 
		NEW_SPHERE,
		NEW_CONSTRAINT,
		NEW_CAMERA,
		NEW_SCRIPT,
		SET_ACTIVE_CAMERA,
		MOVE_SPHERES,
		DELETE_SHAPES,
		SAVE_SCENE,
		BUILD,
		CREATE_LOOPS,
		LOAD_SCENE,
		IMPORT_SCENE,
		IMPORT_SCRIPT,
		NEW_SCENE,
		OPEN_SCRIPT,
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
		Camera3D camera;
		bool initialized = false;
		bool doOneStep = false;
		bool showAllInfo = false;
		int stepDir = 1;
		EditorAction nextEditorAction = EditorAction::NONE;
		EditorActionContainer::Params nextEditorActionParams;
	};

	struct GameState {
	public:
		float screenWidth;
		float screenHeight;
		float aspectRatio;
		float frameTime;
		int gridSize = 4;
		int intersectCount = 0;
		int selectCount = 0;
		const double nearPlaneDistance = 0.05;
		const double farPlaneDistance = 5000;
		ApplicationMode applicationMode = ApplicationMode::EDITOR_MODE;
		EditorState editorState;
		std::array<Shape, MAX_SHAPE_COUNT>shapes;
		std::array<Vector3, MAX_VERTEX_COUNT> vertexArray;
		std::array<int, MAX_LOOP_MEMBER_COUNT> loopMembers;
		std::array<ShapeInstance, MAX_SHAPE_COUNT>shapeInstances;
		std::unordered_map<std::string, std::unique_ptr<MiddleGameplayScript>> gameplayScripts;
		Matrix worldM;
		Vector2 mouseDragPos;
		Matrix oldWorldM;
		Matrix screenOrientorM;
		int activeScene = 0;
		int activeCameraIndex = 0;
		int vertexIndex = 0;
		int loopIndex = 0;
		std::vector<std::string>sceneNames;
		EditorInput input;
		std::set<InputBlockers> inputBlockers;
		bool paused = false;
		bool closeGame = false;
		bool startGame = false;
		bool reload = true;
		bool reset = false;
	};

	template<typename T>
	struct Registrar {
		Registrar(std::string scriptName) {
			scriptMap[scriptName] = std::make_unique<T>();
		}
	};

}
