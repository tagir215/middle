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

		Shape& shapeA = getShape(gameState, indexA);
		Shape& shapeB = getShape(gameState, indexB);

		if (constraintExistsAt(gameState, shapeA.id, shapeB.id) != UNASSIGNED) {
			return;
		}

		newIndex = findFreeIndex(gameState);

		if (indexA != indexB) {
			auto& shapeA = shapes[indexA];
			auto& shapeB = shapes[indexB];
			auto posA = getShapePosition(gameState, indexA);
			auto posB = getShapePosition(gameState, indexB);
			float distBetween = Vector3Distance(posA, posB);
			entities::initConstraint(gameState, newIndex, indexA, indexB, distBetween);

			// auto unselect
			unselect(gameState);
		}

	}

	void EditorActionNewConstraint::undo(GameState* gameState)
	{
		deleteShape(gameState, newIndex);
	}

	void EditorActionDelete::execute(GameState* gameState) {

		for (int i = 0; i < selectedIndexes.size(); ++i) {
			int shapeIndex = selectedIndexes[i];
			if (middle::isShapeAlive(gameState, shapeIndex)) {
				auto delAction = std::make_unique<middle::EditorActionDeleteSingle>(getShape(gameState, shapeIndex).id);
				delAction->execute(gameState);
				actions.push_back(std::move(delAction));
			}
		}
	}

	void EditorActionDelete::undo(GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void EditorActionSaveScene::execute(GameState* gameState) {
		middle::resetGenerations(gameState);
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
		loadScene(gameState, path, name, true, { 0,0,0 }, newIndex);
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

		oldParentIndex = childLoop->parentLoopId.index;

		Shape& parentShape = getShape(gameState, childLoop->parentLoopId.index);
		auto parentLoop = getComponent<components::LoopSociety>(parentShape);
		assert(parentLoop);

		childLoop->parentLoopId = Id();
		for (int i = 0; i < parentLoop->loopMemberIds.size(); ++i) {
			auto& id = parentLoop->loopMemberIds[i];
			if (id == childShape.id) {
				parentLoop->loopMemberIds.erase(parentLoop->loopMemberIds.begin() + i);
				loopIndex = i;
				return;
			}
		}
	}

	void EditorActionRemoveFromLoop::undo(GameState* gameState)
	{
		EditorActionReparent(oldParentIndex, childIndex).execute(gameState);
		EditorActionChangeLoopMemberIndex(oldParentIndex, childIndex, loopIndex).execute(gameState);
	}

	void checkCircularReferences(GameState * gameState, middle::Id & parentId, middle::Id & id) {
		auto& parent = getShape(gameState, parentId.index);
		assert(parentId != id);
		auto loop = getComponent<components::LoopSociety>(parent);
		if (loop->parentLoopId.index != UNASSIGNED) {
			checkCircularReferences(gameState, loop->parentLoopId, id);
		}
	}

	void EditorActionReparent::execute(GameState * gameState)
	{
		assert(parentIndex != childIndex);
		Shape& childShape = getShape(gameState, childIndex);
		auto childLoop = getComponent<components::LoopSociety>(childShape);
		oldParentIndex = childLoop->parentLoopId.index;

		// remove from old parent 
		if (childLoop->parentLoopId.index != UNASSIGNED) {
			auto removeAction = EditorActionRemoveFromLoop(childIndex);
			removeAction.execute(gameState);
		}

		if (parentIndex != UNASSIGNED) {
			Shape& parentShape = getShape(gameState, parentIndex);
			auto parentLoop = getComponent<components::LoopSociety>(parentShape);
			for (Id id : parentLoop->loopMemberIds) {
				if (id == childShape.id) {
					return;
				}
			}

			parentLoop->loopMemberIds.push_back(childShape.id);
			childLoop->parentLoopId = parentShape.id;
			checkCircularReferences(gameState, parentShape.id, childShape.id);
		}
		// set parent as unassigned
		else {
			childLoop->parentLoopId = middle::Id();
		}

	}

	void EditorActionReparent::undo(GameState * gameState)
	{
		auto metaReparent = EditorActionReparent(oldParentIndex, childIndex);
		metaReparent.execute(gameState);
	}

	void EditorActionChangeLoopMemberIndex::execute(GameState * gameState)
	{
		auto& parentShape = middle::getShape(gameState, parentIndex);
		auto& childShape = middle::getShape(gameState, childIndex);
		auto loop = middle::getComponent<components::LoopSociety>(parentShape);
		assert(loop);

		oldLoopIndex = UNASSIGNED;
		int loopSize = loop->loopMemberIds.size();
		for (int i = 0; i < loopSize; ++i) {
			if (loop->loopMemberIds[i] == childShape.id) {
				oldLoopIndex = i;
			}
		}
		assert(oldLoopIndex != UNASSIGNED);

		loop->loopMemberIds.erase(loop->loopMemberIds.begin() + oldLoopIndex);
		assert(newLoopIndex <= loop->loopMemberIds.size());
		loop->loopMemberIds.insert(loop->loopMemberIds.begin() + newLoopIndex, childShape.id);
	}

	void EditorActionChangeLoopMemberIndex::undo(GameState * gameState)
	{
		auto metaChangeIndex = EditorActionChangeLoopMemberIndex(parentIndex, childIndex, oldLoopIndex);
		metaChangeIndex.execute(gameState);
	}

	void EditorActionCopy::execute(GameState * gameState)
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
			newCopyShapes.push_back(newId.index);

			// add new copy as child to parent of the shape that was copied
			if (parentId.index != UNASSIGNED) {
				auto& parentShape = getShape(gameState, parentId.index);
				auto parentLoop = getComponent<components::LoopSociety>(parentShape);
				parentLoop->loopMemberIds.push_back(newId);
			}

			// placement component until placing is done
			auto position = middle::getComponent<components::Position>(copyShape);
			if (position) {
				auto placable = addComponent<components::PlacementComponent>(copyShape);
				placable->grabbing = true;
			}

			// placement component to children
			std::vector<Id>children;
			getAllChildren(gameState, copyShape.id, children);
			for (Id& id : children) {
				Shape& child = getShape(gameState, id.index);
				addComponent<components::PlacementComponent>(child);
			}
		}
	}

	void EditorActionCopy::undo(GameState * gameState)
	{
		for (int shapeIndex : newCopyShapes) {
			deleteShapeRecursive(gameState, shapeIndex);
		}
	}


	void EditorActionHide::execute(GameState * gameState)
	{
		for (int index : selectedShapes) {
			auto& shape = middle::getShape(gameState, index);
			addComponent<components::HiddenTag>(shape);
		}
	}

	void EditorActionHide::undo(GameState * gameState)
	{
		for (int index : selectedShapes) {
			auto& shape = middle::getShape(gameState, index);
			deleteComponent<components::HiddenTag>(shape);
		}
	}

	void EditorActionUnhide::execute(GameState * gameState)
	{
		std::vector<int>& unhidded = unhidIndexes;
		loopInstances(gameState, [&unhidded](int i, middle::Shape& shape) {
			if (middle::getComponent<components::HiddenTag>(shape)) {
				deleteComponent<components::HiddenTag>(shape);
				unhidded.push_back(shape.id.index);
			}
			return true;
			});
	}

	void EditorActionUnhide::undo(GameState * gameState)
	{
		auto hide = EditorActionHide(unhidIndexes);
		hide.execute(gameState);
	}

	void EditorActionMove::execute(GameState * gameState)
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

	void EditorActionMove::undo(GameState * gameState)
	{
		for (int i = 0; i < selectedShapes.size(); ++i) {
			auto& shape = getShape(gameState, selectedShapes[i]);
			auto position = getComponent<components::Position>(shape);
			Vector3 currentPos = { position->posX, position->posY, position->posZ };
			Vector3 displacement = currentPos - oldPositions[i];
			middle::moveShape(gameState, selectedShapes[i], Vector3Negate(displacement));
		}
	}

	void EditorActionDeleteSingle::execute(GameState * gameState)
	{
		middle::Id parentId = middle::getParent(gameState, id);
		if (parentId.index != middle::UNASSIGNED) {
			removeFromLoop = std::make_unique<EditorActionRemoveFromLoop>(id.index);
			removeFromLoop->execute(gameState);
		}
		middle::saveTempShape(gameState, id);
		middle::deleteShapeRecursive(gameState, id.index);
	}

	void EditorActionDeleteSingle::undo(GameState* gameState)
	{
		middle::loadTempShape(gameState, id);
		if (removeFromLoop) {
			removeFromLoop->undo(gameState);
		}
	}

	void EditorActionCopySingle::execute(GameState* gameState)
	{
		resultId = middle::deepCopyShape(gameState, id.index);
	}

	void EditorActionCopySingle::undo(GameState* gameState)
	{
		middle::deleteShapeRecursive(gameState, resultId.index);
	}

	void EditorActionRegisterShape::execute(GameState* gameState)
	{
		int freeIndex = middle::findFreeIndex(gameState);
		auto& newShape = middle::addShape(gameState, freeIndex);
		middle::Id newId = newShape.id;
		shapeToRegister.id = newId;
		newShape = shapeToRegister;
		newShapeId = newId;
	}

	void EditorActionRegisterShape::undo(GameState* gameState)
	{
		middle::deleteShapeRecursive(gameState, newShapeId.index);
	}

}
