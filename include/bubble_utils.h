#pragma once
#include "LoopSociety.h"
#include "game_state.h"
using namespace middle;

namespace bubble {
	bool pointIntersectBubble(middle::GameState* gameState, middle::Shape& bubble, const Vector3& point);
	// get left right top bottom of a rect containing rect children 
	void loopRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	void loopChildrenOnlyRectBoundingBox(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	void loopRectBoundingBoxInternal(GameState* gameState, const Id& shapeId, float* leftX, float* rightX, float* bottomZ, float* topZ);
	bool buttonClicked(middle::GameState* gameState, middle::Shape& shape, int function);
	std::vector<middle::Id>getNodes(middle::GameState* gameState, components::LoopSociety* loop);
	std::vector<middle::Id>getConstraints(middle::GameState* gameState, components::LoopSociety* loop);
	middle::Id findBubbleWithPatern(middle::GameState* gameState, middle::Id containerBubble);
}
