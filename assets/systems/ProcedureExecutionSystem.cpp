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
#include <stack>
#include "CodeBlock.h"
#include "IfComponent.h"

class ProcedureExecutionSystem : public middle::MiddleGameplaySystem {

	void getOneInput(middle::GameState* gameState, middle::Shape& funcShape, components::InputVariable& inputVariable) {
		auto variablesLoop = middle::getComponent<components::LoopSociety>(funcShape);
		assert(variablesLoop);
		for (middle::Id& childId : variablesLoop->loopMemberIds) {
			auto& childShape = middle::getShape(gameState, childId.index);
			auto inputVar = middle::getComponent<components::InputVariable>(childShape);
			if (!inputVar) {
				continue;
			}

			inputVariable = *inputVar;
			return;
		}
		assert(false);
	}

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


	void getNextBlock(middle::GameState* gameState, middle::Id& funcShapeId, middle::Id* nextId, bool& atEnd) {

		std::stack<middle::Id> functionStack;
		functionStack.push(funcShapeId);
		while (functionStack.size() > 0) {
			middle::Id& id = functionStack.top();
			functionStack.pop();
			auto& funcShape = middle::getShape(gameState, id.index);

			auto loop = middle::getComponent<components::LoopSociety>(funcShape);
			auto& codeBlockShape = middle::getShape(gameState, loop->parentLoopId.index);
			auto parentLoop = middle::getComponent<components::LoopSociety>(codeBlockShape);
			auto& scope = middle::getShape(gameState, parentLoop->parentLoopId.index);
			auto scopeLoop = middle::getComponent<components::LoopSociety>(scope);
			int nextIndex = -1;
			for (int i = 0; i < scopeLoop->loopMemberIds.size(); ++i) {
				middle::Id& id = scopeLoop->loopMemberIds[i];
				if (id == codeBlockShape.id) {
					auto codeBlock = middle::getComponent<components::CodeBlock>(codeBlockShape);
					if (codeBlock->type == codeBlockTypes::BLOCK || codeBlock->exitLoop) {
						nextIndex = i + 1;
					}
					// looptypes don't advance nextIndex
					else if (codeBlock->type == codeBlockTypes::LOOP_BLOCK) {
						nextIndex = i;
					}
					break;
				}
			}

			if (nextIndex < scopeLoop->loopMemberIds.size()) {
				atEnd = false;
				*nextId = scopeLoop->loopMemberIds[nextIndex];
			}
			else {
				// check if the parent is a function, if it is look for code block from its friends
				middle::Id& parentId = middle::getParent(gameState, scope.id);
				if (parentId.index != middle::UNASSIGNED) {
					auto& parentShape = middle::getShape(gameState, parentId.index);
					auto funcComp = middle::getComponent<components::CodeFunction>(parentShape);
					if (funcComp) {
						functionStack.push(parentId);
						break;
					}
				}
				// in other case we know we are at end 
				atEnd = true;
				nextId = nullptr;
			}

		}
	}

	// result id is first function of one of the conditional blocks, or if those don't contain functions result is -1 unassigned
	void getConditionalFunc(middle::GameState* gameState, bool isConditionTrue, const middle::Id& ifFunc, middle::Id& resultId) {
		auto& ifShape = middle::getShape(gameState, ifFunc.index);
		auto ifComp = middle::getComponent<components::IfComponent>(ifShape);
		assert(ifComp);
		std::vector<middle::Id>ifChildren;
		middle::getChildren(gameState, ifShape.id, ifChildren);
		assert(ifChildren.size() == 2);
		const int conditionTrueIndex = 0;
		const int conditionFalseIndex = 1;
		middle::Id scopeId;
		if (isConditionTrue) {
			scopeId = ifChildren[conditionTrueIndex];
		}
		else {
			scopeId = ifChildren[conditionFalseIndex];
		}
		auto& scopeShape = middle::getShape(gameState, scopeId.index);
		std::vector<middle::Id>scopeChildren;
		middle::getChildren(gameState, scopeShape.id, scopeChildren);
		// return UNASSIGNED
		if (scopeChildren.size() == 0) {
			resultId = middle::Id();
			return;
		}
		auto& blockShape = middle::getShape(gameState, scopeChildren[0].index);
		std::vector<middle::Id>blockChildren;
		middle::getChildren(gameState, blockShape.id, blockChildren);
		// return UNASSIGNED
		if (blockChildren.size() == 0) {
			resultId = middle::Id();
			return;
		}
		// return first function from one of the conditional scopes
		resultId = blockChildren[0];
		return;
	}

	// execute funcId,  landedFuncId is where the latest execution took place
	void executeFunctions(middle::GameState* gameState, middle::Id& funcId, middle::Id& landedFuncId) {
		auto& funcShape = middle::getShape(gameState, funcId.index);
		auto function = middle::getComponent<components::CodeFunction>(funcShape);
		// assume landedFuncId is same as funcId, however can change due to if statements
		landedFuncId = funcId;

		// combine functions are either multiplciations or additions
		if (function->type == functionTypes::COMBINE) {
			components::InputVariable varA;
			components::InputVariable varB;
			components::OutputVariable output;
			getTwoInputs(gameState, funcShape, varA, varB);
			getOutput(gameState, funcShape, output);
			assert(varA.unitRef.index != middle::UNASSIGNED);
			assert(varB.unitRef.index != middle::UNASSIGNED);
			middle::Id& parentId = middle::getParent(gameState, varA.unitRef);
			auto& parentShape = middle::getShape(gameState, parentId.index);
			auto bubbleMultiplication = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);

			if (bubbleMultiplication) {
				auto multiply = bubbleActions::Multiply(parentShape.id, varA.unitRef, varB.unitRef);
				multiply.execute(gameState);
				multiply.finalize(gameState);
			}
			else {
				auto combine = bubbleActions::Combine(varA.unitRef, varB.unitRef);
				combine.execute(gameState);
				combine.finalize(gameState);
			}
		}

		// exit loops
		else if (function->type == functionTypes::EXIT_LOOP) {
			middle::Id parentId = middle::getParent(gameState, funcShape.id);
			std::stack<middle::Id> parentIds;
			parentIds.push(parentId);
			while (parentIds.size() > 0) {
				middle::Id& currentParent = parentIds.top();
				parentIds.pop();
				auto& parentShape = middle::getShape(gameState, currentParent.index);
				auto codeBlock = middle::getComponent<components::CodeBlock>(parentShape);
				if (codeBlock && codeBlock->type == codeBlockTypes::LOOP_BLOCK) {
					codeBlock->exitLoop = true;
				}
				else {
					parentIds.push(middle::getParent(gameState, currentParent));
				}
			}
		}

		// find bubbles: store found bubble to output, execute upper scope if found, execute lower scope if didn't find
		else if (function->type == functionTypes::FIND_BUBBLE) {
			components::InputVariable input;
			components::OutputVariable output;
			getOneInput(gameState, funcShape, input);
			getOutput(gameState, funcShape, output);
			std::vector<middle::Id>inputChildren;
			middle::getChildren(gameState, input.unitRef, inputChildren);
			bool isConditionTrue = false;
			for (middle::Id& id : inputChildren) {
				auto& childShape = middle::getShape(gameState, id.index);
				auto bubbleComp = middle::getComponent<components::BubbleComponent>(childShape);
				if (!bubbleComp) {
					continue;
				}
				bubbleActions::updateVariable(gameState, childShape.id, output.label);
				isConditionTrue = true;
				break;
			}
			middle::Id newLandedFuncId;
			getConditionalFunc(gameState, isConditionTrue, funcShape.id, newLandedFuncId);
			if (newLandedFuncId.index != middle::UNASSIGNED) {
				landedFuncId = newLandedFuncId;
			}
		}

		// find fractions: store found bubble to output, execute upper scope if found, execute lower scope if didn't find
		else if (function->type == functionTypes::FIND_FRACTION) {
			components::InputVariable input;
			components::OutputVariable output;
			getOneInput(gameState, funcShape, input);
			getOutput(gameState, funcShape, output);
			std::vector<middle::Id>inputChildren;
			middle::getChildren(gameState, input.unitRef, inputChildren);
			bool isConditionTrue = false;
			for (middle::Id& id : inputChildren) {
				auto& childShape = middle::getShape(gameState, id.index);
				auto fractional = middle::getComponent<components::FractionalComponent>(childShape);
				if (!fractional) {
					continue;
				}
				bubbleActions::updateVariable(gameState, childShape.id, output.label);
				isConditionTrue = true;
				break;
			}
			middle::Id newLandedFuncId;
			getConditionalFunc(gameState, isConditionTrue, funcShape.id, newLandedFuncId);
			if (newLandedFuncId.index != middle::UNASSIGNED) {
				landedFuncId = newLandedFuncId;
			}
		}

		else if (function->type == functionTypes::INVERSE) {

		}

		// add new term
		else if (function->type == functionTypes::NEW_TERM) {
			components::InputVariable input;
			components::OutputVariable output;
			getOneInput(gameState, funcShape, input);
			getOutput(gameState, funcShape, output);
			assert(input.unitRef.index != middle::UNASSIGNED);
			middle::Id topBubbleId = bubbleActions::topLevelBubble(gameState);
			middle::Id copy = middle::deepCopyShape(gameState, funcShape.id.index, topBubbleId.index);
			auto reparent = middle::EditorActionReparent(topBubbleId.index, copy.index);
			reparent.execute(gameState);
			bubbleActions::updateVariable(gameState, copy, output.label);
		}

		else if (function->type == functionTypes::NEW_MULTERM) {
			components::InputVariable input;
			components::OutputVariable output;
			getOneInput(gameState, funcShape, input);
			getOutput(gameState, funcShape, output);
			assert(input.unitRef.index != middle::UNASSIGNED);
			middle::Id topBubbleId = bubbleActions::topLevelBubble(gameState);
			middle::Id copy = middle::deepCopyShape(gameState, funcShape.id.index, topBubbleId.index);
			auto replacement = bubbleActions::CreateMulitiplicationReplacementShape(topBubbleId, copy);
			replacement.execute(gameState);
			bubbleActions::updateVariable(gameState, copy, output.label);
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
					middle::Id funcShapeId = activeLoop->loopMemberIds[0];

					middle::Id landedFunc;
					executeFunctions(gameState, funcShapeId, landedFunc);

					middle::Id nextId;
					bool atEnd = false;
					getNextBlock(gameState, landedFunc, &nextId, atEnd);
					if (atEnd) {
						procedure->executing = false;
					}
					procedure->activeCodeBlock = nextId;
				}
			}

			return true;
			});
	}
};

static middle::SystemRegistrar<ProcedureExecutionSystem> reg("ProcedureExecutionSystem");
