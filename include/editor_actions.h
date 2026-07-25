#pragma once
#include "game_state.h"
#include "functional"

namespace middle {
	template<class T, class... Args>
	T* executeAction(GameState* gameState, EditorActionContainer* container, Args&&... args) {
		auto action = std::make_unique<T>(std::forward<Args>(args)...);
		action->execute(gameState);
		auto retVal = action.get();
		container->actions.push_back(std::move(action));
		return retVal;
	}

	// creation of new spheres are here
	class EditorActionNewSphere : public EditorActionContainer {
	public:
		Vector3 position;
		int newIndex = UNASSIGNED;
		EditorActionNewSphere(const Vector3& position) {
			this->position = position;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	// creation of new constraints happened here
	class EditorActionNewConstraint : public EditorActionContainer {
	public:
		int indexA;
		int indexB;
		int newIndex = UNASSIGNED;
		EditorActionNewConstraint(int indexA, int indexB) {
			this->indexA = indexA;
			this->indexB = indexB;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};


	// time to save positions, please don't forget to do it
	class EditorActionSaveScene : public EditorActionContainer {
	public:
		std::string sceneName;
		EditorActionSaveScene(std::string sceneName) {
			this->sceneName = sceneName;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionRegisterShape : public EditorActionContainer {
	public:
		middle::Shape shapeToRegister;
		middle::Id newShapeId;
		EditorActionRegisterShape(middle::Shape& shape) {
			shapeToRegister = shape;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionRegisterId : public EditorActionContainer {
	public:
		middle::Id id;
		EditorActionRegisterId(middle::Id& id) {
			this->id = id;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};


	// build from the editor.  hopefully no crashes...
	class EditorActionBuild : public EditorActionContainer {
	public:
		EditorActionBuild() {

		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	// load scene new exiting start
	class EditorActionLoadScene : public EditorActionContainer {
	public:
		std::string sceneName;
		EditorActionLoadScene(const std::string& sceneName) {
			this->sceneName = sceneName;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	// create loops, 
	class EditorActionCreateLoop : public EditorActionContainer {
	public:
		std::vector<int>memberIndexes;
		std::vector<middle::Id>oldParents;
		int newIndex = UNASSIGNED;
		EditorActionCreateLoop(const std::vector<int>& selectedIndexes) {
			memberIndexes = selectedIndexes;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	// new scene, new world
	class EditorActionNewScene : public EditorActionContainer {
	public:
		std::string sceneName;
		EditorActionNewScene(const std::string& sceneName) {
			this->sceneName = sceneName;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	// what is import scene? it's so that you can import scenes as objects, or loops 
	class EditorActionImportScene : public EditorActionContainer {
	public:
		std::string path;
		std::string name;
		int newIndex = UNASSIGNED;
		EditorActionImportScene(const std::string& path, const std::string& name) {
			this->path = path;
			this->name = name;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	// it is so much work to find scripts. Finding things from list of files is not smart. You need to use the editor
	class EditorActionOpenSystem : public EditorActionContainer {
	public:
		std::string systemName;
		EditorActionOpenSystem(const std::string& systemName) {
			this->systemName = systemName;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	// create new script
	class EditorActionNewSystem : public EditorActionContainer {
	public:
		std::string systemName;
		EditorActionNewSystem(const std::string& systemName) {
			this->systemName = systemName;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	// import script, the code already exists
	class EditorActionImportSystem : public EditorActionContainer {
	public:
		std::string systemName;
		int newIndex = UNASSIGNED;
		EditorActionImportSystem(const std::string& systemName) {
			this->systemName = systemName;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	// new perspective 
	class EditorActionNewCamera : public EditorActionContainer {
	public:
		Vector3 position;
		Vector3 up;
		Vector3 target;
		float fieldOfView;
		// projection 0 or 1
		int projection;
		EditorActionNewCamera(const Vector3& position, const Vector3& targetPos, const Vector3& up, float fovy, int projection) {
			this->position = position;
			this->up = up;
			this->target = targetPos;
			this->fieldOfView = fovy;
			this->projection = projection;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	// we are directors now
	class EditorActionSelectCamera : public EditorActionContainer {
	public:
		int cameraIndex;
		EditorActionSelectCamera(int cameraIndex) {
			this->cameraIndex = cameraIndex;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionNewComponent : public EditorActionContainer {
	public:
		std::string componentName;
		EditorActionNewComponent(const std::string& componentName) {
			this->componentName = componentName;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionImportComponent : public EditorActionContainer {
	public:
		std::string componentName;
		std::vector<int>selectedIndexes;
		EditorActionImportComponent(const std::string& componentName, std::vector<int>selectedIndexes) {
			this->componentName = componentName;
			this->selectedIndexes = selectedIndexes;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionRemoveComponent : public EditorActionContainer {
	public:
		std::string componentName;
		std::vector<int>selectedIndexes;
		EditorActionRemoveComponent(const std::string& componentName, std::vector<int>selectedIndexes) {
			this->componentName = componentName;
			this->selectedIndexes = selectedIndexes;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionOpenComponent : public EditorActionContainer {
	public:
		std::string componentName;
		EditorActionOpenComponent(const std::string& componentName) {
			this->componentName = componentName;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionRemoveFromLoop : public EditorActionContainer {
	public:
		int childIndex = middle::UNASSIGNED;
		int oldParentIndex = middle::UNASSIGNED;
		int loopIndex = middle::UNASSIGNED;
		EditorActionRemoveFromLoop(int childIndex) {
			this->childIndex = childIndex;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionReparent : public EditorActionContainer {
	public:
		int parentIndex;
		int childIndex;
		int oldParentIndex;
		EditorActionReparent(int parentIndex, int childIndex) {
			this->parentIndex = parentIndex;
			this->childIndex = childIndex;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionChangeLoopMemberIndex : public EditorActionContainer {
	public:
		int parentIndex;
		int childIndex;
		int newLoopIndex;
		int oldLoopIndex;
		EditorActionChangeLoopMemberIndex(int parentIndex, int childIndex, int newLoopIndex) {
			this->parentIndex = parentIndex;
			this->childIndex = childIndex;
			this->newLoopIndex = newLoopIndex;
			assert(parentIndex >= 0);
			assert(childIndex >= 0);
			assert(newLoopIndex >= 0);
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionCopy : public EditorActionContainer {
	public:
		std::vector<int> selectedShapes;
		std::vector<int> newCopyShapes;
		EditorActionCopy(std::vector<int>& selectedShapes) {
			this->selectedShapes = selectedShapes;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionCopySingle : public EditorActionContainer {
	public:
		middle::Id id;
		middle::Id resultId;
		EditorActionCopySingle(middle::Id id) {
			this->id = id;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionHide : public EditorActionContainer {
	public:
		std::vector<int> selectedShapes;
		EditorActionHide(std::vector<int>& selectedShapes) {
			this->selectedShapes = selectedShapes;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionUnhide : public EditorActionContainer {
	public:
		std::vector<int> unhidIndexes;
		EditorActionUnhide() {
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionMove : public EditorActionContainer {
	public:
		std::vector<int> selectedShapes;
		std::vector<Vector3>oldPositions;
		std::vector<Vector3>newPositions;
		EditorActionMove(std::vector<int>& selectedShapes) {
			this->selectedShapes = selectedShapes;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class EditorActionDeleteSingle : public EditorActionContainer {
	public:
		middle::Id id;
		int loopIndex = middle::UNASSIGNED;
		std::unique_ptr<EditorActionRemoveFromLoop>removeFromLoop;
		EditorActionDeleteSingle(middle::Id id) {
			this->id = id;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	// deletion of selected things happening here actually
	class EditorActionDelete : public EditorActionContainer {
	public:
		std::vector<int>selectedIndexes;
		EditorActionDelete(std::vector<int> selectedIndexes) {
			this->selectedIndexes = selectedIndexes;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class CustomAction : public EditorActionContainer {
	public:
		std::function<void(middle::GameState*)> func;
		CustomAction(std::function<void(middle::GameState*)> func) {
			this->func = func;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class CustomActionWithUndo : public EditorActionContainer {
	public:
		std::function<void(middle::GameState*)> func;
		std::function<void(middle::GameState*)> undoFunc;
		CustomActionWithUndo(std::function<void(middle::GameState*)> func, 
			std::function<void(middle::GameState*)> undoFunc) {
			this->func = func;
			this->undoFunc = undoFunc;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};

	class MultiAction : public EditorActionContainer {
	public:
		std::vector<std::shared_ptr<EditorActionContainer>>actionList;

		MultiAction(std::vector<std::shared_ptr<EditorActionContainer>>& actionList) {
			this->actionList = actionList;
		}
		void execute(GameState* gameState) override;
		void undo(GameState* gameState) override;
	};
}
