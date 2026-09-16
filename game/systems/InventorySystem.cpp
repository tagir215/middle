#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "Inventory.h"
#include "LoopSociety.h"
#include "middle_shape_utils.h"
#include "Rectangle.h"
#include "Position.h"
#include "Offset.h"
#include "editor_file_utils.h"
#include "InventoryItem.h"

class InventorySystem : public middle::MiddleGameplaySystem {

public:
	void init(middle::GameState* gameState) {
	}

	void update(middle::GameState* gameState) override {
	}
};

static middle::SystemRegistrar<InventorySystem> reg("InventorySystem");
