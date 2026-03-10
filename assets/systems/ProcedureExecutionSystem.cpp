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
#include "component_utils.h"
#include "MouseClickComponent.h"

class ProcedureExecutionSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* buttonCache;
	components::CompCache* procedureCache;

	void init(middle::GameState* gameState) {
		buttonCache = middle::newCompCache(gameState);
		buttonCache->addType<components::Button>();
		buttonCache->addType<components::MouseClickComponent>();
		buttonCache->addType<components::TimerComponent>(components::NOTINTERESTED);
		procedureCache = middle::newCompCache(gameState);
		procedureCache->addType<components::ProcedureContainer>();
	}

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
			bubbleActions::UpdateVariable(output.label, [foundId]() {return foundId;}).execute(gameState);
			bool isConditionTrue = foundId.index != middle::UNASSIGNED;
			return isConditionTrue;
		}

		assert(false);
	}


	void executeFunctions(middle::GameState* gameState, middle::Shape& funcShape, components::ProcedureContainer* container) {

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
				auto multiply = std::make_shared<bubbleActions::ExecuteMultiplication>(varA.unitRef, varB.unitRef);
				auto update = std::make_shared<bubbleActions::UpdateVariable>(output.label, [multiply]() {return multiply->resultShapeId;});
				auto customMul = std::make_shared<middle::CustomActionWithUndo>(
					[multiply, update](middle::GameState* gameState) {
						multiply->execute(gameState);
						update->execute(gameState);
					},
					[multiply, update](middle::GameState* gameState) {
						update->undo(gameState);
						multiply->undo(gameState);
					});
				middle::queueAction(gameState, customMul);
				function->actions.push_back(customMul);
			}
			else {
				auto combine = std::make_shared<bubbleActions::ExecuteAddition>(varA.unitRef, varB.unitRef);
				auto update = std::make_shared<bubbleActions::UpdateVariable>(output.label, [combine]() {return combine->resultShapeId;});
				auto customAdd = std::make_shared<middle::CustomActionWithUndo>(
					[combine, update](middle::GameState* gameState) {
						combine->execute(gameState);
						update->execute(gameState);
					},
					[combine, update](middle::GameState* gameState) {
						update->undo(gameState);
						combine->undo(gameState);
					}
				);
				middle::queueAction(gameState, customAdd);
				function->actions.push_back(customAdd);
			}
		}

		// exit loops
		else if (function->type == functionTypes::EXIT_LOOP) {
			container->exitingLoop = true;
			//			while (true) {
			//				auto& activeCodeBlockShape = middle::getShape(gameState, container->activeBlock.index);
			//				auto codeBlock = middle::getComponent<components::CodeBlock>(activeCodeBlockShape);
			//				if (codeBlock->type == codeBlockTypes::LOOP_BLOCK) {
			//					if (stepForward(gameState, container) == procedureConstants::CanStep){
			//						doStep(container);
			//					}
			//					break;
			//				}
			//				else{
			//					stepWest(gameState, container);
			//					doStep(container);
			//				}
			//			}
		}

		else if (function->type == functionTypes::COPY) {
			components::InputVariable input;
			components::OutputVariable output;
			getOneInput(gameState, funcShape, input);
			getOneOutput(gameState, funcShape, output);
			auto copyAction = std::make_shared<middle::EditorActionCopySingle>(input.unitRef);
			auto update = std::make_shared<bubbleActions::UpdateVariable>(output.label, [copyAction] {return copyAction->resultId;});
			auto customCopy = std::make_shared<middle::CustomActionWithUndo>(
				[copyAction, update](middle::GameState* gameState) {
					copyAction->execute(gameState);
					update->execute(gameState);
				},
				[copyAction, update](middle::GameState* gameState) {
					update->undo(gameState);
					copyAction->undo(gameState);
				});
			middle::queueAction(gameState, customCopy);
			function->actions.push_back(customCopy);
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
			middle::Id copyId = middle::deepCopyShape(gameState, funcShape.id.index, topBubbleId.index);
			auto reparent = std::make_shared<middle::EditorActionReparent>(topBubbleId.index, copyId.index);
			auto update = std::make_shared<bubbleActions::UpdateVariable>(output.label, [copyId]() {return copyId;});
			auto customReparent = std::make_shared<middle::CustomActionWithUndo>(
				[reparent, update](middle::GameState* gameState) {
					reparent->execute(gameState);
					update->execute(gameState);
				},
				[reparent, update](middle::GameState* gameState) {
					update->undo(gameState);
					reparent->undo(gameState);
				});
			middle::queueAction(gameState, customReparent);
			function->actions.push_back(customReparent);
		}

		else if (function->type == functionTypes::NEW_MULTERM) {
			components::InputVariable input;
			components::OutputVariable output;
			getOneInput(gameState, funcShape, input);
			getOneOutput(gameState, funcShape, output);
			assert(input.unitRef.index != middle::UNASSIGNED);
			middle::Id topBubbleId = bubbleActions::topLevelBubble(gameState);
			middle::Id copyId = middle::deepCopyShape(gameState, funcShape.id.index, topBubbleId.index);
			auto replacement = std::make_shared<bubbleActions::CreateMulitiplicationReplacementShape>(topBubbleId, copyId);
			auto update = std::make_shared<bubbleActions::UpdateVariable>(output.label, [copyId]() {return copyId;});
			auto customReplacement = std::make_shared<middle::CustomActionWithUndo>(
				[replacement, update](middle::GameState* gameState) {
					replacement->execute(gameState);
					update->execute(gameState);
				},
				[replacement, update](middle::GameState* gameState) {
					update->undo(gameState);
					replacement->undo(gameState);
				});
			middle::queueAction(gameState, customReplacement);
			function->actions.push_back(customReplacement);
		}

		else if (function->type == functionTypes::POP) {
			components::InputVariable input;
			getOneInput(gameState, funcShape, input);
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto popAction = std::make_shared<bubbleActions::Pop>(input.unitRef);
			middle::queueAction(gameState, popAction);
			function->actions.push_back(popAction);
		}

		else if (function->type == functionTypes::MUL_ONE) {
			components::InputVariable input;
			components::OutputVariable output;
			getOneInput(gameState, funcShape, input);
			getOneOutput(gameState, funcShape, output);
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto mulOneAction = std::make_shared<bubbleActions::MulOne>(input.unitRef);
			auto update = std::make_shared<bubbleActions::UpdateVariable>(output.label, [mulOneAction]() {return mulOneAction->resultShapeId;});
			auto customMulOne = std::make_shared<middle::CustomActionWithUndo>(
				[mulOneAction, update](middle::GameState* gameState) {
					mulOneAction->execute(gameState);
					update->execute(gameState);
				},
				[mulOneAction, update](middle::GameState* gameState) {
					update->undo(gameState);
					mulOneAction->undo(gameState);
				});
			middle::queueAction(gameState, customMulOne);
			function->actions.push_back(customMulOne);
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
			auto breakAction = std::make_shared<bubbleActions::Break>(inputA.unitRef, value);
			auto update = std::make_shared<bubbleActions::UpdateVariable>(output.label, [breakAction]() {return breakAction->resultShapeId;});
			auto customBreak = std::make_shared<middle::CustomActionWithUndo>(
				[breakAction, update](middle::GameState* gameState) {
					breakAction->execute(gameState);
					update->execute(gameState);
				},
				[breakAction, update](middle::GameState* gameState) {
					update->undo(gameState);
					breakAction->undo(gameState);
				});
			middle::queueAction(gameState, customBreak);
			function->actions.push_back(customBreak);
		}

		else if (function->type == functionTypes::COMPRESS) {
			components::InputVariable input;
			components::OutputVariable outputA;
			components::OutputVariable outputB;
			getOneInput(gameState, funcShape, input);
			getTwoOutputs(gameState, funcShape, outputA, outputB);
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto compressAction = std::make_shared<bubbleActions::Compress>(input.unitRef);
			auto updateA = std::make_shared<bubbleActions::UpdateVariable>(outputA.label, [compressAction]() {return compressAction->resultCountBubbleId;});
			auto updateB = std::make_shared<bubbleActions::UpdateVariable>(outputB.label, [compressAction]() {return compressAction->resultCompressedBubbleId;});
			auto customCompress = std::make_shared<middle::CustomActionWithUndo>(
				[compressAction, updateA, updateB](middle::GameState* gameState) {
					compressAction->execute(gameState);
					updateA->execute(gameState);
					updateB->execute(gameState);
				},
				[compressAction, updateA, updateB](middle::GameState* gameState) {
					updateB->undo(gameState);
					updateA->undo(gameState);
					compressAction->undo(gameState);
				});
			middle::queueAction(gameState, customCompress);
			function->actions.push_back(customCompress);
		}
	}

	void undoFunctions(middle::GameState* gameState, middle::Shape& funcShape) {
		auto function = middle::getComponent<components::CodeFunction>(funcShape);
		if (function->actions.size() > 0) {
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

	procedureConstants::StepStatus loopingStep(middle::GameState* gameState, components::ProcedureContainer* container) {
		middle::Id previousId = container->procedureTransitionStack.back().destinationId;
		auto& previousShape = middle::getShape(gameState, previousId.index);
		auto previousBlock = middle::getComponent<components::CodeBlock>(previousShape);
		if (previousBlock->type == codeBlockTypes::LOOP_BLOCK && !container->exitingLoop) {
			return procedureConstants::Stationary;
		}
	}


	procedureConstants::StepStatus stepSouth(middle::GameState* gameState, components::ProcedureContainer* container) {
		middle::Id previousId = container->procedureTransitionStack.back().destinationId;
		if (previousId.index == middle::UNASSIGNED) {
			return procedureConstants::CannotStep;
		}

		auto loopingResult = loopingStep(gameState, container);
		if (loopingResult == procedureConstants::Stationary) {
			return loopingResult;
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
		if (statusA == procedureConstants::CanStep || statusA == procedureConstants::Stationary) {
			return statusA;
		}

		auto statusB = stepWest(gameState, container);
		if (statusB == procedureConstants::CanStep) {
			// step to the next, unless is looping
			auto statusC = stepSouth(gameState, container);
			if (statusC == procedureConstants::CanStep) {
				container->exitingLoop = false;
				return statusC;
			}
			return statusB;
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

		auto buttonIt = buttonCache->begin<components::Button>();
		for (int i = 0; i < buttonCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, buttonCache->relevantIdVector[i].index);
			auto button = *buttonIt;

			// start
			if (button->function == bubbleButton::START_PROCEDURE_BUTTON) {
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
			if (button->function == bubbleButton::STEP_FORWARD) {
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
			if (button->function == bubbleButton::STEP_BACKWARD) {
				// navigate to procedure scope... 
				middle::Id parentId = middle::getParent(gameState, shape.id);
				auto& parentShape = middle::getShape(gameState, parentId.index);
				auto procedureContainer = middle::getComponent<components::ProcedureContainer>(parentShape);
				procedureContainer->mode = procedureConstants::STEPPING;
				procedureContainer->direction = procedureConstants::BACKWARD;
			}
		}


		auto procedureIt = procedureCache->begin<components::ProcedureContainer>();
		for (int i = 0; i < procedureCache->getSize(); ++i) {
			auto procedure = *procedureIt;
			auto& shape = middle::getShape(gameState, procedureCache->relevantIdVector[i].index);

			if ((procedure->mode == procedureConstants::EXECUTING
					|| procedure->mode == procedureConstants::STEPPING)) {

				// early exit if at beginning or end
				if (procedure->direction == procedureConstants::BACKWARD && procedure->procedureTransitionStack.size() == 0) {
					procedure->mode = procedureConstants::IDLE;
					continue;
				}
				if (procedure->direction == procedureConstants::FORWARD && procedure->procedureTransitionStack.size() > 0) {
					// update to previous before end
					if (procedure->procedureTransitionStack.back().type == procedureConstants::End) {
						procedure->activeBlock = procedure->procedureTransitionStack.back().previousId;
						procedure->procedureTransitionStack.pop_back();
					}
				}

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
							executeFunctions(gameState, funcShape, procedure);
						}
					}
				}

				if (procedure->direction == procedureConstants::BACKWARD) {
					if (procedure->activeBlock.index == middle::UNASSIGNED) {
						if (stepBackward(gameState, procedure) == procedureConstants::CanStep) {
							doStep(procedure);
						}
					}
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
					auto timer = middle::attachComponent<components::TimerComponent>(gameState, shape.id);
					timer->timeLeft = 1;
				}

				if (procedure->mode == procedureConstants::STEPPING) {
					procedure->mode = procedureConstants::IDLE;
				}
			}
		}
	}
};

static middle::SystemRegistrar<ProcedureExecutionSystem> reg("ProcedureExecutionSystem");
