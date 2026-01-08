#pragma once
#include <memory>

namespace middle {

	class GameState;

	class MiddleGameplaySystem {
	public:
		virtual void update(GameState* gameState) = 0;
	};

}