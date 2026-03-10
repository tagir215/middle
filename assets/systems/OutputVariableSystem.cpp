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
public:
	components::CompCache* compCache;
	components::CompCache* outputCache;

	void init(middle::GameState* gameState) {
		compCache = middle::newCompCache(gameState);
		compCache->addType<components::ProcedureComponent>();
		outputCache = middle::newCompCache(gameState);
		outputCache->addType<components::OutputVariable>();
	}

	std::set<std::string> usedNamesSet() {
		std::set < std::string> usedNames;
		auto outputIt = outputCache->begin<components::OutputVariable>();
		for (int i = 0; i < outputCache->getSize(); ++i) {
			auto output = *outputIt;
			usedNames.insert(output->label);
		}
		return usedNames;
	}

	void update(middle::GameState* gameState) override {

		auto procedureIt = compCache->begin<components::ProcedureComponent>();
		for (int i = 0; i < compCache->getSize(); ++i) {
			auto procedure = *procedureIt;
			auto& shape = middle::getShape(gameState, compCache->relevantIdVector[i].index);

			std::stack<middle::Id>idStack;
			idStack.push(shape.id);
			while (idStack.size() > 0) {
				middle::Id containerId = idStack.top();
				idStack.pop();
				middle::Shape& containerShape = middle::getShape(gameState, containerId.index);
				auto outputVariable = middle::getComponent<components::OutputVariable>(containerShape);
				if (outputVariable && outputVariable->label == "") {
					auto nameSet = usedNamesSet();
					while (true) {
						std::string suggestedName = "r" + std::to_string(resultId++);
						if (nameSet.find(suggestedName) == nameSet.end()) {
							outputVariable->label = suggestedName;
							break;
						}
					}
				}

				auto loop = middle::getComponent<components::LoopSociety>(containerShape);
				for (middle::Id& childId : loop->loopMemberIds) {
					idStack.push(childId);
				}
			}
		}

	}
};

static middle::SystemRegistrar<OutputVariableSystem> reg("OutputVariableSystem");
