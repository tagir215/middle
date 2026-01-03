#pragma once
#include <memory>

namespace middle {

	class GameState;

	class MiddleGameplayScript {
	public:
		virtual void onCreate(GameState* gameState) = 0;
		virtual void onUpdate(GameState* gameState) = 0;
		virtual void onDestroy(GameState* gameState) = 0;
	};

}