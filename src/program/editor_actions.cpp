#include "editor_actions.h"
#include <thread>
#include "middle_shape_utils.h"
#include "editor_file_utils.h"
#include "middle_math.h"
#include <unordered_map>
#include <set>
#include "script_opener.h"
#include "middle_script_registry.h"

namespace middle {
	void processEditorActions(GameState* gameState) {
		switch (gameState->nextEditorAction) {
		case EditorAction::NEW_SPHERE: {
			auto action = EditorActionNewSphere();
			action.params = gameState->nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::NEW_CONSTRAINT: {

			auto action = EditorActionNewConstraint();
			action.params = gameState->nextEditorActionParams;
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
			action.params = gameState->nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::NEW_SCENE: {
			auto action = EditorActionNewScene();
			action.params = gameState->nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		case EditorAction::OPEN_SCRIPT: {
			auto action = EditorActionOpenScript();
			action.params = gameState->nextEditorActionParams;
			action.execute(gameState);
			break;
		}
		}
		gameState->nextEditorAction = EditorAction::NONE;
		gameState->nextEditorActionParams = {};
	}

	void EditorActionNewSphere::execute(GameState* gameState) {
		auto& shapes = gameState->shapes;

		int freeIndex = findFreeIndex(gameState);

		Vector3 xzPos = gameState->input.mouseXZ_PlanePos;
		Shape shape;
		shape.type = ShapeType::SPHERE;
		shape.position = xzPos;

		gameState->addShape(freeIndex, shape);

	};

	void EditorActionNewConstraint::execute(GameState* gameState) {
		auto& shapes = gameState->shapes;

		std::vector<int> selectedIndexes;
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (gameState->shapes[i].type != ShapeType::SPHERE)
				continue;
			if (gameState->isShapeAlive(i) && gameState->getShapeInstance(i).selected)
				selectedIndexes.push_back(i);
		}
		// must have 2 constraints selected 
		if (selectedIndexes.size() != 2)
			return;

		int indexA = selectedIndexes[0];
		int indexB = selectedIndexes[1];
		assert(shapes[indexA].type == ShapeType::SPHERE);
		assert(shapes[indexB].type == ShapeType::SPHERE);
		if (constraintAlreadyExists(gameState, indexA, indexB)) {

			return;
		}

		int freeIndex = findFreeIndex(gameState);

		if (indexA != indexB) {
			shapes[freeIndex].type = ShapeType::CONSTRAINT;
			shapes[freeIndex].maxLifetime = DEF_LIFETIME;
			shapes[freeIndex].color = DEF_COLOR;
			shapes[freeIndex].constraint.indexA = indexA;
			shapes[freeIndex].constraint.indexB = indexB;

			float distBetween = descart::DistV(gameState->getShapeInstance(indexA).pData.position, gameState->getShapeInstance(indexB).pData.position);
			auto& shapeA = shapes[indexA];
			auto& shapeB = shapes[indexB];
			shapes[freeIndex].constraint.targetDistance = distBetween;

			// auto unselect
			unselect(gameState);

		}

	}

	void EditorActionDelete::execute(GameState* gameState) {
		bool foundSelected = false;
		bool editedLoops = false;
		// loops of deleted spheres that belong to loops
		std::set<int>deteledLoopMembersParentLoops;

		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!gameState->isShapeAlive(i))
				continue;
			ShapeInstance& shapeInstance = gameState->getShapeInstance(i);
			if (!shapeInstance.selected)
				continue;
			Shape& shape = gameState->shapes[i];
			// delete all connected constraints
			std::vector<int> connectedConstraints = findConnectedConstraints(gameState, i);

			// remove parent indexes if deleting loops from children
			if (shape.type == ShapeType::LOOP) {
				auto childIndexes = getChildIndexes(gameState, i);
				for (int childIndex : childIndexes) {
					gameState->shapes[childIndex].parentLoopIndex = UNASSIGNED;
				}
			}

			// store parent loops to re generate later
			if (shape.parentLoopIndex != UNASSIGNED) {
				deteledLoopMembersParentLoops.insert(shape.parentLoopIndex);
			}

			foundSelected = true;
			// set all connected constraints as selected
			for (int id : connectedConstraints) {
				auto& constraint = gameState->getShapeInstance(id);
				constraint.selected = true;
			}
		}

		// set all selected shapes as type none to activate deletion process
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!gameState->isShapeAlive(i))
				continue;
			ShapeInstance& shapeInstance = gameState->getShapeInstance(i);
			if (!shapeInstance.selected)
				continue;
			gameState->deleteShape(i);
		}

		// if member was deleted from a group we obviously need to update the loop
		for (int loopShapeIndex : deteledLoopMembersParentLoops) {
			if (gameState->shapes[loopShapeIndex].type == ShapeType::NONE)
				continue;
			updateLoop(gameState, loopShapeIndex);
			editedLoops = true;
		}

		// if we edited loops we need to reorder the loop array, to remove gaps
		if (editedLoops) {
			reorderLoops(gameState);
		}

	}

	void EditorActionSaveScene::execute(GameState* gameState) {

		loopInstances(gameState, [gameState](int i, ShapeInstance& instance) {
			gameState->shapes[i].position = FromDescVec(instance.pData.position);
			});

		saveScene(gameState, gameState->sceneNames[gameState->activeScene]);
	}

	void EditorActionBuild::execute(GameState* gameState) {
		std::string command = "python ../src/editor_scripts/build_project.py";
		//std::thread([command]() {
		system(command.c_str());
			//}).detach();
	}

	void EditorActionCreateLoop::execute(GameState* gameState) {
		int freeIndex = findFreeIndex(gameState);
		int loopIndex = gameState->loopIndex;

		// find selected items and set them to the loop member array
		int loopSize = 0;
		loopInstances(gameState, [&](int i, ShapeInstance& instance) {
			if (instance.shape.type != ShapeType::SPHERE && instance.shape.type != ShapeType::LOOP)
				return;

			// unselect shapes that are already in loops, don't allow them into new loops betray not allowed
			if (instance.shape.parentLoopIndex != UNASSIGNED) {
				instance.selected = false;
			}

			if (instance.selected) {
				gameState->loopMembers[loopIndex + loopSize] = i;
				++loopSize;
			}
			});

		// loops must have at least 2 things
		if (loopSize < 2)
			return;

		// create new shape, why I call them shapes?
		gameState->shapes[freeIndex].type = ShapeType::LOOP;
		gameState->shapes[freeIndex].loopArrayOffset = loopIndex;
		gameState->shapes[freeIndex].radius = DEF_RADIUS_LOOP_INDICATOR;

		// set loop size
		gameState->shapes[freeIndex].loopSize = loopSize;


		// auto unselect
		unselect(gameState);

		// iterate members once more to set parents to all the things
		for (int i = gameState->loopIndex; i < gameState->loopIndex + loopSize; ++i) {
			auto& shape = gameState->shapes[gameState->loopMembers[i]];
			shape.parentLoopIndex = freeIndex;
		}

		// move loop index
		gameState->loopIndex += loopSize;

	}


	void EditorActionLoadScene::execute(GameState* gameState)
	{
		assert(params.intValue != UNASSIGNED);
		saveScene(gameState, gameState->sceneNames[gameState->activeScene]);
		// DELETE EVERYTHING
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			gameState->deleteShape(i);
		}
		gameState->reload = true;
		gameState->activeScene = params.intValue;
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
			gameState->deleteShape(i);
		}
		// save new scene
		saveScene(gameState, params.stringValue);
		gameState->sceneNames.push_back(params.stringValue);
		gameState->reload = true;
		gameState->reset = true;
	}

	void EditorActionOpenScript::execute(GameState* gameState)
	{
		assert(params.intValue != UNASSIGNED);
		std::string sceneName = gameState->sceneNames[gameState->activeScene];
		std::string scriptName = sceneName + std::to_string(params.intValue);
		std::string filename = "../assets/scripts/" + scriptName + ".cpp";

		if (!scriptExists(scriptName)) {
			newScript(gameState, filename, sceneName, params.intValue);
		}


		shell_open_file(filename);
	}

}
