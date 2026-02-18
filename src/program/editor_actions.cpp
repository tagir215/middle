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
#include "PlacementComponent.h"
#include "CameraEntity.h"
#include "HiddenTag.h"

namespace middle {

	void EditorActionNewSphere::execute(GameState* gameState) {
		auto& shapes = gameState->shapes;
		int freeIndex = findFreeIndex(gameState);
		entities::initJoint(gameState, freeIndex, position);
		newIndex = freeIndex;
	}
	void EditorActionNewSphere::undo(GameState* gameState)
	{
		deleteShape(gameState, newIndex);
	}

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

		if (constraintExistsAt(gameState, shapeA.id, shapeB.id) != UNASSIGNED) {
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

	void EditorActionNewConstraint::undo(GameState* gameState)
	{
		deleteShape(gameState, newIndex);
	}

	void EditorActionDelete::execute(GameState* gameState) {
		// loops of deleted spheres that belong to loops
		std::set<int>deteledLoopMembersParentLoops;

		for (int i : selectedIndexes) {
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

			saveShape(gameState, shape.id, "../src/editor_data/temp/", "delshape" + std::to_string(i));

			deleteShape(gameState, i);
		}

	}

	void EditorActionDelete::undo(GameState* gameState)
	{
		for (int i : selectedIndexes) {
			loadShape(gameState, "../src/editor_data/temp/", "delshape" + std::to_string(i), false);
		}
	}

	void EditorActionSaveScene::execute(GameState* gameState) {
		saveScene(gameState, sceneName);
	}

	void EditorActionSaveScene::undo(GameState* gameState)
	{
	}

	void EditorActionBuild::execute(GameState* gameState) {
		std::string command = "python ../src/editor_scripts/build_project.py";
		system(command.c_str());
	}

	void EditorActionBuild::undo(GameState* gameState)
	{
	}

	void EditorActionCreateLoop::execute(GameState* gameState) {
		newIndex = findFreeIndex(gameState);
		int loopIndex = gameState->loopIndex;

		std::vector<Id>ids;
		for (int i : memberIndexes) {
			ids.push_back(gameState->ids[i]);
			auto& memberShape = middle::getShape(gameState, i);
			auto loop = middle::getComponent<components::LoopSociety>(memberShape);
			oldParents.push_back(loop->parentLoopId);
		}

		// set position to mouse pos
		if (memberIndexes.size() == 0) {
			entities::initLoop(gameState, newIndex, ids, gameState->input.mouseXZ_PlanePos);
		}
		// set position to centroid
		else {
			Vector3 centroid = { 0,0,0 };
			for (int i = 0; i < ids.size(); ++i) {
				auto& shape = getShape(gameState, ids[i].index);
				auto pos = getComponent<components::Position>(shape);
				centroid += { pos->posX, pos->posY, pos->posZ };
			}
			centroid *= 1.0f / ids.size();
			entities::initLoop(gameState, newIndex, ids, centroid);
		}

		// auto unselect
		unselect(gameState);
	}

	void EditorActionCreateLoop::undo(GameState* gameState)
	{
		for (int i = 0; i < memberIndexes.size(); ++i) {
			auto reparent = EditorActionReparent(oldParents[i].index, memberIndexes[i]);
			reparent.execute(gameState);
		}

		deleteShape(gameState, newIndex);
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

	void EditorActionLoadScene::undo(GameState* gameState)
	{
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

	void EditorActionNewScene::undo(GameState* gameState)
	{
	}

	void EditorActionImportScene::execute(GameState* gameState)
	{
		newIndex = findFreeIndex(gameState);
		loadScene(gameState, "../assets/scenes/", sceneName, true, {0,0,0}, newIndex);
	}

	void EditorActionImportScene::undo(GameState* gameState)
	{
		deleteShape(gameState, newIndex);
	}


	void EditorActionOpenSystem::execute(GameState* gameState)
	{
		std::string name = systemName;
		shell_open_file("../assets/systems/" + systemName + ".cpp");
		gameState->closeGame = true;
	}

	void EditorActionOpenSystem::undo(GameState* gameState)
	{
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

	void EditorActionNewSystem::undo(GameState* gameState)
	{
	}

	void EditorActionImportSystem::execute(GameState* gameState) 
	{
		newIndex = findFreeIndex(gameState);
		entities::initSystem(gameState, newIndex, { 0,0,0 }, systemName);
	}

	void EditorActionImportSystem::undo(GameState* gameState)
	{
		deleteShape(gameState, newIndex);
	}

	void EditorActionNewCamera::execute(GameState* gameState)
	{
		int freeIndex = findFreeIndex(gameState);
		Vector3& pos = gameState->editorState.camera.position;
		entities::initCamera(gameState, freeIndex, pos, up, target, fieldOfView, projection);
	}

	void EditorActionNewCamera::undo(GameState* gameState)
	{
	}

	void EditorActionSelectCamera::execute(GameState* gameState)
	{

	}

	void EditorActionSelectCamera::undo(GameState* gameState)
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

	void EditorActionNewComponent::undo(GameState* gameState)
	{
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

	void EditorActionImportComponent::undo(GameState* gameState)
	{
		for (int i : selectedIndexes) {
			auto& shape = gameState->shapes[i];
			int componentTypeId = componentTypeMap[componentName];
			assert(shape.componentMap.find(componentTypeId) != shape.componentMap.end());
			Component component = shape.componentMap[componentTypeId];
			componentListMap[componentTypeId]->shrink(component.componentOffset);
			shape.componentMap.erase(componentTypeId);
		}
	}

	void EditorActionRemoveComponent::execute(GameState* gameState)
	{
		auto importAction = EditorActionImportComponent(componentName, selectedIndexes);
		importAction.undo(gameState);
	}

	void EditorActionRemoveComponent::undo(GameState* gameState)
	{
		auto importAction = EditorActionImportComponent(componentName, selectedIndexes);
		importAction.execute(gameState);
	}


	void EditorActionOpenComponent::execute(GameState* gameState)
	{
		shell_open_file("../assets/components/" + componentName + ".h");
		unselect(gameState);
		gameState->closeGame = true;
	}

	void EditorActionOpenComponent::undo(GameState* gameState)
	{
	}

	void EditorActionRemoveFromLoop::execute(GameState* gameState)
	{
		Shape& childShape = getShape(gameState, childIndex);
		auto childLoop = getComponent<components::LoopSociety>(childShape);
		assert(childLoop);
		if (childLoop->parentLoopId.index == UNASSIGNED) {
			return;
		}
		Shape& parentShape = getShape(gameState, childLoop->parentLoopId.index);
		auto parentLoop = getComponent<components::LoopSociety>(parentShape);
		assert(parentLoop);

		childLoop->parentLoopId = Id();
		for (int i = 0; i < parentLoop->loopMemberIds.size(); ++i) {
			auto& id = parentLoop->loopMemberIds[i];
			if (id == childShape.id) {
				parentLoop->loopMemberIds.erase(parentLoop->loopMemberIds.begin() + i);
				return;
			}
		}
	}

	void EditorActionRemoveFromLoop::undo(GameState* gameState)
	{
	}

	void checkCircularReferences(GameState* gameState, middle::Id& parentId, middle::Id& id) {
		auto& parent = getShape(gameState, parentId.index);
		assert(parentId != id);
		auto loop = getComponent<components::LoopSociety>(parent);
		if (loop->parentLoopId.index != UNASSIGNED) {
			checkCircularReferences(gameState, loop->parentLoopId, id);
		}
	}

	void EditorActionReparent::execute(GameState* gameState)
	{
		assert(parentIndex != childIndex);
		Shape& parentShape = getShape(gameState, parentIndex);
		Shape& childShape = getShape(gameState, childIndex);
		auto parentLoop = getComponent<components::LoopSociety>(parentShape);
		auto childLoop = getComponent<components::LoopSociety>(childShape);
		assert(parentLoop);
		assert(childLoop);
		for (Id id : parentLoop->loopMemberIds) {
			if (id == childShape.id) {
				return;
			}
		}

		// remove from old parent 
		if (childLoop->parentLoopId.index != UNASSIGNED) {
			auto removeAction = EditorActionRemoveFromLoop(childIndex);
			removeAction.execute(gameState);
		}

		parentLoop->loopMemberIds.push_back(childShape.id);
		childLoop->parentLoopId = parentShape.id;

		checkCircularReferences(gameState, parentShape.id, childShape.id);
	}

	void EditorActionReparent::undo(GameState* gameState)
	{
	}

	void EditorActionChangeLoopMemberIndex::execute(GameState* gameState)
	{
		auto& parentShape = middle::getShape(gameState, parentIndex);
		auto& childShape = middle::getShape(gameState, childIndex);
		auto loop = middle::getComponent<components::LoopSociety>(parentShape);
		assert(loop);

		int childLoopIndex = -1;
		int loopSize = loop->loopMemberIds.size();
		for (int i = 0; i < loopSize; ++i) {
			if (loop->loopMemberIds[i] == childShape.id) {
				childLoopIndex = i;
			}
		}
		assert(childLoopIndex != -1);

		loop->loopMemberIds.erase(loop->loopMemberIds.begin() + childLoopIndex);
		assert(newLoopIndex <= loop->loopMemberIds.size());
		loop->loopMemberIds.insert(loop->loopMemberIds.begin() + newLoopIndex, childShape.id);
	}

	void EditorActionChangeLoopMemberIndex::undo(GameState* gameState)
	{
	}

	void EditorActionCopy::execute(GameState* gameState)
	{
		unselect(gameState);

		for (int shapeIndex : selectedShapes) {
			// create new shape
			auto& ogShape = getShape(gameState, shapeIndex);

			// store parent id
			middle::Id parentId;
			auto loop = getComponent<components::LoopSociety>(ogShape);
			if (loop) {
				parentId = loop->parentLoopId;
			}

			Id& newId = deepCopyShape(gameState, shapeIndex, parentId.index);
			auto& copyShape = getShape(gameState, newId.index);

			// add new copy as child to parent of the shape that was copied
			if (parentId.index != UNASSIGNED) {
				auto& parentShape = getShape(gameState, parentId.index);
				auto parentLoop = getComponent<components::LoopSociety>(parentShape);
				parentLoop->loopMemberIds.push_back(newId);
			}

			// placement component until placing is done
			auto placable = addComponent<components::PlacementComponent>(copyShape);
			placable->grabbing = true;

			// placement component to children
			std::vector<Id>children;
			getChildren(gameState, copyShape.id, children);
			for (Id& id : children) {
				Shape& child = getShape(gameState, id.index);
				addComponent<components::PlacementComponent>(child);
			}
		}
	}

	void EditorActionCopy::undo(GameState* gameState)
	{
	}


	void EditorActionHide::execute(GameState* gameState)
	{
		for (int index : selectedShapes) {
			auto& shape = middle::getShape(gameState, index);
			addComponent<components::HiddenTag>(shape);
		}
	}

	void EditorActionHide::undo(GameState* gameState)
	{
	}

	void EditorActionUnhide::execute(GameState* gameState)
	{
		loopInstances(gameState, [](int i, middle::Shape& shape) {
			if (middle::getComponent<components::HiddenTag>(shape)) {
				deleteComponent<components::HiddenTag>(shape);
			}
			});
	}

	void EditorActionUnhide::undo(GameState* gameState)
	{
	}

	void EditorActionMove::execute(GameState* gameState)
	{
		oldPositions.resize(selectedShapes.size());

		for (int i = 0; i < selectedShapes.size(); ++i) {
			auto& shape = getShape(gameState, selectedShapes[i]);
			auto position = getComponent<components::Position>(shape);
			oldPositions[i] = { position->posX, position->posY, position->posZ };
		}

		for (int i = 0; i < newPositions.size(); ++i) {
			Vector3 displacement = newPositions[i] - oldPositions[i];
			middle::moveShape(gameState, selectedShapes[i], displacement);
		}
	}

	void EditorActionMove::undo(GameState* gameState)
	{
		for (int i = 0; i < selectedShapes.size(); ++i) {
			auto& shape = getShape(gameState, selectedShapes[i]);
			auto position = getComponent<components::Position>(shape);
			Vector3 currentPos = { position->posX, position->posY, position->posZ };
			Vector3 displacement = currentPos - oldPositions[i];
			middle::moveShape(gameState, selectedShapes[i], Vector3Negate(displacement));
		}
	}

}
