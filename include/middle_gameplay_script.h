#pragma once
#include <memory>

namespace middle {

	class GameState;

	enum class SystemUpdateType {
		PREFRAME,
		// Imported systems are dynamically placed to scenes, they are updated between pre and post frame
		GAMEPLAY_MIDFRAME,
		GAMEPLAY_POSTFRAME,
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
		virtual ~MiddleGameplaySystem() = default;
		SystemUpdateType systemUpdateType = SystemUpdateType::GAMEPLAY_MIDFRAME;
		SystemModeType systemModeType = SystemModeType::GAMEPLAY;
		virtual void init(GameState* gameState) = 0;
		virtual void update(GameState* gameState) = 0;
	};

}