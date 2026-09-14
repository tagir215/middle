#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "BubbleLogicComponent.h"
#include "ModifiedBubbleTag.h"
#include "bubble_utils.h"
#include "BubbleGateComponent.h"
#include "BubbleLogicGateConnection.h"
#include "BubbleLockedComponent.h"
#include "NeedsUpdateTag.h"

class BubbleLogicSystem : public middle::MiddleGameplaySystem {
	components::CompCache* modifiedCache;
	components::CompCache* unConnectedGateCache;
	components::CompCache* connectedGateCache;
	components::CompCache* needsUpdateGateCache;

	void init(middle::GameState* gameState) override {
		modifiedCache = middle::newCompCache(gameState, systemName);
		modifiedCache->addType<components::ModifiedBubbleTag>();

		unConnectedGateCache = middle::newCompCache(gameState, systemName);
		unConnectedGateCache->addType<components::BubbleGateComponent>();
		unConnectedGateCache->addType<components::BubbleLogicGateConnection>(components::NOTINTERESTED);

		connectedGateCache = middle::newCompCache(gameState, systemName);
		connectedGateCache->addType<components::BubbleGateComponent>();
		connectedGateCache->addType<components::BubbleLogicGateConnection>();

		needsUpdateGateCache = middle::newCompCache(gameState, systemName);
		needsUpdateGateCache->addType<components::BubbleGateComponent>();
		needsUpdateGateCache->addType<components::NeedsUpdateTag>();
	}

	void recursiveOpenLocks(middle::GameState* gameState, middle::Id id){
		auto& shape = middle::getShape(gameState, id.index);
		if (hasComp(shape, middle::getTypeId<components::BubbleLockedComponent>())) {
			middle::queueComponentDeletion<components::BubbleLockedComponent>(gameState, id);
		}

		// don't open other gates children
		if (hasComp(shape, middle::getTypeId<components::BubbleGateComponent>())) {
			return;
		}
		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);
		for (middle::Id childId : children) {
			recursiveOpenLocks(gameState, childId);
		}
	}

	void updateGateState(middle::GameState* gameState, middle::Id id, components::BubbleGateComponent* gate) {
		// open gate
		if (gate->status == components::BubbleGateStatus::OPEN) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, id, children);
			for (middle::Id childId : children) {
				recursiveOpenLocks(gameState, childId);
			}
		}
		// close gate
		else if (gate->status == components::BubbleGateStatus::CLOSED) {
			std::vector<middle::Id>children;
			middle::getAllChildren(gameState, id, children);
			int lockCompType = middle::getTypeId<components::BubbleLockedComponent>();
			for (middle::Id childId : children) {
				auto& shape = middle::getShape(gameState, childId.index);
				if (!middle::hasComp(shape, lockCompType)) {
					middle::attachComponent<components::BubbleLockedComponent>(gameState, childId);
				}
			}
		}
	}

	void update(middle::GameState* gameState) override {

		{
			// connect unconnected gate to first parent logic component
			auto gateIt = unConnectedGateCache->begin<components::BubbleGateComponent>();
			for (middle::Id id : unConnectedGateCache->relevantIdVector) {
				auto gate = *gateIt;
				// skip dummy gates
				if (gate->status == components::BubbleGateStatus::DUMMY) {
					continue;
				}
				middle::Id logicBubbleParentId = bubble::findIdWithCompFromShapeOrItsParents<components::BubbleLogicComponent>(gameState, id);
				auto connection = middle::attachComponent<components::BubbleLogicGateConnection>(gameState, id);
				connection->connectionId = logicBubbleParentId;
				updateGateState(gameState, id, gate);
			}
		}


		{
			auto gateIt = needsUpdateGateCache->begin<components::BubbleGateComponent>();
			for (middle::Id id : needsUpdateGateCache->relevantIdVector) {
				auto gate = *gateIt;
				updateGateState(gameState, id, gate);
			}
		}

		// check if something was modified
		middle::Id logicBubbleToUpdate;
		for (middle::Id id : modifiedCache->relevantIdVector) {
			middle::queueComponentDeletion<components::ModifiedBubbleTag>(gameState, id);
			middle::Id logicId = bubble::findIdWithCompFromShapeOrItsParents<components::BubbleLogicComponent>(gameState, id);
			logicBubbleToUpdate = logicId;
			break;
		}


		// check similarity 
		if (middle::isValidId(gameState, logicBubbleToUpdate)) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, logicBubbleToUpdate, children);
			// logic bubble has not enough children yet
			if (children.size() < 2) {
				return;
			}
			middle::Id left, right;
			bubble::getLogicLeftAndRight(gameState, logicBubbleToUpdate, left, right);
			// if matching open the gate
			if (bubble::matchingBubbles(gameState, left, right)) {
				// find connected gate
				middle::Id connectedId;
				auto connectionIt = connectedGateCache->begin<components::BubbleLogicGateConnection>();
				for (middle::Id gateId : connectedGateCache->relevantIdVector) {
					auto connection = *connectionIt;
					if (connection->connectionId == logicBubbleToUpdate) {
						connectedId = gateId;
						break;
					}
				}
				// open the gate
				if (connectedId.index != middle::UNASSIGNED) {
					auto gateComp = middle::getComp<components::BubbleGateComponent>(gameState, connectedId);
					gateComp->status = components::BubbleGateStatus::OPEN;
					updateGateState(gameState, connectedId, gateComp);
				}
			}

		}
	}
};

static middle::SystemRegistrar<BubbleLogicSystem> reg("BubbleLogicSystem");
