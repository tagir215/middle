#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "rlgl.h"
#include <filesystem>

class FileDropSystem : public middle::MiddleGameplaySystem {


	void init(middle::GameState* gameState) override {
	}

	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar<FileDropSystem> reg("FileDropSystem");
