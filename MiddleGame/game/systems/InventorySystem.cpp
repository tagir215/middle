#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "MidComp/Inventory.h"
#include "MidComp/LoopSociety.h"
#include "middle_shape_utils.h"
#include "MidComp/Rectangle.h"
#include "MidComp/Position.h"
#include "MidComp/Offset.h"
#include "editor_file_utils.h"
#include "MidComp/InventoryItem.h"

class InventorySystem : public middle::MiddleGameplaySystem {

public:
	void init(middle::GameState* gameState) {
	}

	void update(middle::GameState* gameState) override {
	}
};

static middle::SystemRegistrar<InventorySystem> reg("InventorySystem");
