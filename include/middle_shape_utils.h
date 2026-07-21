#pragma once
#include <vector>
#include "game_state.h"

namespace middle {
	// next free index in shape array
	int findFreeIndex(GameState* gameState);
	// constraints connected to shape
	std::vector<int>findConnectedConstraints(GameState* gameState, Id id);
	// this constraint already exists... don't make duplicates
	int constraintExistsAt(GameState* gameState, Id idA, Id idB);
	// unselect selected things
	void unselect(GameState* gameState);
	// drag shape and its children
	void dragShape(GameState* gameState, int index, Vector3 linearVel);
	// move shape and its chidlren
	void moveShape(GameState* gameState, int index, const Vector3& displacement);
	// find container of containers containers
	int findHighestLevelContainer(GameState* gameState, int index);
	// loop the shape instances
	int findHighestUsedIndex(GameState* gameState);
	// find next usable ghost index
	int findNextFreeGhostIndex(GameState* gameState);
	// is the shape real editable thing or a ghost (reference from other scene)
	bool isGhostShape(int index);
	// check that if the child belong to the parent or parents children
	bool isRecursiveChildOf(GameState* gameState, int childIndex, int parentIndex);
	// is the shape selected via mouse
	bool isShapeSelected(GameState* gameState, int index);
	// is the mouse intersecting this shape
	bool isMouseIntersectingShape(GameState* gameState, int index);
	// is id generation correct?  and is alive
	bool isValidId(GameState* gameState, middle::Id id);
	// get pos quickly
	Vector3 getGlobalPosition(GameState* gameState, int index);
	// get shape instance
	Shape& getShape(GameState* gameState, int index);
	// delete shape , updates generational indexes
	void deleteShape(GameState* gameState, int index, bool deleteComponentsOnly = false);
	// deletes shapes and its children
	void deleteShapeRecursive(GameState* gameState, int index, bool deleteComponentsOnly = false);
	// add shape and updates generations
	Shape& registerShape(GameState* gameState, middle::Shape shape);
	// add shape and updates generations
	Shape& registerShapeAtIndex(GameState* gameState, middle::Shape shape, int index);
	// add shape and updates generations
	Shape& registerAsGhostShape(GameState* gameState, middle::Shape shape);
	// add shape, doesn't update generations
	Shape& insertShape(GameState* gameState, middle::Id& id);
	// adds not serialized ghost shape and updates generations
	Shape& addGhostShape(GameState* gameState);
	// move camera in xz plane moving also the target 
	void moveCameraXZ(Camera3D& initCamera, const Vector3& pos);
	// get shapes selected..
	std::vector<int>getSelectedShapes(GameState* gameState);
	// return first shape intersect by mouse
	int getMouseIntersectedShape(GameState* gameState);
	// copy shape 
	Id copyShape(GameState* gameState, int shapeToCopyIndex, int parentIndex = UNASSIGNED);
	// copy shape and its children
	Id deepCopyShape(GameState* gameState, int shapeToCopyIndex, int parentIndex = UNASSIGNED);
	// copy shape and preserve its global coordinate
	Id deepCopyShapeGlobalCoordinates(GameState* gameState, middle::Id id);
	// get vertices of rectangles
	std::vector<Vector3>getRectVertices(GameState* gameState, const Id& shapeId);
	// get scale and multiply it with all the parents scales
	Vector3 getTotalScale(GameState* gameState, const Id& shapeId);
	// get parent of shape with loopSocietyComponent
	Id getParent(GameState* gameState, Id& id);
	// get children in a flat array
	void getChildren(GameState* gameState, Id id, std::vector<Id>& result);
	// get children in a flat array
	void getAllChildren(GameState* gameState, Id id, std::vector<Id>& result);
	// get children in a flat array
	void getChildrenWithComp(GameState* gameState, Id id, std::vector<Id>& result, int typeId);
	// get children in a flat array
	void getAllChildrenWithComp(GameState* gameState, Id id, std::vector<Id>& result, int typeId);
	// iterate children and return first child with component type id
	middle::Id getFirstChildWithComponent(GameState* gameState, Id& id, int typeId);
	// get index of child
	int getLoopIndex(GameState* gameState, Id& parentId, Id& childId);
	// find shape with comp
	middle::Id findFirstShapeWithComp(GameState* gameState, int typeId);
	// get children in a flat array
	void findShapesWithComp(GameState* gameState, std::vector<Id>& result, int typeId);
	// check if is id is assigned and up to date
	bool isIdCurrent(GameState* gameState, middle::Id& id);
	// new comp cache for caching components for better cache locality of components
	components::CompCache* newCompCache(GameState* gameState, const std::string& systemName);
	// queue action
	void queueAction(GameState* gameState, std::shared_ptr<EditorActionContainer> container);
	// queue action for editor, with undos
	void queueEditorAction(GameState* gameState, std::shared_ptr<EditorActionContainer> container);
	// get transform matrix for some id with components globaltransform, localPos, localScale, and probably rotation in future 
	Matrix getTransformMatrix(GameState* gameState, middle::Id id);
	// project world coordinate as local coordinate 
	Vector3 projectGlobalCoordinateToLocalCoordinate(GameState* gameState, const Vector3& globalCoord, middle::Id shapeId);
	// project local coordinate to match old global coordinate
	void updateLocalCoordinateToProjectedGlobalCoordinate(GameState* gameState, middle::Id id, middle::Id oldParentId);

	template<typename F>
	void loopInstances(GameState* gameState, F func) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!isValidId(gameState, gameState->ids[i]))
				continue;
			if (!func(i, gameState->shapes[i])) {
				break;
			}
		}
	}
}