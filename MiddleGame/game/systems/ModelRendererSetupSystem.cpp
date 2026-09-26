#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MidComp/ModelComponent.h"
#include "MidComp/GlobalTransform.h"

class ModelRendererSetupSystem : public middle::MiddleGameplaySystem {
public:
	void init(middle::GameState* gameState) override {
	}
	void update(middle::GameState* gameState) override {
	}
};

static middle::SystemRegistrar<ModelRendererSetupSystem> reg("ModelRendererSetupSystem");
