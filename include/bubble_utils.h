#pragma once
#include "game_state.h"
using namespace middle;

namespace bubble {
	bool pointIntersectBubble(middle::GameState* gameState, middle::Shape& bubble, const Vector3& point);
	// get left right top bottom of a rect containing rect children 
	void loopRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	void loopChildrenOnlyRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	void loopRectBoundingBoxInternal(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	bool buttonClicked(middle::GameState* gameState, middle::Shape& shape, int function);
}
