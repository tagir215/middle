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
#include "PlacementComponent.h"
#include "ExponentComponent.h"

class ProcedureExecutionSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* buttonCache;
	components::CompCache* procedureCache;
	components::CompCache* executingProcedureCache;
	components::CompCache* inputCache;

	void init(middle::GameState* gameState) {
		buttonCache = middle::newCompCache(gameState);
		buttonCache->addType<components::Button>();
		buttonCache->addType<components::MouseClickComponent>();
		buttonCache->addType<components::TimerComponent>(components::NOTINTERESTED);
		procedureCache = middle::newCompCache(gameState);
		procedureCache->addType<components::ProcedureContainer>();
		procedureCache->addType<components::TimerComponent>(components::NOTINTERESTED);
		inputCache = middle::newCompCache(gameState);
		inputCache->addType<components::InputVariable>();
		inputCache->addType<components::MouseIntersectable>();
	}

	bool getOneInput(middle::GameState* gameState, middle::Shape& funcShape,  components::InputVariable& inputVariable) {
		std::vector<middle::Id>children;
		middle::getChildren(gameState, funcShape.id, children);
		for (middle::Id& childId : children) {
			auto& childShape = middle::getShape(gameState, childId.index);
			auto inputVar = middle::getComponent<components::InputVariable>(childShape);
			if (!inputVar) {
				continue;
			}

			inputVariable = *inputVar;

			if (inputVariable.unitRef.index == middle::UNASSIGNED) {
				return false;
			}
			if (inputVariable.structureIds.size() == 0) {
				return false;
			}

			return true;
		}
		assert(false);
		return false;
	}

	bool getTwoInputs(middle::GameState* gameState, middle::Shape& funcShape, components::InputVariable& varA, components::InputVariable& varB) {
		std::vector<middle::Id>children;
		middle::getChildren(gameState, funcShape.id, children);
		bool varAFound = false;
		for (middle::Id& childId : children) {
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
				middle::Id idA, idB;
				if (varA.unitRef.index == middle::UNASSIGNED || varB.unitRef.index == middle::UNASSIGNED) {
					return false;
				}
				if (varA.structureIds.size() == 0 || varB.structureIds.size() == 0) {
					return false;
				}
				return true;
			}
		}
		assert(false);
		return false;
	}

	// search type of thing from a bubble
	middle::Id searchFromBubble(middle::GameState* gameState, middle::Id& bubbleId, int typeId) {
		auto& bubbleShape = middle::getShape(gameState, bubbleId.index);
		auto bubbleComp = middle::getComponent<components::BubbleComponent>(bubbleShape);
		std::vector<middle::Id>children;
		middle::getChildren(gameState, bubbleId, children);
		int size = children.size();
		int index = 0;
		//while (index < size) {
		//	if (bubbleComp->searchIndex >= size) {
		//		bubbleComp->searchIndex = 0;
		//	}
		//	middle::Id id = children[bubbleComp->searchIndex];
		//	++bubbleComp->searchIndex;
		//	auto& shape = middle::getShape(gameState, id.index);
		//	if (shape.componentMap.find(typeId) != shape.componentMap.end()) {
		//		return id;
		//	}
		//	++index;
		//}
		return middle::Id();
	}


	void executeFunctions(middle::GameState* gameState, middle::Shape& funcShape, components::ProcedureContainer* container) {

		auto function = middle::getComponent<components::CodeFunction>(funcShape);

		// combine functions are either multiplciations or additions
		if (function->type == functionTypes::COMBINE) {
			components::InputVariable varA;
			components::InputVariable varB;
			if (getTwoInputs(gameState, funcShape, varA, varB)) {
				assert(varA.unitRef.index != middle::UNASSIGNED);
				assert(varB.unitRef.index != middle::UNASSIGNED);
				assert(varA.unitRef.index != varB.unitRef.index);
				middle::Id& parentId = middle::getParent(gameState, varA.unitRef);
				auto& parentShape = middle::getShape(gameState, parentId.index);
				auto bubbleMultiplication = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);

				if (bubbleMultiplication) {
					auto multiply = std::make_shared<bubbleActions::ExecuteMultiplication>(varA.unitRef, varB.unitRef);
					middle::queueAction(gameState, multiply);
					container->procedureTransitionStack.back().action = multiply;
				}
				else {
					auto combine = std::make_shared<bubbleActions::ExecuteAddition>(varA.unitRef, varB.unitRef);
					middle::queueAction(gameState, combine);
					container->procedureTransitionStack.back().action = combine;
				}
			}
			if (getOneInput(gameState, funcShape, varA)) {
				assert(varA.unitRef.index != middle::UNASSIGNED);
				auto& shape = middle::getShape(gameState, varA.unitRef.index);
				if (middle::getComponent<components::ExponentComponent>(shape)) {
					auto power = std::make_shared<bubbleActions::ExecutePower>(varA.unitRef);
					middle::queueAction(gameState, power);
					container->procedureTransitionStack.back().action = power;
				}
			}

		}

		// exit loops
		else if (function->type == functionTypes::EXIT_LOOP) {
			container->exitingLoop = true;
		}

		else if (function->type == functionTypes::COPY) {
		}


		else if (function->type == functionTypes::INVERSE) {

		}

		// add new term
		else if (function->type == functionTypes::NEW_TERM) {
			components::InputVariable input;
			if (!getOneInput(gameState, funcShape, input)) {
				return;
			}
			assert(input.unitRef.index != middle::UNASSIGNED);
			middle::Id topBubbleId = bubble::topLevelBubble(gameState);
			middle::Id copyId = middle::deepCopyShape(gameState, funcShape.id.index, topBubbleId.index);
			auto reparent = std::make_shared<middle::EditorActionReparent>(topBubbleId.index, copyId.index);
			middle::queueAction(gameState, reparent);
			container->procedureTransitionStack.back().action = reparent;

		}

		else if (function->type == functionTypes::NEW_MULTERM) {
			components::InputVariable input;
			if (!getOneInput(gameState, funcShape, input)) {
				return;
			}
			assert(input.unitRef.index != middle::UNASSIGNED);
			middle::Id topBubbleId = bubble::topLevelBubble(gameState);
			middle::Id copyId = middle::deepCopyShape(gameState, funcShape.id.index, topBubbleId.index);
			auto replacement = std::make_shared<bubbleActions::CreateMulitiplicationReplacementShape>(topBubbleId, copyId);
			middle::queueAction(gameState, replacement);
			container->procedureTransitionStack.back().action = replacement;

		}

		else if (function->type == functionTypes::POP) {
			components::InputVariable input;
			if (!getOneInput(gameState, funcShape, input)) {
				return;
			}
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto popAction = std::make_shared<bubbleActions::Pop>(input.unitRef);
			middle::queueAction(gameState, popAction);
			container->procedureTransitionStack.back().action = popAction;
		}

		else if (function->type == functionTypes::MUL_ONE) {
			components::InputVariable input;
			if (!getOneInput(gameState, funcShape, input)) {
				return;
			}
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto mulOneAction = std::make_shared<bubbleActions::MulOne>(input.unitRef);
			middle::queueAction(gameState, mulOneAction);
			container->procedureTransitionStack.back().action = mulOneAction;
		}

		else if (function->type == functionTypes::MUL_NEGATIVE_ONE) {
			components::InputVariable input;
			if (!getOneInput(gameState, funcShape, input)) {
				return;
			}
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto mulOneAction = std::make_shared<bubbleActions::MulNegativeOne>(input.unitRef);
			middle::queueAction(gameState, mulOneAction);
			container->procedureTransitionStack.back().action = mulOneAction;
		}

		else if (function->type == functionTypes::BREAK) {
			components::InputVariable inputA;
			components::InputVariable inputB;
			if (!getTwoInputs(gameState, funcShape, inputA, inputB)) {
				return;
			}
			assert(inputA.unitRef.index != middle::UNASSIGNED);
			assert(inputB.unitRef.index != middle::UNASSIGNED);
			//int value = (int)bubbleActions::unitValue(gameState, inputB.unitRef);
			int value = 3;
			auto breakAction = std::make_shared<bubbleActions::Break>(inputA.unitRef, value);
			middle::queueAction(gameState, breakAction);
			container->procedureTransitionStack.back().action = breakAction;

		}

		else if (function->type == functionTypes::COMPRESS_MULTIPLICATION) {
			components::InputVariable input;
			if (!getOneInput(gameState, funcShape, input)) {
				return;
			}
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto compressAction = std::make_shared<bubbleActions::Compress>(input.unitRef, false);
			middle::queueAction(gameState, compressAction);
			container->procedureTransitionStack.back().action = compressAction;
		}
		else if (function->type == functionTypes::COMPRESS_EXPONENT) {
			components::InputVariable input;
			if (!getOneInput(gameState, funcShape, input)) {
				return;
			}
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto compressAction = std::make_shared<bubbleActions::Compress>(input.unitRef, true);
			middle::queueAction(gameState, compressAction);
			container->procedureTransitionStack.back().action = compressAction;
		}

		else if (function->type == functionTypes::BUBBLIFY) {
			components::InputVariable input;
			if (!getOneInput(gameState, funcShape, input)) {
				return;
			}
			auto bubblifyAction = std::make_shared<bubbleActions::Bubblify>(input.unitRef);
			middle::queueAction(gameState, bubblifyAction);
			container->procedureTransitionStack.back().action = bubblifyAction;
		}
		else if (function->type == functionTypes::CANCEL) {
			components::InputVariable input;
			if (!getOneInput(gameState, funcShape, input)) {
				return;
			}
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto cancelAction = std::make_shared<bubbleActions::Cancel>(input.unitRef);
			middle::queueAction(gameState, cancelAction);
			container->procedureTransitionStack.back().action = cancelAction;
		}
		else if (function->type == functionTypes::SIMPLIFY) {
			components::InputVariable input;
			if (!getOneInput(gameState, funcShape, input)) {
				return;
			}
			assert(input.unitRef.index != middle::UNASSIGNED);
			auto simplifyAction = std::make_shared<bubbleActions::Simplify>(input.unitRef);
			middle::queueAction(gameState, simplifyAction);
			container->procedureTransitionStack.back().action = simplifyAction;
		}
	}

	void undoFunctions(middle::GameState* gameState, components::ProcedureContainer* container) {
		int index = container->procedureTransitionStack.size() - 2;
		if (index < 0) {
			return;
		}
		auto action = container->procedureTransitionStack[index].action;
		if (action) {
			middle::queueAction(gameState, std::make_shared<middle::CustomAction>(
				[action](middle::GameState* gameState) {
					action->undo(gameState);
				}));
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
		assert(index != -1);
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

	void doStepBackward(components::ProcedureContainer* container) {
		container->activeBlock = container->procedureTransitionStack.back().previousId;
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

		auto statusA = stepSouth(gameState, container);
		if (statusA == procedureConstants::CanStep || statusA == procedureConstants::Stationary) {
			return statusA;
		}

		auto statusB = stepWest(gameState, container);
		if (statusB == procedureConstants::CanStep) {
			// step to the next, unless is looping
			auto statusC = stepForward(gameState, container);
			if (statusC == procedureConstants::CanStep) {
				container->exitingLoop = false;
				return statusC;
			}
			return statusB;
		}

		return stepEnd(gameState, container);
	}

	procedureConstants::StepStatus stepBackward(middle::GameState* gameState, components::ProcedureContainer* container) {
		// don't step back before start (magic num 1)
		if (container->procedureTransitionStack.size() > 1) {
			container->procedureTransitionStack.pop_back();
		}
		if (container->procedureTransitionStack.size() > 0) {
			return procedureConstants::CanStep;
		}
		return procedureConstants::CannotStep;

	}


	procedureConstants::StepStatus procedureStepForward(middle::GameState* gameState, components::ProcedureContainer* procedure) {
		if (procedure->procedureTransitionStack.size() == 0) {
			auto status = stepStart(gameState, procedure);
			doStep(procedure);
			if (procedure->mode == procedureConstants::STEPPING) {
				procedure->mode = procedureConstants::IDLE;
			}
			return status;
		}

		doStep(procedure);

		if (procedure->activeBlock.index != middle::UNASSIGNED) {
			middle::Id funcShapeId = getCodeBlockFunc(gameState, procedure->activeBlock);
			if (funcShapeId.index != middle::UNASSIGNED) {
				auto& funcShape = middle::getShape(gameState, funcShapeId.index);
				executeFunctions(gameState, funcShape, procedure);
				stepForward(gameState, procedure);
			}
			else {
				auto status = stepForward(gameState, procedure);
				if (status == procedureConstants::StepStatus::CannotStep) {
					return status;
				}
				return procedureConstants::StepStatus::StepWithoutFunction;
			}
		}

		if (procedure->procedureTransitionStack.back().type == procedureConstants::End) {
			procedure->targetActionStackSize = procedure->procedureTransitionStack.size();
		}

		return procedureConstants::CannotStep;
	}

	procedureConstants::StepStatus procedureStepBackward(middle::GameState* gameState, components::ProcedureContainer* procedure) {
		if (procedure->targetActionStackSize < 1) {
			procedure->targetActionStackSize = 1;
		}

		if (procedure->activeBlock.index != middle::UNASSIGNED) {
			middle::Id funcShapeId = getCodeBlockFunc(gameState, procedure->activeBlock);
			if (funcShapeId.index != middle::UNASSIGNED) {
				auto& funcShape = middle::getShape(gameState, funcShapeId.index);
				undoFunctions(gameState, procedure);
			}
		}

		auto status = stepBackward(gameState, procedure);
		if (status == procedureConstants::CanStep) {
			doStepBackward(procedure);
		}
		return status;
	}


	void updateInputVariableReferences(middle::GameState* gameState, middle::Id codeBlockId, middle::Id topDogContainerId, std::unordered_map<std::string, middle::Id>& overrideMap) {
		if (codeBlockId.index == middle::UNASSIGNED) {
			return;
		}
		assert(topDogContainerId.index != middle::UNASSIGNED);
		middle::Id funcId = getCodeBlockFunc(gameState, codeBlockId);
		if (funcId.index == middle::UNASSIGNED) {
			return;
		}
		auto& funcShape = middle::getShape(gameState, funcId.index);
		if (middle::getComponent<components::PlacementComponent>(funcShape)) {
			return;
		}
		std::vector<middle::Id>inputChildren;
		middle::getChildrenWithComp(gameState, funcId, inputChildren, middle::getTypeId<components::InputVariable>());
		// update one input case
		if (inputChildren.size() == 1) {
			auto& inputChild = middle::getShape(gameState, inputChildren[0].index);
			auto input = middle::getComponent<components::InputVariable>(inputChild);
			if (input->structureIds.size() == 1) {
				middle::Id& structureId = input->structureIds[0];
				middle::Id result = bubble::findMatchingBubbleWithVariables(gameState, topDogContainerId,
					structureId, input->structureDepth, overrideMap);
				input->unitRef = result;
			}
			else if (input->structureIds.size() >= 2) {
				middle::Id& structureIdA = input->structureIds[0];
				middle::Id& structureIdB = input->structureIds[1];
				middle::Id idA, idB;
				bubble::findMatchingStructurePairWithVariables(gameState, topDogContainerId,
					structureIdA, structureIdB, input->structureDepth, overrideMap, idA, idB);
				input->unitRef = idA;
			}

		}
		// update two input case
		if (inputChildren.size() == 2) {
			auto& inputChildA = middle::getShape(gameState, inputChildren[0].index);
			auto& inputChildB = middle::getShape(gameState, inputChildren[1].index);
			auto inputA = middle::getComponent<components::InputVariable>(inputChildA);
			auto inputB = middle::getComponent<components::InputVariable>(inputChildB);
			middle::Id structureIdA = inputA->structureIds.size() > 0 ? inputA->structureIds[0] : middle::Id();
			middle::Id structureIdB = inputB->structureIds.size() > 0 ? inputB->structureIds[0] : middle::Id();


			bool bothValid = isValidId(gameState, structureIdA) && isValidId(gameState, structureIdB);
			if (bothValid) {
				middle::Id idA, idB;
				bubble::findMatchingStructurePairWithVariables(gameState, topDogContainerId,
					structureIdA, structureIdB, inputA->structureDepth, overrideMap, idA, idB);
				inputA->unitRef = idA;
				inputB->unitRef = idB;
			}
			// if one is valid update it for visual indicators
			else if (isValidId(gameState, structureIdA)) {
				if (structureIdA.index == 30) {
					int a = 0;
				}
				auto& inputShape = middle::getShape(gameState, structureIdA.index);
				auto comp = middle::getComponent<components::AlgebraNode>(inputShape);
				middle::Id result = bubble::findMatchingBubbleWithVariables(gameState, topDogContainerId,
					structureIdA, inputA->structureDepth, overrideMap);
				inputA->unitRef = result;
			}
			else if (isValidId(gameState, structureIdB)) {
				middle::Id result = bubble::findMatchingBubbleWithVariables(gameState, topDogContainerId,
					structureIdB, inputB->structureDepth, overrideMap);
				inputB->unitRef = result;
			}

		}
	}


	void update(middle::GameState* gameState) override {

		auto buttonIt = buttonCache->begin<components::Button>();
		for (int i = 0; i < buttonCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, buttonCache->relevantIdVector[i].index);
			auto button = *buttonIt;

			// start
			if (button->function == bubbleButton::START_PROCEDURE_BUTTON) {
				middle::Id procContainerId = procedureCache->relevantIdVector[0];
				auto procedureShape = middle::getShape(gameState, procContainerId.index);
				auto procedureContainer = middle::getComponent<components::ProcedureContainer>(procedureShape);
				procedureContainer->mode = procedureConstants::EXECUTING;
				procedureContainer->direction = procedureConstants::FORWARD;
				procedureContainer->procedureTransitionStack.clear();

				std::unordered_map<std::string, std::vector<middle::Id>> variableMap;
				std::vector<middle::Id>children;
				middle::getChildren(gameState, procedureShape.id, children);
				for (middle::Id& childId : children) {
					auto& childShape = middle::getShape(gameState, childId.index);
					auto input = middle::getComponent<components::InputVariable>(childShape);
					if (input) {
						bubble::getVariableStructuresMap(gameState, childId, variableMap);
						break;
					}
				}

			}

			// step
			if (button->function == bubbleButton::STEP_FORWARD) {
				// navigate to procedure scope... 
				middle::Id procedureId = procedureCache->relevantIdVector[0];
				auto& procedureShape = middle::getShape(gameState, procedureId.index);
				auto procedureContainer = middle::getComponent<components::ProcedureContainer>(procedureShape);
				procedureContainer->mode = procedureConstants::STEPPING;
				procedureContainer->direction = procedureConstants::FORWARD;
				if (procedureContainer->procedureTransitionStack.size() == 0) {
				}
			}

			// step back
			if (button->function == bubbleButton::STEP_BACKWARD) {
				// navigate to procedure scope... 
				middle::Id procedureId = procedureCache->relevantIdVector[0];
				auto& procedureShape = middle::getShape(gameState, procedureId.index);
				auto procedureContainer = middle::getComponent<components::ProcedureContainer>(procedureShape);
				procedureContainer->mode = procedureConstants::STEPPING;
				procedureContainer->direction = procedureConstants::BACKWARD;
			}

			if (button->function == bubbleButton::REVERSE_PROCEDURE) {
				middle::Id procedureId = procedureCache->relevantIdVector[0];
				auto& procedureShape = middle::getShape(gameState, procedureId.index);
				auto procedureContainer = middle::getComponent<components::ProcedureContainer>(procedureShape);
				procedureContainer->targetActionStackSize = 0;
			}
		}


		auto procedureIt = procedureCache->begin<components::ProcedureContainer>();
		for (int i = 0; i < procedureCache->getSize(); ++i) {
			auto procedure = *procedureIt;
			auto& procedureShape = middle::getShape(gameState, procedureCache->relevantIdVector[i].index);

			if (procedure->procedureTransitionStack.size() > 0 && procedure->updateInputs) {
				updateInputVariableReferences(gameState, procedure->procedureTransitionStack.back().destinationId, procedure->bubbleRef, procedure->variableOverrides);
			}

			if ((procedure->mode == procedureConstants::EXECUTING
				|| procedure->mode == procedureConstants::STEPPING)) {

				// early exit if at beginning or end
				if (procedure->direction == procedureConstants::BACKWARD && procedure->procedureTransitionStack.size() == 0) {
					procedure->mode = procedureConstants::IDLE;
					continue;
				}

				if (procedure->direction == procedureConstants::FORWARD) {
					if (procedure->procedureTransitionStack.size() == 0 || procedure->procedureTransitionStack.back().type != procedureConstants::End) {
						++procedure->targetActionStackSize;
					}
				}

				if (procedure->direction == procedureConstants::BACKWARD) {
					--procedure->targetActionStackSize;
				}
			}

			bool skipPause = false;

			// move 
			while (procedure->procedureTransitionStack.size() < procedure->targetActionStackSize) {

				if (procedure->procedureTransitionStack.size() > 0) {
					updateInputVariableReferences(gameState, procedure->procedureTransitionStack.back().destinationId, procedure->bubbleRef, procedure->variableOverrides);
				}

				auto status = procedureStepForward(gameState, procedure);
				if (status == procedureConstants::CannotStep) {
					break;
				}
				if (status == procedureConstants::StepWithoutFunction) {
					skipPause = true;
				}
			}

			while (procedure->procedureTransitionStack.size() > procedure->targetActionStackSize) {

				if (procedure->procedureTransitionStack.size() > 0) {
					updateInputVariableReferences(gameState, procedure->procedureTransitionStack.back().destinationId, procedure->bubbleRef, procedure->variableOverrides);
				}

				auto status = procedureStepBackward(gameState, procedure);
				if (status == procedureConstants::CannotStep) {
					break;
				}
			}

			if (procedure->mode == procedureConstants::EXECUTING && !skipPause) {
				auto timer = middle::attachComponent<components::TimerComponent>(gameState, procedureShape.id);
				timer->timeLeft = 0.3f;
			}

			if (procedure->mode == procedureConstants::STEPPING) {
				procedure->mode = procedureConstants::IDLE;
			}

			if (procedure->procedureTransitionStack.size() > 0) {
				if (procedure->procedureTransitionStack.back().type == procedureConstants::End) {
					procedure->mode = procedureConstants::IDLE;

					// add unod actions to last actions, which is assumed to be start procedure, in action solve mode
					// also delete procedure after execution
					if (gameState->bubbleAlgebraState.bubbleActions.size() > 0) {
						auto lastAction = gameState->bubbleAlgebraState.bubbleActions.back();
						auto startProcedure = static_cast<bubbleActions::StartProcedure*>(lastAction.get());
						for (auto& transition : procedure->procedureTransitionStack) {
							if (transition.action) {
								startProcedure->actions.push_back(transition.action);
							}
						}
						auto deleteShape = std::make_shared<middle::EditorActionDeleteSingle>(procedureShape.id);
						middle::queueAction(gameState, deleteShape);
					}
				}
			}

		}
	}
};

static middle::SystemRegistrar<ProcedureExecutionSystem> reg("ProcedureExecutionSystem");
