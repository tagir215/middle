#pragma once
#include "input.h"
#include <array>
#include <list>
#include "middle_gameplay_script.h"
#include "middle_gameplay_script_map.h"
#include "entity.h"
#include <string>
#include <memory>
#include <functional>
#include "comp_cache.h"
#include <queue>
#include <stack>
#include <set>
#include "middle_math.h"
#include "middle_state.h"


namespace middle {

	struct GameState;


	enum class CreationMode {
		SELECT_MODE,
		SPHERE_MODE,
		CONSTRAINT_MODE,
		CAMERA_MODE,
		LOOP_MODE,
	};

	struct CollisionData {
		midMath::Vector3 normal;
		midMath::Vector3 collisionPoint;
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
		std::vector < std::unique_ptr<EditorActionContainer>>actions;
		bool cancelled = false;
		// for debugging only
		std::string callerSystem;
	};


	struct EditorState {
		CreationMode creationMode;
		midPrimitive::Camera3D camera;
		bool initialized = false;
		bool doOneStep = false;
		bool showAllInfo = false;
		int stepDir = 1;
		int intersectCount = 0;
		int selectCount = 0;
		int selectChangeCountAfterClick = 0;
		midPrimitive::Color backgroundColor = BACKGROUND_COLOR;
		int historySinkDepth = 0;
		std::vector<std::shared_ptr<EditorActionContainer>>actionHistory;
		bool grabbing = false;
	};


	enum BubbleInsertType {
		ADD_OUTER,
		MULTIPLY_OUTER,
		POWER_OUTER,
		ADD_X_MINUS_X,
		MULTIPLY_X_OVER_X,
	};

	struct Animation : public std::enable_shared_from_this<Animation> {
		float progress = 0;
		float prevProgress = 0;
		float duration = 0;
		bool reverseMode = false;
		virtual void update(middle::GameState* gameState) = 0;
		virtual void start(middle::GameState* gameState) = 0;
		void setDuration(float duration);
		void progressAnimation(middle::GameState* gameState);
		virtual ~Animation() = default;
	};

	struct BubbleAlgebraState {
		middle::Id grabbedId;
		std::vector<std::shared_ptr<middle::EditorActionContainer>>bubbleActions;
		BubbleInsertType currentInsertType;
		bool copyNegated;
		bool copyInverted;
		int postUndoFrames;
		float worldScale;
		std::string activeBubbleName;
		std::vector<int>traversePath;
		std::vector<middle::Id>traversePathIds;
		middle::Id backgroundBubbleId;
		const int loadDepth;
		midMath::Vector3 cameraVelocity;
		float worldScalarRate;
		BubbleAlgebraState();
	};

	struct ModelContainer {
		std::string path = "";
		midPrimitive::Model model;
	};

	struct TextureContainer {
		midPrimitive::Texture2D texture;
	};

	struct ShaderContainer {
		midPrimitive::Shader shader;
	};

	typedef int shapeIndex;
	typedef int componentType;

	struct GameState {
	public:
		float screenWidth;
		float screenHeight;
		float frameTime;
		float frameTimeAccumulator = 0;
		float nearPlaneAxisX = 0;
		float nearPlaneAxisY = 0;
		bool systemsRegistered = false;
		bool releaseBuild = false;
		MiddleState middleState;
		EditorState editorState;
		// shapes
		std::array<Id, MAX_SHAPE_COUNT>ids;
		std::array<Shape, MAX_SHAPE_COUNT>shapes;
		// systems
		std::unique_ptr<MiddleGameplaySystem>componentCacheSystem;
		std::unordered_map<std::string, std::unique_ptr<MiddleGameplaySystem>> gameplaySystems;
		std::unordered_map<std::string, std::unique_ptr<MiddleGameplaySystem>> gameplaySystemsPostFrame;
		std::vector<std::unique_ptr<MiddleGameplaySystem>> engineSystemInitFrame;
		std::vector<std::unique_ptr<MiddleGameplaySystem>> engineSystemsFrameStart;
		std::vector<std::unique_ptr<MiddleGameplaySystem>> enginePostFrameSystems;
		std::vector<std::unique_ptr<MiddleGameplaySystem>> engineRendererSystems;
		std::vector<middle::Id>newShapeList;

		std::array<midMath::Vector3, MAX_VERTEX_COUNT> vertexArray;
		midMath::Vector2 mouseDragPos;
		midMath::Vector3 mouseIntersectTopPosition;
		std::string activeSceneName = "";
		std::string activeSystemName = "";
		int vertexIndex = 0;
		int loopIndex = 0;
		int uniqueComponentCount = 0;
		std::vector<std::string>sceneNames;
		std::vector<std::string>shapeNames;
		std::vector<std::string>systemNames;
		std::vector<std::string>componentNames;
		std::vector<midPrimitive::Sound>sounds;
		std::vector<std::string>debugInfo;
		EditorInput input;
		// todo move these
		GameInput gameInput;
		EqulabInput equlabInput;
		std::set<InputBlockers> inputBlockers;
		bool paused = false;
		bool closeGame = false;
		bool startGame = false;
		bool reload = true;
		bool reset = false;
		bool loaded = false;
		bool quit = false;

		const char* workingDir;
		std::vector<middle::FieldInfo>fields;
		BubbleAlgebraState bubbleAlgebraState;
		std::vector<std::unique_ptr<components::CompCache>>compCaches;
		std::unordered_map<componentType, std::vector<middle::Id>>structuralChangesMap;
		std::queue<std::shared_ptr<EditorActionContainer>>actionQueue;
		std::queue<std::shared_ptr<EditorActionContainer>>undoQueue;
		std::vector<ModelContainer> loadedModels;
		std::unordered_map<std::string, TextureContainer>textureMap;
		std::unordered_map<std::string, ShaderContainer>shaderMap;
		std::queue<std::string>modelsToLoadQueue;
		std::queue<midPrimitive::Sound>soundQueue;
		std::vector<std::shared_ptr<Animation>>animations;

		std::vector<std::string>slowSystems;
		std::vector<std::string>slowActions;
	};

}
