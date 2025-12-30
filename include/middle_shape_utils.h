#pragma once
#include <vector>
#include "game_state.h"

namespace middle {
	// next free index in shape array
	int findFreeIndex(GameState* gameState);
	// constraints connected to shape
	std::vector<int>findConnectedConstraints(GameState* gameState, int id);
	// this constraint already exists... don't make duplicates
	bool constraintAlreadyExists(GameState* gameState, int indexA, int indexB);
	// update loop if some members have been deleted 
	void updateLoop(GameState* gameState, int id);
	// reorder loop member array, after editing loops
	void reorderLoops(GameState* gameState);
	// return child indexes of loops, because it's so confusing to think about everytime
	std::vector<int> getChildIndexes(GameState* gameState, int id);
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
	// return if the shape is of sphere family
	bool isSphere(GameState* gameState, int index);
	// is it a loop or a reference?
	bool isContainer(GameState* gameState, int index);
	// is the shape real editable thing or a ghost (reference from other scene)
	bool isGhostShape(int index);
	// is slot free in shape array
	bool isSlotFree(GameState* gameStaet, int index);
	// is shape slot taken and there should be instance
	bool isShapeAlive(GameState* gameState, int index);
	// get shape instance
	ShapeInstance& getShapeInstance(GameState* gameState, int index);
	// delete shape , updates generational indexes
	void deleteShape(GameState* gameState, int index);
	// deletes shapes and its children
	void deleteShapeRecursive(GameState* gameState, int index);
	// add shape and updates generations
	void addShape(GameState* gameState, int index, Shape shape);
	// add isntance make generation match shape
	void addInstance(GameState* gameState, int index, ShapeInstance instance);
	// move camera in xz plane moving also the target 
	void moveCameraXZ(Camera3D& camera, const Vector3& pos);

	template<typename F>
	void loopInstances(GameState* gameState, F func) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!isShapeAlive(gameState, i))
				continue;
			func(i, getShapeInstance(gameState, i));
		}
	}
}