#include "editor_actions.h"
#include <thread>
#include "middle_shape_utils.h"
#include "editor_file_utils.h"
#include "middle_math.h"
#include <unordered_map>
#include <set>
#include "middle_component_table.h"
#include "script_opener.h"
#include "LoopSociety.h"
#include "Reference.h"
#include "Position.h"
#include "MouseSelectable.h"
#include "JointEntity.h"
#include "ConstraintEntity.h"
#include "LoopEntity.h"
#include "LoopTag.h"
#include "SystemEntity.h"
#include "ComponentReference.h"
#include "ComponentRefParent.h"

namespace middle {
	void processEditorActions(GameState* gameState) {
		switch (gameState->editorState.nextEditorAction) {
		case EditorAction::NEW_SPHERE: {
			auto action = EditorActionNewSphere();
			action.params = gameState->editorState.nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::NEW_CONSTRAINT: {

			auto action = EditorActionNewConstraint();
			action.params = gameState->editorState.nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::BUILD: {

			auto action = EditorActionBuild();
			action.execute(gameState);
			break;
		}
		case EditorAction::SAVE_SCENE: {

			auto action = EditorActionSaveScene();
			action.execute(gameState);
			break;
		}
		case EditorAction::DELETE_SHAPES: {

			auto action = EditorActionDelete();
			action.execute(gameState);
			break;
		}
		case EditorAction::CREATE_LOOPS: {

			auto action = EditorActionCreateLoop();
			action.execute(gameState);
			break;
		}
		case EditorAction::LOAD_SCENE: {
			auto action = EditorActionLoadScene();
			action.params = gameState->editorState.nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::NEW_SCENE: {
			auto action = EditorActionNewScene();
			action.params = gameState->editorState.nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::IMPORT_SCENE: {
			auto action = EditorActionImportScene();
			action.params = gameState->editorState.nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::IMPORT_SYSTEM: {
			auto action = EditorActionImportSystem();
			action.params = gameState->editorState.nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::IMPORT_COMPONENT: {
			auto action = EditorActionImportComponent();
			action.params = gameState->editorState.nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::NEW_SYSTEM: {
			auto action = EditorActionNewSystem();
			action.params = gameState->editorState.nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::NEW_COMPONENT: {
			auto action = EditorActionNewComponent();
			action.params = gameState->editorState.nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::OPEN_SYSTEM: {
			auto action = EditorActionOpenSystem();
			action.params = gameState->editorState.nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::OPEN_COMPONENT: {
			auto action = EditorActionOpenComponent();
			action.params = gameState->editorState.nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::NEW_CAMERA: {
			auto action = EditorActionNewCamera();
			action.params = gameState->editorState.nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::SET_ACTIVE_CAMERA: {
			auto action = EditorActionSelectCamera();
			action.execute(gameState);
			break;
		}
		}
		gameState->editorState.nextEditorAction = EditorAction::NONE;
		gameState->editorState.nextEditorActionParams = {};
	}

	void EditorActionNewSphere::execute(GameState* gameState) {
		auto& shapes = gameState->shapes;

		int freeIndex = findFreeIndex(gameState);

		Vector3 xzPos = gameState->input.mouseXZ_PlanePos;
		entities::initJoint(gameState, freeIndex, xzPos);
	};

	void EditorActionNewConstraint::execute(GameState* gameState) {
		auto& shapes = gameState->shapes;

		std::vector<int> selectedIndexes;
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!getComponent<components::Sphere>(shapes[i]))
				continue;
			if (isShapeAlive(gameState, i) && isShapeSelected(gameState, i))
				selectedIndexes.push_back(i);
		}
		// must have 2 constraints selected 
		if (selectedIndexes.size() != 2)
			return;

		int indexA = selectedIndexes[0];
		int indexB = selectedIndexes[1];

		if (constraintAlreadyExists(gameState, indexA, indexB)) {
			return;
		}

		int freeIndex = findFreeIndex(gameState);

		if (indexA != indexB) {
			auto& shapeA = shapes[indexA];
			auto& shapeB = shapes[indexB];
			auto posA = getShapePosition(gameState, indexA);
			auto posB = getShapePosition(gameState, indexB);
			float distBetween = Vector3Distance(posA, posB);
			entities::initConstraint(gameState, freeIndex, indexA, indexB, distBetween);

			// auto unselect
			unselect(gameState);

		}

	}

	void EditorActionDelete::execute(GameState* gameState) {
		// loops of deleted spheres that belong to loops
		std::set<int>deteledLoopMembersParentLoops;

		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!isShapeAlive(gameState, i))
				continue;
			if(!isShapeSelected(gameState, i))
				continue;

			Shape& shape = getShape(gameState, i);

			// delete all connected constraints
			std::vector<int> connectedConstraints = findConnectedConstraints(gameState, i);


			if(getComponent<components::Reference>(shape)){
				deleteShapeRecursive(gameState, i);
			}
             
			// set all connected constraints as selected
			for (int id : connectedConstraints) {
				Shape& constraintShape = getShape(gameState, id);
				auto selectable = getComponent<components::MouseSelectable>(constraintShape);
				assert(selectable);
				selectable->selected = true;
			}

			deleteShape(gameState, i);
		}

	}

	void EditorActionSaveScene::execute(GameState* gameState) {
		saveScene(gameState, gameState->sceneNames[gameState->activeScene]);
	}

	void EditorActionBuild::execute(GameState* gameState) {
		std::string command = "python ../src/editor_scripts/build_project.py";
		system(command.c_str());
	}

	void EditorActionCreateLoop::execute(GameState* gameState) {
		int freeIndex = findFreeIndex(gameState);
		int loopIndex = gameState->loopIndex;

		// find selected items 
		std::vector<int>memberIndexes;
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!isShapeAlive(gameState, i))
				continue;
			auto loop = getComponent<components::LoopTag>(gameState->shapes[i]);
			auto sphere = getComponent<components::Sphere>(gameState->shapes[i]);
			if ((sphere || loop) && isShapeSelected(gameState, i)) {
				memberIndexes.push_back(i);
			}
		}

		// loops must have at least 2 things
		if (memberIndexes.size() < 2)
			return;

		// create 
		entities::initLoop(gameState, freeIndex, memberIndexes);

		// auto unselect
		unselect(gameState);
	}


	void EditorActionLoadScene::execute(GameState* gameState)
	{
		assert(params.intValue != UNASSIGNED);
		// DELETE EVERYTHING
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			deleteShape(gameState, i);
		}
		gameState->reload = true;
		gameState->activeScene = params.intValue;
		gameState->loopIndex = 0;
	}

	void EditorActionNewScene::execute(GameState* gameState)
	{
		assert(params.intValue != UNASSIGNED);
		assert(params.stringValue != "");
		for (auto& name : gameState->sceneNames) {
			//todo print error
			if (name == params.stringValue)
				return;
		}
		gameState->activeScene = params.intValue;
		// DELETE EVERYTHING
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			deleteShape(gameState, i);
		}
		// save new scene
		saveScene(gameState, params.stringValue);
		gameState->sceneNames.push_back(params.stringValue);
		gameState->reload = true;
		gameState->reset = true;
	}

	void EditorActionImportScene::execute(GameState* gameState)
	{
		assert(params.intValue != UNASSIGNED);
		std::string sceneName = gameState->sceneNames[params.intValue];
		loadScene(gameState, sceneName, true, {0,0,0}, findFreeIndex(gameState));
	}


	void EditorActionOpenSystem::execute(GameState* gameState)
	{
		std::string systemName = "";
		bool found = false;
		loopInstances(gameState, [gameState, &systemName, &found](int i, Shape& shape) {
			auto system = getComponent<components::SystemReference>(shape);
			if (!system)
				return;

			auto intersectable = getComponent<components::MouseIntersectable>(shape);

			if (intersectable->intersecting) {
				systemName = system->systemName;
				found = true;
			}
			});

		if (found) {
			shell_open_file("../assets/systems/" + systemName + ".cpp");
		}

	}

	void EditorActionNewSystem::execute(GameState* gameState)
	{
		assert(params.stringValue != "");

		std::string systemName = params.stringValue;

		if (gameState->gameplaySystems.find(systemName) == gameState->gameplaySystems.end()) {
			newSystemFile(gameState, systemName);
		}

		shell_open_file("../assets/systems/" + systemName + ".cpp");

		std::string command = "python ../src/editor_scripts/build_project.py";
		system(command.c_str());


		gameState->closeGame = true;
	}

	void EditorActionImportSystem::execute(GameState* gameState) 
	{
		assert(params.stringValue != "");
		std::string systemName = params.stringValue;

		int freeIndex = findFreeIndex(gameState);
		entities::initSystem(gameState, freeIndex, { 0,0,0 }, systemName);
	}

	void EditorActionNewCamera::execute(GameState* gameState)
	{
		int freeIndex = findFreeIndex(gameState);
		Vector3& pos = gameState->editorState.camera.position;
		//initCamera(gameState, freeIndex, pos);
	}

	void EditorActionSelectCamera::execute(GameState* gameState)
	{

	}


	void EditorActionNewComponent::execute(GameState* gameState)
	{
		assert(params.stringValue != "");

		std::string componentName = params.stringValue;

		if (gameState->gameplaySystems.find(componentName) == gameState->gameplaySystems.end()) {
			newComponentFile(gameState, componentName);
		}

		shell_open_file("../assets/components/" + componentName + ".cpp");

		std::string command = "python ../src/editor_scripts/build_project.py";
		system(command.c_str());

		gameState->closeGame = true;
	}

	void EditorActionImportComponent::execute(GameState* gameState)
	{
		assert(params.stringValue != "");
		const std::string componentName = params.stringValue;

		loopInstances(gameState, [&componentName](int i, Shape& shape) {
			auto selectable = getComponent<components::MouseSelectable>(shape);
			if (selectable && selectable->selected) {
				int componentTypeId = componentTypeMap[componentName];
				assert(shape.componentMap.find(componentTypeId) == shape.componentMap.end());

				Component component;
				component.componentOffset = componentListMap[componentTypeId]->grow();
				shape.componentMap[componentTypeId] = component;
			}
			});
	}

	void EditorActionOpenComponent::execute(GameState* gameState)
	{
		std::string componentName = "";
		bool found = false;
		loopInstances(gameState, [gameState, &componentName, &found](int i, Shape& shape) {
			auto componentRef = getComponent<components::ComponentReference>(shape);
			if (!system)
				return;

			auto intersectable = getComponent<components::MouseIntersectable>(shape);

			if (intersectable->intersecting) {
				componentName = componentRef->componentName;
				found = true;
			}
			});

		if (found) {
			shell_open_file("../assets/components/" + componentName + ".h");
		}

		unselect(gameState);
	}

}
