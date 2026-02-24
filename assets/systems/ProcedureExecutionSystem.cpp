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
#include "ScopeComponent.h"

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

	middle::Id findParentScope(middle::GameState* gameState, middle::Id& parentId) {
		std::stack<middle::Id>stack;
		middle::Id id = middle::getParent(gameState, parentId);
		if (parentId.index == middle::UNASSIGNED) {
			return middle::Id();
		}
		stack.push(parentId);
		while (stack.size() > 0) {
			middle::Id currentId = stack.top();
			stack.pop();
			auto& parentShape = middle::getShape(gameState, currentId.index);
			auto scope = middle::getComponent<components::ScopeComponent>(parentShape);
			if (scope) {
				return currentId;
			}
			else {
				middle::Id parentParentId = middle::getParent(gameState, currentId);
				if (parentParentId.index == middle::UNASSIGNED) {
					return parentParentId;
				}
				stack.push(parentParentId);
			}
		}
	}

	// move scope index forward unless loop
	middle::Id updateIndex(middle::GameState* gameState, middle::Id& scopeShapeId) {
		auto& scopeShape = middle::getShape(gameState, scopeShapeId.index);
		auto loop = middle::getComponent<components::LoopSociety>(scopeShape);
		auto scope = middle::getComponent<components::ScopeComponent>(scopeShape);
		auto& currentBlockShape = middle::getShape(gameState, loop->loopMemberIds[scope->currentIndex].index);
		auto block = middle::getComponent<components::CodeBlock>(currentBlockShape);
		if (block->type == codeBlockTypes::BLOCK) {
			scope->currentIndex += 1;
		}
		if (block->type == codeBlockTypes::LOOP_BLOCK) {

		}
		if (scope->currentIndex >= loop->loopMemberIds.size()) {
			scope->currentIndex = 0;
			return middle::Id();
		}
		return loop->loopMemberIds[scope->currentIndex];
	}

	// returns scope
	middle::Id updateScope(middle::GameState* gameState, middle::Id scopeId) {
		middle::Id nextBlock = updateIndex(gameState, scopeId);
		if (nextBlock.index != middle::UNASSIGNED) {
			return scopeId;
		}
		return findParentScope(gameState, scopeId);

	}

	middle::Id& getActiveFunction(middle::GameState* gameState, middle::Id& scope) {
		auto& scopeShape = middle::getShape(gameState, scope.index);
		auto scopeComp = middle::getComponent<components::ScopeComponent>(scopeShape);
		auto loop = middle::getComponent<components::LoopSociety>(scopeShape);
		auto& codeBlock = middle::getShape(gameState, loop->loopMemberIds[scopeComp->currentIndex].index);
		auto codeLoop = middle::getComponent<components::LoopSociety>(codeBlock);
		if (codeLoop->loopMemberIds.size() > 0) {
			return codeLoop->loopMemberIds[0];
		}
		return middle::Id();
	}

	middle::Id getConditionScope(middle::GameState* gameState, middle::Id& condFuncId, bool isConditionTrue) {
		auto& funcShape = middle::getShape(gameState, condFuncId.index);
		auto funcLoop = middle::getComponent<components::LoopSociety>(funcShape);
		bool shouldSkip = !isConditionTrue;
		for (middle::Id& id : funcLoop->loopMemberIds) {
			auto& childShape = middle::getShape(gameState, id.index);
			auto scope = middle::getComponent<components::ScopeComponent>(childShape);
			if (!scope) {
				continue;
			}
			if (shouldSkip) {
				shouldSkip = false;
				continue;
			}
			scope->currentIndex = 0;
			return id;
		}
		assert(false);
	}

	// returns scope 
	middle::Id handleConditionals(middle::GameState* gameState, middle::Id& scope) {
		middle::Id activeFunction = getActiveFunction(gameState, scope);
		if (activeFunction.index == middle::UNASSIGNED) {
			return scope;
		}
		auto& funcShape = middle::getShape(gameState, activeFunction.index);
		auto function = middle::getComponent<components::CodeFunction>(funcShape);

		// find bubbles: store found bubble to output, execute upper scope if found, execute lower scope if didn't find
		if (function->type == functionTypes::FIND_BUBBLE) {
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
			return getConditionScope(gameState, funcShape.id, isConditionTrue);
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
			return getConditionScope(gameState, funcShape.id, isConditionTrue);
		}

		return scope;
	}

	// execute funcId,  landedFuncId is where the latest execution took place
	void executeFunctions(middle::GameState* gameState, middle::Id& scope) {

		middle::Id funcId = getActiveFunction(gameState, scope);

		if (funcId.index == middle::UNASSIGNED) {
			return;
		}

		auto& funcShape = middle::getShape(gameState, funcId.index);
		auto function = middle::getComponent<components::CodeFunction>(funcShape);

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
				auto multiply = bubbleActions::ExecuteMultiplication(parentShape.id, varA.unitRef, varB.unitRef);
				multiply.execute(gameState);
				multiply.finalize(gameState);
				bubbleActions::updateVariable(gameState, multiply.resultShapeId, output.label);
			}
			else {
				auto combine = bubbleActions::ExecuteAddition(varA.unitRef, varB.unitRef);
				combine.execute(gameState);
				combine.finalize(gameState);
				bubbleActions::updateVariable(gameState, combine.resultShapeId, output.label);
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
					procedure->activeScope = procedureShape.id;
				}
			}

			auto procedure = middle::getComponent<components::ProcedureComponent>(shape);
			if (procedure && procedure->executing) {
				middle::Id scope = handleConditionals(gameState, procedure->activeScope);
				executeFunctions(gameState, scope);
				middle::Id updatedScope = updateScope(gameState, scope);
				procedure->activeScope = updatedScope;

				if (updatedScope.index == middle::UNASSIGNED) {
					procedure->executing = false;
				}
			}

			return true;
			});
	}
};

static middle::SystemRegistrar<ProcedureExecutionSystem> reg("ProcedureExecutionSystem");
