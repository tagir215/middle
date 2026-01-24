#pragma once
#include "game_state.h"

namespace bubble {
	bool pointIntersectBubble(middle::GameState* gameState, middle::Shape& bubble, const Vector3& point);
}
