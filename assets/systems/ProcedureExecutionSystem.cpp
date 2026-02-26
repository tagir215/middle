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
#include "TimerComponent.h"
#include "ProcedureContainer.h"
#include "bubble_utils.h"

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

	void getOneOutput(middle::GameState* gameState, middle::Shape& funcShape, components::OutputVariable& var) {
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

	void getTwoOutputs(middle::GameState* gameState, middle::Shape& funcShape, components::OutputVariable& varA, components::OutputVariable& varB) {
		auto variablesLoop = middle::getComponent<components::LoopSociety>(funcShape);
		assert(variablesLoop);
		bool varAFound = false;
		for (middle::Id& childId : variablesLoop->loopMemberIds) {
			auto& childShape = middle::getShape(gameState, childId.index);
			auto inputVar = middle::getComponent<components::OutputVariable>(childShape);
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



	middle::Id findParentScope(middle::GameState* gameState, middle::Id& scopeId) {
		std::stack<middle::Id>stack;
		middle::Id parentId = middle::getParent(gameState, scopeId);
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
		if (block->type == codeBlockTypes::BLOCK || block->exitLoop) {
			scope->currentIndex += 1;
			block->exitLoop = false;
		}
		if (block->type == codeBlockTypes::LOOP_BLOCK) {

		}
		if (scope->currentIndex >= loop->loopMemberIds.size()) {
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
		middle::Id parentScopeId = findParentScope(gameState, scopeId);
		if (parentScopeId.index != middle::UNASSIGNED) {
			updateIndex(gameState, parentScopeId);
			return parentScopeId;
		}
		return scopeId;
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

	// find block loop
	middle::Id findParentLoopBlock(middle::GameState* gameState, middle::Id& funcId) {
		std::stack<middle::Id> parents;
		middle::Id blockParent = middle::getParent(gameState, funcId);
		parents.push(blockParent);
		while (parents.size() > 0) {
			middle::Id id = parents.top();
			parents.pop();
			auto& blockShape = middle::getShape(gameState, id.index);
			auto codeBlock = middle::getComponent<components::CodeBlock>(blockShape);
			if (codeBlock && codeBlock->type == codeBlockTypes::LOOP_BLOCK) {
				return blockShape.id;
			}
			middle::Id parent = middle::getParent(gameState, id);
			if (parent.index == middle::UNASSIGNED) {
				return middle::Id();
			}
			parents.push(parent);
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

	// search type of thing from a bubble
	middle::Id searchFromBubble(middle::GameState* gameState, middle::Id& bubbleId, int typeId) {
		auto& bubbleShape = middle::getShape(gameState, bubbleId.index);
		auto bubbleComp = middle::getComponent<components::BubbleComponent>(bubbleShape);
		std::vector<middle::Id>children;
		middle::getChildren(gameState, bubbleId, children);
		int size = children.size();
		int index = 0;
		while (index < size) {
			middle::Id id = children[bubbleComp->searchIndex];
			++bubbleComp->searchIndex;
			auto& shape = middle::getShape(gameState, id.index);
			if (shape.componentMap.find(typeId) != shape.componentMap.end()) {
				return id;
			}
			if (bubbleComp->searchIndex >= size) {
				bubbleComp->searchIndex = 0;
			}
			++index;
		}
		return middle::Id();
	}

	// returns scope 
	middle::Id handleConditionals(middle::GameState* gameState, middle::Id& scope) {
		middle::Id activeFunction = getActiveFunction(gameState, scope);
		if (activeFunction.index == middle::UNASSIGNED) {
			return scope;
		}
		auto& funcShape = middle::getShape(gameState, activeFunction.index);
		auto function = middle::getComponent<components::CodeFunction>(funcShape);

		int findTypeId = -1;
		if (function->type == functionTypes::FIND_BUBBLE) {
			findTypeId = middle::getTypeId<components::BubbleComponent>();
		}
		if (function->type == functionTypes::FIND_FRACTION) {
			findTypeId = middle::getTypeId<components::FractionalComponent>();
		}
		if (function->type == functionTypes::FIND_UNIT) {
			findTypeId = middle::getTypeId<components::BubbleUnit>();
		}

		// find bubbles: store found bubble to output, execute upper scope if found, execute lower scope if didn't find
		if (findTypeId >= 0) {
			components::InputVariable input;
			components::OutputVariable output;
			getOneInput(gameState, funcShape, input);
			getOneOutput(gameState, funcShape, output);
			middle::Id foundId = searchFromBubble(gameState, input.unitRef, findTypeId);
			bubbleActions::updateVariable(gameState, foundId, output.label);
			bool isConditionTrue = foundId.index != middle::UNASSIGNED;
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
			getOneOutput(gameState, funcShape, output);
			assert(varA.unitRef.index != middle::UNASSIGNED);
			assert(varB.unitRef.index != middle::UNASSIGNED);
			middle::Id& parentId = middle::getParent(gameState, varA.unitRef);
			auto& parentShape = middle::getShape(gameState, parentId.index);
			auto bubbleMultiplication = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);

			if (bubbleMultiplication) {
				auto multiply = bubbleActions::ExecuteMultiplication(varA.unitRef, varB.unitRef);
				multiply.execute(gameState);
				bubbleActions::updateVariable(gameState, multiply.resultShapeId, output.label);
			}
			else {
				auto combine = bubbleActions::ExecuteAddition(varA.unitRef, varB.unitRef);
				combine.execute(gameState);
				bubbleActions::updateVariable(gameState, combine.resultShapeId, output.label);
			}
		}

		// exit loops
		else if (function->type == functionTypes::EXIT_LOOP) {
			middle::Id parentId = findParentLoopBlock(gameState, funcId);
			if (parentId.index == middle::UNASSIGNED) {
				return;
			}
			auto& loopBlockShape = middle::getShape(gameState, parentId.index);
			auto block = middle::getComponent<components::CodeBlock>(loopBlockShape);
			block->exitLoop = true;
		}

		else if (function->type == functionTypes::COPY) {
			components::InputVariable input;
			components::OutputVariable output;
			getOneInput(gameState, funcShape, input);
			getOneOutput(gameState, funcShape, output);
			middle::Id copy = middle::deepCopyShape(gameState, input.unitRef.index);
			bubbleActions::updateVariable(gameState, copy, output.label);
		}


		else if (function->type == functionTypes::INVERSE) {

		}

		// add new term
		else if (function->type == functionTypes::NEW_TERM) {
			components::InputVariable input;
			components::OutputVariable output;
			getOneInput(gameState, funcShape, input);
			getOneOutput(gameState, funcShape, output);
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
			getOneOutput(gameState, funcShape, output);
			assert(input.unitRef.index != middle::UNASSIGNED);
			middle::Id topBubbleId = bubbleActions::topLevelBubble(gameState);
			middle::Id copy = middle::deepCopyShape(gameState, funcShape.id.index, topBubbleId.index);
			auto replacement = bubbleActions::CreateMulitiplicationReplacementShape(topBubbleId, copy);
			replacement.execute(gameState);
			bubbleActions::updateVariable(gameState, copy, output.label);
		}

		else if (function->type == functionTypes::POP) {
			components::InputVariable input;
			getOneInput(gameState, funcShape, input);
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto popAction = bubbleActions::Pop(input.unitRef);
			popAction.execute(gameState);
		}

		else if (function->type == functionTypes::MUL_ONE) {
			components::InputVariable input;
			components::OutputVariable output;
			getOneInput(gameState, funcShape, input);
			getOneOutput(gameState, funcShape, output);
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto mulOneAction = bubbleActions::MulOne(input.unitRef);
			mulOneAction.execute(gameState);
			bubbleActions::updateVariable(gameState, mulOneAction.resultShapeId, output.label);
		}

		else if (function->type == functionTypes::BREAK) {
			components::InputVariable inputA;
			components::InputVariable inputB;
			components::OutputVariable output;
			getTwoInputs(gameState, funcShape, inputA, inputB);
			getOneOutput(gameState, funcShape, output);
			assert(inputA.unitRef.index != middle::UNASSIGNED);
			assert(inputB.unitRef.index != middle::UNASSIGNED);
			int value = (int)bubbleActions::unitValue(gameState, inputB.unitRef);
			auto breakAction = bubbleActions::Break(inputA.unitRef, value);
			breakAction.execute(gameState);
			bubbleActions::updateVariable(gameState, breakAction.resultShapeId, output.label);
		}

		else if (function->type == functionTypes::COMPRESS) {
			components::InputVariable input;
			components::OutputVariable outputA;
			components::OutputVariable outputB;
			getOneInput(gameState, funcShape, input);
			getTwoOutputs(gameState, funcShape, outputA, outputB);
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto compressAction = bubbleActions::Compress(input.unitRef);
			compressAction.execute(gameState);

			outputA.unitRef = compressAction.resultCountBubbleId;
			outputB.unitRef = compressAction.resultCompressedBubbleId;
		}
	}

	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {
			auto button = middle::getComponent<components::Button>(shape);
			auto timer = middle::getComponent<components::TimerComponent>(shape);
			if (timer && timer->timeLeft > 0) {
				return true;
			}

			// start
			if (bubble::buttonClicked(gameState, shape, bubbleButton::START_PROCEDURE_BUTTON)) {
				// navigate to procedure scope... 
				middle::Id parentId = middle::getParent(gameState, shape.id);
				auto& parentShape = middle::getShape(gameState, parentId.index);
				auto procedureContainer = middle::getComponent<components::ProcedureContainer>(parentShape);
				procedureContainer->mode = procedureConstants::EXECUTING;
				procedureContainer->direction = procedureConstants::FORWARD;
				if (procedureContainer->activeScope.index == middle::UNASSIGNED) {
					middle::Id scopeChild = middle::getFirstChildWithComponent(
						gameState, parentId, middle::getTypeId<components::ScopeComponent>());
					procedureContainer->activeScope = scopeChild;
				}
			}

			// step
			if (bubble::buttonClicked(gameState, shape, bubbleButton::STEP_FORWARD)) {
				// navigate to procedure scope... 
				middle::Id parentId = middle::getParent(gameState, shape.id);
				auto& parentShape = middle::getShape(gameState, parentId.index);
				auto procedureContainer = middle::getComponent<components::ProcedureContainer>(parentShape);
				procedureContainer->mode = procedureConstants::STEPPING;
				procedureContainer->direction = procedureConstants::FORWARD;
				if (procedureContainer->activeScope.index == middle::UNASSIGNED) {
					middle::Id scopeChild = middle::getFirstChildWithComponent(
						gameState, parentId, middle::getTypeId<components::ScopeComponent>());
					procedureContainer->activeScope = scopeChild;
				}
			}

			// step back
			if (bubble::buttonClicked(gameState, shape, bubbleButton::STEP_BACKWARD)) {
				// navigate to procedure scope... 
				middle::Id parentId = middle::getParent(gameState, shape.id);
				auto& parentShape = middle::getShape(gameState, parentId.index);
				auto procedureContainer = middle::getComponent<components::ProcedureContainer>(parentShape);
				procedureContainer->mode = procedureConstants::STEPPING;
				procedureContainer->direction = procedureConstants::BACKWARD;
				if (procedureContainer->activeScope.index == middle::UNASSIGNED) {
					middle::Id scopeChild = middle::getFirstChildWithComponent(
						gameState, parentId, middle::getTypeId<components::ScopeComponent>());
					procedureContainer->activeScope = scopeChild;
				}
			}


			auto procedure = middle::getComponent<components::ProcedureContainer>(shape);
			if (procedure &&
				(procedure->mode == procedureConstants::EXECUTING
					|| procedure->mode == procedureConstants::STEPPING)) {
				middle::Id scope = handleConditionals(gameState, procedure->activeScope);
				executeFunctions(gameState, scope);
				middle::Id updatedScope = updateScope(gameState, scope);
				procedure->activeScope = updatedScope;

				auto& procedureShape = middle::getShape(gameState, updatedScope.index);
				auto scopeComponent = middle::getComponent<components::ScopeComponent>(procedureShape);
				auto loop = middle::getComponent<components::LoopSociety>(procedureShape);

				if (scopeComponent->currentIndex >= loop->loopMemberIds.size()
					|| procedure->mode == procedureConstants::STEPPING) {
					procedure->mode = procedureConstants::IDLE;
				}

				if (procedure->mode == procedureConstants::EXECUTING) {
					timer = middle::addComponent<components::TimerComponent>(shape);
					timer->timeLeft = 1;
				}
			}

			return true;
			});
	}
};

static middle::SystemRegistrar<ProcedureExecutionSystem> reg("ProcedureExecutionSystem");
