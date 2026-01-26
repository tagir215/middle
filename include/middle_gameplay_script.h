#pragma once
#include <memory>

namespace middle {

	class GameState;

	enum class SystemUpdateType {
		PREFRAME,
		IMPORTED,
		POSTFRAME,
		RENDERING,
	};

	enum class SystemModeType {
		ENGINE,
		EDITOR,
		GAMEPLAY,
	};

	class MiddleGameplaySystem {
	public:
		SystemUpdateType systemUpdateType = SystemUpdateType::IMPORTED;
		SystemModeType systemModeType = SystemModeType::GAMEPLAY;
		virtual void update(GameState* gameState) = 0;
	};

}