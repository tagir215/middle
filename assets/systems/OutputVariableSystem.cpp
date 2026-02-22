#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "ProcedureComponent.h"
#include <stack>
#include "OutputVariable.h"
#include "LoopSociety.h"

static int resultId = 0;

class OutputVariableSystem : public middle::MiddleGameplaySystem {

	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto procedure = middle::getComponent<components::ProcedureComponent>(shape);
			if (!procedure)
				return true;

			std::stack<middle::Id>idStack;
			idStack.push(shape.id);
			while (idStack.size() > 0) {
				middle::Id containerId = idStack.top();
				idStack.pop();
				middle::Shape& containerShape = middle::getShape(gameState, containerId.index);
				auto outputVariable = middle::getComponent<components::OutputVariable>(containerShape);
				if (outputVariable && outputVariable->label == "") {
					outputVariable->label = "r" + std::to_string(resultId++);
				}

				auto loop = middle::getComponent<components::LoopSociety>(containerShape);
				for (middle::Id& childId : loop->loopMemberIds) {
					idStack.push(childId);
				}
			}

			return true;
			});
	}
};

static middle::SystemRegistrar<OutputVariableSystem> reg("OutputVariableSystem");
