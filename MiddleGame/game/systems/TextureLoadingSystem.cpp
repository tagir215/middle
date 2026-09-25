#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "TextureComponent.h"
#include "MouseSelectable.h"
#include "middle_shape_utils.h"
#include "editor_actions.h"
#include "component_utils.h"

class TextureLoadingSystem : public middle::MiddleGameplaySystem {
public:
	TextureLoadingSystem() {
	}

	void init(middle::GameState* gameState) override {
	}


	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar<TextureLoadingSystem> reg("TextureLoadingSystem");
