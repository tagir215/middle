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
#include <queue>

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
	bool checkIfConditionTrue(middle::GameState* gameState, middle::Shape& funcShape) {
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
			bubbleActions::UpdateVariable(output.label, foundId).execute(gameState);
			bool isConditionTrue = foundId.index != middle::UNASSIGNED;
			return isConditionTrue;
		}

		assert(false);
	}

	// execute funcId,  landedFuncId is where the latest execution took place
	void executeFunctions(middle::GameState* gameState, middle::Shape& funcShape) {

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
				auto multiply = std::make_unique<bubbleActions::ExecuteMultiplication>(varA.unitRef, varB.unitRef);
				multiply->execute(gameState);
				auto update = std::make_unique<bubbleActions::UpdateVariable>(output.label, multiply->resultShapeId);
				update->execute(gameState);
				function->actions.push_back(std::move(multiply));
				function->actions.push_back(std::move(update));
			}
			else {
				auto combine = std::make_unique<bubbleActions::ExecuteAddition>(varA.unitRef, varB.unitRef);
				combine->execute(gameState);
				auto update = std::make_unique<bubbleActions::UpdateVariable>(output.label, combine->resultShapeId);
				update->execute(gameState);
				function->actions.push_back(std::move(combine));
				function->actions.push_back(std::move(update));
			}
		}

		// exit loops
		else if (function->type == functionTypes::EXIT_LOOP) {

		}

		else if (function->type == functionTypes::COPY) {
			components::InputVariable input;
			components::OutputVariable output;
			getOneInput(gameState, funcShape, input);
			getOneOutput(gameState, funcShape, output);
			auto copyAction = std::make_unique<middle::EditorActionCopySingle>(input.unitRef);
			copyAction->execute(gameState);
			auto update = std::make_unique<bubbleActions::UpdateVariable>(output.label, copyAction->resultId);
			update->execute(gameState);
			function->actions.push_back(std::move(copyAction));
			function->actions.push_back(std::move(update));
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
			auto reparent = std::make_unique<middle::EditorActionReparent>(topBubbleId.index, copy.index);
			reparent->execute(gameState);
			auto update = std::make_unique<bubbleActions::UpdateVariable>(output.label, copy);
			update->execute(gameState);
			function->actions.push_back(std::move(reparent));
			function->actions.push_back(std::move(update));
		}

		else if (function->type == functionTypes::NEW_MULTERM) {
			components::InputVariable input;
			components::OutputVariable output;
			getOneInput(gameState, funcShape, input);
			getOneOutput(gameState, funcShape, output);
			assert(input.unitRef.index != middle::UNASSIGNED);
			middle::Id topBubbleId = bubbleActions::topLevelBubble(gameState);
			middle::Id copy = middle::deepCopyShape(gameState, funcShape.id.index, topBubbleId.index);
			auto replacement = std::make_unique<bubbleActions::CreateMulitiplicationReplacementShape>(topBubbleId, copy);
			replacement->execute(gameState);
			auto update = std::make_unique<bubbleActions::UpdateVariable>(output.label, copy);
			update->execute(gameState);
			function->actions.push_back(std::move(replacement));
			function->actions.push_back(std::move(update));
		}

		else if (function->type == functionTypes::POP) {
			components::InputVariable input;
			getOneInput(gameState, funcShape, input);
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto popAction = std::make_unique<bubbleActions::Pop>(input.unitRef);
			popAction->execute(gameState);
			function->actions.push_back(std::move(popAction));
		}

		else if (function->type == functionTypes::MUL_ONE) {
			components::InputVariable input;
			components::OutputVariable output;
			getOneInput(gameState, funcShape, input);
			getOneOutput(gameState, funcShape, output);
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto mulOneAction = std::make_unique<bubbleActions::MulOne>(input.unitRef);
			mulOneAction->execute(gameState);
			auto update = std::make_unique<bubbleActions::UpdateVariable>(output.label, mulOneAction->resultShapeId);
			update->execute(gameState);
			function->actions.push_back(std::move(mulOneAction));
			function->actions.push_back(std::move(update));
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
			auto breakAction = std::make_unique<bubbleActions::Break>(inputA.unitRef, value);
			breakAction->execute(gameState);
			auto update = std::make_unique<bubbleActions::UpdateVariable>(output.label, breakAction->resultShapeId);
			update->execute(gameState);
			function->actions.push_back(std::move(breakAction));
			function->actions.push_back(std::move(update));
		}

		else if (function->type == functionTypes::COMPRESS) {
			components::InputVariable input;
			components::OutputVariable outputA;
			components::OutputVariable outputB;
			getOneInput(gameState, funcShape, input);
			getTwoOutputs(gameState, funcShape, outputA, outputB);
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto compressAction = std::make_unique<bubbleActions::Compress>(input.unitRef);
			compressAction->execute(gameState);
			auto updateA = std::make_unique<bubbleActions::UpdateVariable>(outputA.label, compressAction->resultCountBubbleId);
			updateA->execute(gameState);
			auto updateB = std::make_unique<bubbleActions::UpdateVariable>(outputB.label, compressAction->resultCompressedBubbleId);
			updateB->execute(gameState);
			function->actions.push_back(std::move(compressAction));
			function->actions.push_back(std::move(updateA));
			function->actions.push_back(std::move(updateB));
		}
	}

	void undoFunctions(middle::GameState* gameState, middle::Shape& funcShape) {
		auto function = middle::getComponent<components::CodeFunction>(funcShape);
		while (function->actions.size() > 0) {
			function->actions.back()->undo(gameState);
			function->actions.pop_back();
		}
	}

	int currentIndex(middle::GameState* gameState, middle::Id& id, std::vector<middle::Id>& neighbors) {
		for (int i = 0; i < neighbors.size(); ++i) {
			if (neighbors[i] == id) {
				return i;
			}
		}
		return middle::UNASSIGNED;
	}

	procedureConstants::StepStatus stepSouth(middle::GameState* gameState, components::ProcedureContainer* container) {
		middle::Id previousId = container->procedureTransitionStack.back().destinationId;
		if (previousId.index == middle::UNASSIGNED) {
			return procedureConstants::CannotStep;
		}
		middle::Id& parentId = middle::getParent(gameState, previousId);
		std::vector<middle::Id> neighbors;
		middle::getChildren(gameState, parentId, neighbors);
		int index = currentIndex(gameState, previousId, neighbors);
		if (index + 1 >= neighbors.size()) {
			return procedureConstants::StepStatus::CannotStep;
		}
		middle::Id nextId = neighbors[index + 1];
		container->procedureTransitionStack.push_back(
			{ procedureConstants::TransitionType::South, previousId, nextId 
			});
		return procedureConstants::StepStatus::CanStep;
	}

	bool isScope(middle::GameState* gameState, middle::Id& id) {
		auto& childShape = middle::getShape(gameState, id.index);
		return middle::getComponent<components::ScopeComponent>(childShape) != nullptr;
	}

	procedureConstants::StepStatus stepEast(middle::GameState* gameState, components::ProcedureContainer* container, bool isConditionTrue) {
		middle::Id previousId = container->procedureTransitionStack.back().destinationId;
		std::queue<middle::Id>stepQueue;
		stepQueue.push(previousId);
		int scopeCount = 0;
		// step down until scope
		while (stepQueue.size() > 0) {
			middle::Id id = stepQueue.front();
			stepQueue.pop();
			if (isScope(gameState, id)) {
				++scopeCount;
				middle::Id nextId = middle::getFirstChildWithComponent(gameState, id, middle::getTypeId<components::CodeBlock>());
				if (scopeCount == 1 && isConditionTrue) {
					container->procedureTransitionStack.push_back(
						{ procedureConstants::TransitionType::NorthEast, previousId, nextId });
					return procedureConstants::StepStatus::CanStep;
				}
				else if (scopeCount == 2 && !isConditionTrue) {
					container->procedureTransitionStack.push_back(
						{ procedureConstants::TransitionType::SouthEast, previousId, nextId });
					return procedureConstants::StepStatus::CanStep;
				}
			}

			std::vector<middle::Id>children;
			middle::getChildren(gameState, id, children);
			for (middle::Id childId : children) {
				stepQueue.push(childId);
			}
		}
		return procedureConstants::CannotStep;
	}

	middle::Id getCodeBlockFunc(middle::GameState* gameState, middle::Id& codeBlockId) {
		std::vector<middle::Id>children;
		middle::getChildren(gameState, codeBlockId, children);
		if (children.size() == 0) {
			return middle::Id();
		}
		// code blocks expect 1 child
		assert(children.size() == 1);
		return children[0];
	}

	void doStep(components::ProcedureContainer* container) {
		container->activeBlock = container->procedureTransitionStack.back().destinationId;
	}


	procedureConstants::StepStatus stepWest(middle::GameState* gameState, components::ProcedureContainer* container) {
		middle::Id previousId = container->procedureTransitionStack.back().destinationId;
		if (previousId.index == middle::UNASSIGNED) {
			return procedureConstants::CannotStep;
		}
		std::stack<middle::Id>stepStack;
		stepStack.push(previousId);
		// step up until if comp
		while (stepStack.size() > 0) {
			middle::Id id = stepStack.top();
			stepStack.pop();
			auto& shape = middle::getShape(gameState, id.index);
			auto ifComp = middle::getComponent<components::IfComponent>(shape);
			if (ifComp) {
				middle::Id nextId = middle::getParent(gameState, id);
				container->procedureTransitionStack.push_back(
					{ procedureConstants::TransitionType::West, previousId, nextId });
				return procedureConstants::StepStatus::CanStep;
			}

			middle::Id& parentId = middle::getParent(gameState, id);
			if (parentId.index == middle::UNASSIGNED) {
				return procedureConstants::CannotStep;
			}
			stepStack.push(parentId);
		}
		return procedureConstants::CannotStep;
	}

	procedureConstants::StepStatus stepEnd(middle::GameState* gameState, components::ProcedureContainer* container) {
		middle::Id previousId = container->procedureTransitionStack.back().destinationId;
		if (previousId.index != middle::UNASSIGNED) {
			container->procedureTransitionStack.push_back(
				{ procedureConstants::TransitionType::End, previousId, middle::Id() }
			);
			return procedureConstants::CanStep;
		}
		return procedureConstants::CannotStep;
	}

	procedureConstants::StepStatus stepStart(middle::GameState* gameState, components::ProcedureContainer* container) {
		container->procedureTransitionStack.push_back(
			{ procedureConstants::TransitionType::Start, middle::Id(), container->startBlock }
		);
		return procedureConstants::CanStep;
	}

	procedureConstants::StepStatus stepForward(middle::GameState* gameState, components::ProcedureContainer* container) {
		if (container->reset) {
			return stepStart(gameState, container);
		}

		auto statusA = stepSouth(gameState, container);
		if (statusA == procedureConstants::CanStep) {
			return statusA;
		}

		auto statusB = stepWest(gameState, container);
		if (statusB == procedureConstants::CanStep) {
			return stepSouth(gameState, container);
		}

		return stepEnd(gameState, container);
	}

	procedureConstants::StepStatus stepBackward(middle::GameState* gameState, components::ProcedureContainer* container) {
		if (container->procedureTransitionStack.size() > 0) {
			container->procedureTransitionStack.pop_back();
		}
		if (container->procedureTransitionStack.size() > 0) {
			return procedureConstants::CanStep;
		}
		return procedureConstants::CannotStep;

	}

	procedureConstants::StepStatus potentialConditionalStep(middle::GameState* gameState, components::ProcedureContainer* container) {
		middle::Id& previousId = container->procedureTransitionStack.back().destinationId;
		if (previousId.index == middle::UNASSIGNED) {
			return procedureConstants::CannotStep;
		}
		middle::Id id = getCodeBlockFunc(gameState, container->activeBlock);
		if (id.index == middle::UNASSIGNED) {
			return procedureConstants::CannotStep;
		}
		auto& funcShape = middle::getShape(gameState, id.index);
		auto ifComp = middle::getComponent<components::IfComponent>(funcShape);
		if (ifComp) {
			bool condResult = checkIfConditionTrue(gameState, funcShape);
			auto status = stepEast(gameState, container, condResult);
			return status;
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
				procedureContainer->reset = true;
				procedureContainer->procedureTransitionStack.clear();
			}

			// step
			if (bubble::buttonClicked(gameState, shape, bubbleButton::STEP_FORWARD)) {
				// navigate to procedure scope... 
				middle::Id parentId = middle::getParent(gameState, shape.id);
				auto& parentShape = middle::getShape(gameState, parentId.index);
				auto procedureContainer = middle::getComponent<components::ProcedureContainer>(parentShape);
				procedureContainer->mode = procedureConstants::STEPPING;
				procedureContainer->direction = procedureConstants::FORWARD;
				if (procedureContainer->procedureTransitionStack.size() == 0) {
					procedureContainer->reset = true;
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
			}


			auto procedure = middle::getComponent<components::ProcedureContainer>(shape);
			if (procedure &&
				(procedure->mode == procedureConstants::EXECUTING
					|| procedure->mode == procedureConstants::STEPPING)) {

				if (procedure->direction == procedureConstants::FORWARD) {
					if (stepForward(gameState, procedure) == procedureConstants::CanStep) {
						doStep(procedure);
					}

					if (potentialConditionalStep(gameState, procedure) == procedureConstants::CanStep) {
						doStep(procedure);
					}

					if (procedure->activeBlock.index != middle::UNASSIGNED) {
						middle::Id funcShapeId = getCodeBlockFunc(gameState, procedure->activeBlock);
						if (funcShapeId.index != middle::UNASSIGNED) {
							auto& funcShape = middle::getShape(gameState, funcShapeId.index);
							executeFunctions(gameState, funcShape);
						}
					}
				}

				if (procedure->direction == procedureConstants::BACKWARD) {
					middle::Id funcShapeId = getCodeBlockFunc(gameState, procedure->activeBlock);
					if (funcShapeId.index != middle::UNASSIGNED) {
						auto& funcShape = middle::getShape(gameState, funcShapeId.index);
						undoFunctions(gameState, funcShape);
					}
					if (stepBackward(gameState, procedure) == procedureConstants::CanStep) {
						doStep(procedure);
					}
				}

				if (procedure->reset) {
					procedure->reset = false;
				}

				if (procedure->mode == procedureConstants::EXECUTING) {
					timer = middle::addComponent<components::TimerComponent>(shape);
					timer->timeLeft = 1;
				}

				if (procedure->mode == procedureConstants::STEPPING) {
					procedure->mode = procedureConstants::IDLE;
				}
			}

			return true;
			});
	}
};

static middle::SystemRegistrar<ProcedureExecutionSystem> reg("ProcedureExecutionSystem");
