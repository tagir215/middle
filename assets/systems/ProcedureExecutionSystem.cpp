#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "bubble_algebra_buttons.h"
#include "bubble_actions.h"
#include "Button.h"
#include "ProcedureComponent.h"
#include "CodeFunction.h"
#include "InputVariable.h"
#include "OutputVariable.h"

class ProcedureExecutionSystem : public middle::MiddleGameplaySystem {

	void getTwoInputs(middle::GameState* gameState, middle::Shape& funcShape, components::InputVariable& varA, components::InputVariable& varB) {
		auto variablesLoop = middle::getComponent<components::LoopSociety>(funcShape);
		assert(variablesLoop);
		bool varAFound = false;
		for (middle::Id& childId : variablesLoop->loopMemberIds) {
			auto& childShape = middle::getShape(gameState, childId.index);
			auto inputVar = middle::getComponent<components::InputVariable>(childShape);
			if (!inputVar) {
				continue;
			}

			if (!varAFound) {
				varAFound = true;
				varA = *inputVar;
			}
			else {
				varB = *inputVar;
				return;
			}
		}
		assert(false);
	}

	void getOutput(middle::GameState* gameState, middle::Shape& funcShape, components::OutputVariable& var) {
		std::vector<middle::Id>children;
		middle::getChildren(gameState, funcShape.id, children);
		for (middle::Id& id : children) {
			auto& childShape = middle::getShape(gameState, id.index);
			auto outputVariable = middle::getComponent<components::OutputVariable>(childShape);
			if (outputVariable) {
				var = *outputVariable;
				return;
			}
		}
		assert(false);
	}


	void getNextBlock(middle::GameState* gameState, middle::Shape& funcShape, middle::Id* nextId, bool& atEnd) {
		auto loop = middle::getComponent<components::LoopSociety>(funcShape);
		auto& codeBlock = middle::getShape(gameState, loop->parentLoopId.index);
		auto parentLoop = middle::getComponent<components::LoopSociety>(codeBlock);
		auto& scope = middle::getShape(gameState, parentLoop->parentLoopId.index);
		auto scopeLoop = middle::getComponent<components::LoopSociety>(scope);
		int nextIndex = -1;
		for (int i = 0; i < scopeLoop->loopMemberIds.size(); ++i) {
			middle::Id& id = scopeLoop->loopMemberIds[i];
			if (id == codeBlock.id) {
				nextIndex = i + 1;
				break;
			}
		}

		if (nextIndex < scopeLoop->loopMemberIds.size()) {
			atEnd = false;
			*nextId = scopeLoop->loopMemberIds[nextIndex];
		}
		else {
			atEnd = true;
			nextId = nullptr;
		}

	}

	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {
			auto button = middle::getComponent<components::Button>(shape);
			if (button && button->function == bubble::START_PROCEDURE_BUTTON) {
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				assert(intersectable);
				if (intersectable->intersectingTop && gameState->input.mouseClicked) {
					auto loop = middle::getComponent<components::LoopSociety>(shape);
					auto& procedureShape = middle::getShape(gameState, loop->parentLoopId.index);
					auto procedure = middle::getComponent<components::ProcedureComponent>(procedureShape);
					procedure->executing = true;
					auto procedureLoop = middle::getComponent<components::LoopSociety>(procedureShape);
					procedure->activeCodeBlock = procedureLoop->loopMemberIds[0];
				}
			}

			auto procedure = middle::getComponent<components::ProcedureComponent>(shape);
			if (procedure && procedure->executing) {
				auto& activeBlock = middle::getShape(gameState, procedure->activeCodeBlock.index);
				auto activeLoop = middle::getComponent<components::LoopSociety>(activeBlock);
				if (activeLoop->loopMemberIds.size() > 0) {
					assert(activeLoop->loopMemberIds.size() == 1);
					auto& funcShape = middle::getShape(gameState, activeLoop->loopMemberIds[0].index);
					auto function = middle::getComponent<components::CodeFunction>(funcShape);

					if (function->type == functionTypes::COMBINE) {
						components::InputVariable varA;
						components::InputVariable varB;
						components::OutputVariable output;
						getTwoInputs(gameState, funcShape, varA, varB);
						getOutput(gameState, funcShape, output);
						auto combine = bubbleActions::Combine(varA.unitRef, varB.unitRef);
						combine.execute(gameState);
						combine.finalize(gameState);
					}

					middle::Id nextId;
					bool atEnd = false;
					getNextBlock(gameState, funcShape, &nextId, atEnd);
					if (atEnd) {
						procedure->executing = false;
					}
					procedure->activeCodeBlock = nextId;
				}
			}

			});
	}
};

static middle::SystemRegistrar<ProcedureExecutionSystem> reg("ProcedureExecutionSystem");
