#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "Reference.h"
#include "Rectangle.h"
#include "Position.h"
#include "LoopSociety.h"
#include "CodeBlock.h"

class CodeBlockSystem : public middle::MiddleGameplaySystem {
public:
	CodeBlockSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
	}

	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto codeBlock = middle::getComponent<components::CodeBlock>(shape);
			if (!codeBlock)
				return;


			});
	}
};

static middle::SystemRegistrar<CodeBlockSystem> reg("CodeBlockSystem");
