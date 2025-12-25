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
	// unseelect selected things
	void unselect(GameState* gameState);
	// move shape and its children
	void moveShape(GameState* gameState, ShapeInstance& instance, Vector3 linearVel);
	// loop the shape instances
	template<typename F>
	void loopInstances(GameState* gameState, F func) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!gameState->isShapeAlive(i))
				continue;
			func(i, gameState->getShapeInstance(i));
		}
	}
}