#pragma once
#include <memory>

namespace middle {

	class GameState;

	enum class SystemUpdateType {
		PREFRAME,
		// Imported systems are dynamically placed to scenes, they are updated between pre and post frame
		IMPORTED,
		POSTFRAME,
		RENDERING,
	};

	// Engine systems are updated in both editor and game mode
	// Editor systems are updated only in editor mode
	// Gameplay systems are updated only in game mode
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