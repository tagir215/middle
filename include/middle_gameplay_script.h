#pragma once
#include "game_state.h"

namespace middle {

	class MiddleGamePlayScript {
	public:
		virtual void onCreate(GameState* gameState) = 0;
		virtual void onUpdate(GameState* gameState) = 0;
		virtual void onDestroy(GameState* gameState) = 0;
	};
}