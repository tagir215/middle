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
	// is the shape selected via mouse
	bool isShapeSelected(GameState* gameState, int index);
	// is the mouse intersecting this shape
	bool isMouseIntersectingShape(GameState* gameState, int index);
	// is shape slot taken and there should be instance
	bool isShapeAlive(GameState* gameState, int index);
	// get pos quickly
	Vector3 getShapePosition(GameState* gameState, int index);
	// get center pos os loops for 
	Vector3 getLoopCentroid(GameState* gameState, int index);
	// get shape instance
	Shape& getShape(GameState* gameState, int index);
	// delete shape , updates generational indexes
	void deleteShape(GameState* gameState, int index);
	// deletes shapes and its children
	void deleteShapeRecursive(GameState* gameState, int index);
	// add shape and updates generations
	void addShape(GameState* gameState, int index, Shape shape);
	// move camera in xz plane moving also the target 
	void moveCameraXZ(Camera3D& initCamera, const Vector3& pos);
	// get shapes selected..
	std::vector<int>getSelectedShapes(GameState* gameState);

	template<typename F>
	void loopInstances(GameState* gameState, F func) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!isShapeAlive(gameState, i))
				continue;
			func(i, gameState->shapes[i]);
		}
	}
}