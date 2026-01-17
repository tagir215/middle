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
		Shape& shapeA = getShape(gameState, indexA);
		Shape& shapeB = getShape(gameState, indexB);

		if (constraintAlreadyExists(gameState, shapeA.id, shapeB.id)) {
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
			std::vector<int> connectedConstraints = findConnectedConstraints(gameState, shape.id);


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
		saveScene(gameState, sceneName);
	}

	void EditorActionBuild::execute(GameState* gameState) {
		std::string command = "python ../src/editor_scripts/build_project.py";
		system(command.c_str());
	}

	void EditorActionCreateLoop::execute(GameState* gameState) {
		int freeIndex = findFreeIndex(gameState);
		int loopIndex = gameState->loopIndex;

		// find selected items 
		std::vector<Id>memberIds;
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!isShapeAlive(gameState, i))
				continue;
			auto loop = getComponent<components::LoopTag>(gameState->shapes[i]);
			auto sphere = getComponent<components::Sphere>(gameState->shapes[i]);
			auto reference = getComponent<components::Reference>(gameState->shapes[i]);
			if ((sphere || loop || reference) && isShapeSelected(gameState, i)) {
				memberIds.push_back(gameState->shapes[i].id);
			}
		}

		// loops must have at least 2 things
		if (memberIds.size() == 0) {
			entities::initLoop(gameState, freeIndex, memberIds, gameState->input.mouseXZ_PlanePos);
		}
		else {
			Vector3 centroid = { 0,0,0 };
			for (int i = 0; i < memberIds.size(); ++i) {
				auto& shape = getShape(gameState, memberIds[i].index);
				auto pos = getComponent<components::Position>(shape);
				centroid += { pos->posX, pos->posY, pos->posZ };
			}
			centroid *= 1.0f / memberIds.size();
			entities::initLoop(gameState, freeIndex, memberIds, centroid);
		}

		// create 

		// auto unselect
		unselect(gameState);
	}


	void EditorActionLoadScene::execute(GameState* gameState)
	{
		// DELETE EVERYTHING
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			deleteShape(gameState, i);
		}
		gameState->reload = true;
		int index = -1;
		for (int i = 0; i < gameState->sceneNames.size(); ++i) {
			if (gameState->sceneNames[i] == sceneName) {
				index = i;
				break;
			}
		}
		gameState->activeScene = index;
		gameState->loopIndex = 0;
	}

	void EditorActionNewScene::execute(GameState* gameState)
	{
		// DELETE EVERYTHING
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			deleteShape(gameState, i);
		}
		gameState->sceneNames.push_back(sceneName);
		int index = gameState->sceneNames.size() - 1;
		gameState->activeScene = index;
		gameState->reload = true;
		gameState->reset = true;
		saveScene(gameState, sceneName);
	}

	void EditorActionImportScene::execute(GameState* gameState)
	{
		loadScene(gameState, sceneName, true, {0,0,0}, findFreeIndex(gameState));
	}


	void EditorActionOpenSystem::execute(GameState* gameState)
	{
		std::string name = systemName;
		shell_open_file("../assets/systems/" + systemName + ".cpp");

	}

	void EditorActionNewSystem::execute(GameState* gameState)
	{
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
		for (int i : selectedIndexes) {
			auto& shape = gameState->shapes[i];
			int componentTypeId = componentTypeMap[componentName];
			assert(shape.componentMap.find(componentTypeId) == shape.componentMap.end());

			Component component;
			component.componentOffset = componentListMap[componentTypeId]->grow();
			shape.componentMap[componentTypeId] = component;
		};
	}

	void EditorActionOpenComponent::execute(GameState* gameState)
	{
		shell_open_file("../assets/components/" + componentName + ".h");
		unselect(gameState);
	}

}
