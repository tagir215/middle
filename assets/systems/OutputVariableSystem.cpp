#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "ProcedureComponent.h"
#include <stack>
#include "OutputVariable.h"
#include "LoopSociety.h"


class OutputVariableSystem : public middle::MiddleGameplaySystem {
public:

	void init(middle::GameState* gameState) {
	}


	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar<OutputVariableSystem> reg("OutputVariableSystem");
