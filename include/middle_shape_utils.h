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
	// is shape slot taken and there should be instance
	bool isShapeAlive(GameState* gameState, int index);
	// get pos quickly
	Vector3 getShapePosition(GameState* gameState, int index);
	// get shape instance
	Shape& getShape(GameState* gameState, int index);
	// delete shape , updates generational indexes
	void deleteShape(GameState* gameState, int index);
	// deletes shapes and its children
	void deleteShapeRecursive(GameState* gameState, int index);
	// add shape and updates generations
	Shape& addShape(GameState* gameState, int index);
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
	// get children in a flat array
	void getChildren(GameState* gameState, Id id, std::vector<Id>& result);
	// get left right top bottom of a rect containing rect children 
	void loopRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	void loopChildrenOnlyRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	void loopRectBoundingBoxInternal(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);

	template<typename F>
	void loopInstances(GameState* gameState, F func) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!isShapeAlive(gameState, i))
				continue;
			func(i, gameState->shapes[i]);
		}
	}
}